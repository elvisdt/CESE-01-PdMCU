/**
 * @file    app_ui.c
 * @brief   Aplicación de la estación: interfaz de usuario y consola.
 * @author  Elvis de la Torre
 * @date    4 oct 2026
 *
 * La MEF de interfaz decide qué muestra el LCD y qué hace cada botón:
 *
 * - UI_INIT: arranca en dos pasos. Primero el bus, el encoder y el LCD, que
 *   muestra la bienvenida; 1,5 s después el sensor y la primera lectura.
 * - UI_SHOW: pantalla principal. Fila 1, temperatura y humedad. Fila 2, la
 *   presión con el ícono de la alarma, o qué variable la disparó.
 * - UI_SEL_VAR: elige si se editan los límites de temperatura o de humedad.
 * - UI_SET_MIN / UI_SET_MAX: ajusta los límites sobre una copia, que recién
 *   al confirmar pasa al módulo de alarma.
 * - UI_ERROR: el sensor no responde. Libera el bus y reintenta cada 2 s.
 *
 * La medición, la alarma y la consola siguen corriendo en todos los estados:
 * mientras se edita un límite la alarma sigue evaluando con los anteriores.
 */

#include "app_ui.h"
#include "app_fmt.h"
#include "app_icons.h"

#include "API_delay.h"
#include "API_debounce.h"
#include "API_encoder.h"
#include "API_lcd.h"
#include "API_bme280.h"
#include "API_alarm.h"
#include "API_uart.h"
#include "API_cmdparser.h"
#include "API_port.h"

#include <stdio.h>
#include <string.h>

/* Configuración privada -----------------------------------------------------*/

#define UI_SAMPLE_MS    1000U   /**< el sensor mide una vez por segundo */
#define UI_RETRY_MS     2000U   /**< reintento con el sensor caído */
#define UI_RENDER_MS    200U    /**< refresco máximo del LCD */
#define UI_SPLASH_MS    1500U   /**< bienvenida antes de arrancar el sensor */
#define UI_BLINK_MS     500U    /**< parpadeo de la campana, igual al buzzer */
#define UI_NOTE_TICKS   5U      /**< refrescos que dura un aviso (unos 1 s) */

#define UI_LINE_LEN     (LCD_COLS + 1U)
#define UI_TEXT_COLS    (LCD_COLS - 1U)   /**< fila 2: texto + ícono al final */
#define UI_MSG_LEN      96U

#define LCD_DEGREE      "\xDF"  /**< el ° está en la ROM del HD44780 */

/* Tipos privados ------------------------------------------------------------*/

typedef enum {
	UI_INIT = 0,
	UI_SHOW,
	UI_SEL_VAR,
	UI_SET_MIN,
	UI_SET_MAX,
	UI_ERROR
} uiState_t;

/* Variables privadas --------------------------------------------------------*/

static const char  *varName[VAR_COUNT]   = { "Temperatura", "Humedad" };
static const char  *varUnit[VAR_COUNT]   = { "C", "%" };
static const char   varLetter[VAR_COUNT] = { 'T', 'H' };
static const float  varStep[VAR_COUNT]   = { 0.5f, 1.0f };   /**< por clic del encoder */
static const char  *alarmName[]          = { "DISABLED", "ARMED", "ACTIVE" };
static const char  *stateName[]          = { "INIT", "SHOW", "SEL_VAR", "SET_MIN", "SET_MAX", "ERROR" };

static uiState_t    state = UI_INIT;
static variable_t   sel   = VAR_TEMP;   /**< variable cuyos límites se editan */
static limits_t     edit;               /**< copia en edición */
static bme280Data_t meas;
static bool_t       measValid  = false;
static bool_t       lcdOk      = false;
static bool_t       lcdStarted = false; /**< UI_INIT: ya se hizo el paso 1 */
static bool_t       dirty      = true;  /**< hay algo nuevo para mostrar */
static const char  *note       = NULL;  /**< aviso corto en la fila 2 */
static uint8_t      noteTicks  = 0U;

/* Lo último escrito en cada fila. Si no cambió, no se vuelve a mandar: la
 * mayoría de los refrescos solo cambian una fila. */
static char shown[LCD_ROWS][UI_LINE_LEN];

static delay_t sampleTimer;
static delay_t retryTimer;
static delay_t renderTimer;
static delay_t splashTimer;

static char msg[UI_MSG_LEN];

/* Funciones privadas --------------------------------------------------------*/

static void        uiStartPeripherals(void);
static void        uiEnter(uiState_t next);
static void        uiNote(const char *text);
static void        uiSample(void);
static void        uiRetry(void);
static void        uiRender(void);
static void        uiRenderShow(char *l0, char *l1);
static void        uiAlarmText(char *buf, size_t len);
static void        uiLine(uint8_t row, const char *text);
static void        uiLogAlarm(void);
static void        uiPrint(const char *text);
static float       uiValue(variable_t v);
static float       uiClamp(float x, float lo, float hi);

static cmdStatus_t cmdGet(uint8_t argc, char *argv[]);
static cmdStatus_t cmdLim(uint8_t argc, char *argv[]);
static cmdStatus_t cmdSet(uint8_t argc, char *argv[]);
static cmdStatus_t cmdAlarm(uint8_t argc, char *argv[]);
static cmdStatus_t cmdStatus(uint8_t argc, char *argv[]);

static const cmdEntry_t uiCommands[] = {
	{ "GET",    0U, 0U, cmdGet,    "GET                  mediciones actuales" },
	{ "LIM?",   0U, 0U, cmdLim,    "LIM?                 limites de alarma (T y H)" },
	{ "SET",    3U, 3U, cmdSet,    "SET <T|H> <min> <max>" },
	{ "ALARM",  1U, 1U, cmdAlarm,  "ALARM <ON|OFF>" },
	{ "STATUS", 0U, 0U, cmdStatus, "STATUS               estado de la MEF y la alarma" },
};

#define UI_COMMAND_COUNT   ((uint8_t)(sizeof(uiCommands) / sizeof(uiCommands[0])))

/* Funciones públicas --------------------------------------------------------*/

void uiInit(void)
{
	debounceFSM_init();
	alarmInit();
	(void)cmdParserInit(uiCommands, UI_COMMAND_COUNT);
	delayInit(&sampleTimer, UI_SAMPLE_MS);
	delayInit(&retryTimer, UI_RETRY_MS);
	delayInit(&renderTimer, UI_RENDER_MS);
	delayInit(&splashTimer, UI_SPLASH_MS);
	lcdStarted = false;
	state      = UI_INIT;
}

void uiUpdate(void)
{
	/* Las entradas se leen en todos los estados: así una pulsación hecha
	 * donde no se usa se descarta, y no aparece después en otro estado. */
	debounceFSM_update();
	bool_t  okKey   = readKey(BTN_OK);
	bool_t  backKey = readKey(BTN_BACK);
	bool_t  swKey   = readKey(BTN_ENC_SW);
	int16_t steps   = encoderGetDelta();

	cmdPoll();
	alarmTask();
	uiLogAlarm();

	switch (state) {
	case UI_INIT:
		if (!lcdStarted) {
			uiStartPeripherals();
			break;
		}
		if (!delayRead(&splashTimer)) {
			break;
		}
		if (bme280Init() == BME280_OK) {
			uiPrint("BME280 OK\r\n");
			uiSample();                   /**< si falla, ya pasó a UI_ERROR */
			if (measValid) {
				uiEnter(UI_SHOW);
			}
		} else {
			uiEnter(UI_ERROR);
		}
		break;

	case UI_SHOW:
		if (swKey) {
			alarmToggle();
			uiNote((alarmGetState() == ALARM_DISABLED) ? "Alarma OFF" : "Alarma ON");
		}
		if (okKey) {
			uiEnter(UI_SEL_VAR);
		}
		break;

	case UI_SEL_VAR:
		if (steps != 0) {
			int16_t s = (int16_t)(((int16_t)sel + steps) % (int16_t)VAR_COUNT);
			sel   = (variable_t)((s < 0) ? s + (int16_t)VAR_COUNT : s);
			dirty = true;
		}
		if (okKey) {
			edit = alarmGetLimits(sel);
			uiEnter(UI_SET_MIN);
		} else if (backKey) {
			uiEnter(UI_SHOW);
		}
		break;

	case UI_SET_MIN:
		if (steps != 0) {
			/* Entre el piso del sensor y un paso por debajo del máximo. */
			edit.min = uiClamp(edit.min + (float)steps * varStep[sel],
			                   alarmGetRange(sel).min, edit.max - varStep[sel]);
			dirty = true;
		}
		if (okKey) {
			uiEnter(UI_SET_MAX);
		} else if (backKey) {
			uiEnter(UI_SEL_VAR);
		}
		break;

	case UI_SET_MAX:
		if (steps != 0) {
			/* Entre un paso por encima del mínimo y el techo del sensor. */
			edit.max = uiClamp(edit.max + (float)steps * varStep[sel],
			                   edit.min + varStep[sel], alarmGetRange(sel).max);
			note  = NULL;
			dirty = true;
		}
		if (okKey) {
			if (alarmSetLimits(sel, edit.min, edit.max)) {
				uiEnter(UI_SHOW);
				uiNote(ICON_SAVE_STR " Guardado");
			} else {
				note  = "Rango invalido";
				dirty = true;
			}
		} else if (backKey) {
			uiEnter(UI_SET_MIN);
		}
		break;

	case UI_ERROR:
		if (delayRead(&retryTimer)) {
			uiRetry();
		}
		break;

	default:
		/* Estado inválido: se vuelve a empezar desde un lugar conocido. */
		uiInit();
		break;
	}

	if (state != UI_INIT && state != UI_ERROR && delayRead(&sampleTimer)) {
		uiSample();
	}

	if (delayRead(&renderTimer) && dirty) {
		uiRender();
	}
}

/* Funciones privadas --------------------------------------------------------*/

/** @brief Paso 1 de UI_INIT: bus, encoder, LCD e íconos. */
static void uiStartPeripherals(void)
{
	uiPrint("\r\nEstacion ambiental - TP PdM\r\n");

	if (!portI2cInit()) {
		uiPrint("ERROR: bus I2C\r\n");
	}
	if (!encoderInit()) {
		uiPrint("AVISO: encoder no inicializado\r\n");
	}
	lcdOk = (lcdInit() == LCD_OK) && iconsLoad();
	uiPrint(lcdOk ? "LCD OK\r\n" : "AVISO: LCD no responde, sigo solo por UART\r\n");

	memset(shown, 0, sizeof(shown));
	lcdStarted = true;
	dirty      = true;
	(void)delayRead(&splashTimer);   /* arranca la espera de la bienvenida */
}

static void uiEnter(uiState_t next)
{
	if (next == UI_ERROR && state != UI_ERROR) {
		uiPrint("ERROR: BME280 no responde, reintentando\r\n");
		alarmSensorLost();
	}
	if (next != state) {
		(void)snprintf(msg, sizeof(msg), "UI: %s -> %s\r\n", stateName[state], stateName[next]);
		uiPrint(msg);
	}
	state = next;
	note  = NULL;
	dirty = true;
}

/** @brief Aviso corto en la fila 2 de UI_SHOW. */
static void uiNote(const char *text)
{
	note      = text;
	noteTicks = UI_NOTE_TICKS;
	dirty     = true;
}

static void uiSample(void)
{
	if (bme280Read(&meas) == BME280_OK) {
		measValid = true;
		alarmUpdate(&meas);
		dirty = true;
	} else {
		measValid = false;
		uiEnter(UI_ERROR);
	}
}

/**
 * @brief Reintento desde UI_ERROR.
 *
 * Si el sensor se desconectó en medio de una lectura puede haber quedado
 * reteniendo SDA, y entonces ni el LCD responde. Antes de reintentar se
 * libera el bus; si el LCD también se había caído, se lo vuelve a iniciar.
 */
static void uiRetry(void)
{
	if (!portI2cRecover()) {
		return;
	}
	if (!lcdOk) {
		lcdOk = (lcdInit() == LCD_OK) && iconsLoad();
	}
	memset(shown, 0, sizeof(shown));   /**< el LCD pudo haber quedado con basura */
	dirty = true;

	if (bme280Init() == BME280_OK) {
		uiSample();
		if (measValid) {
			uiEnter(UI_SHOW);
		}
	}
}

static void uiRender(void)
{
	char l0[UI_LINE_LEN];
	char l1[UI_LINE_LEN];
	char a[FMT_FLOAT1_LEN];

	dirty = false;
	if (!lcdOk) {
		return;
	}

	switch (state) {
	case UI_SHOW:
		uiRenderShow(l0, l1);
		break;

	case UI_SEL_VAR:
		(void)snprintf(l0, sizeof(l0), "Lim: %s", varName[sel]);
		(void)snprintf(l1, sizeof(l1), "OK:%c  BACK:%c", ICON_NEXT, ICON_BACK);
		break;

	case UI_SET_MIN:
		fmtFloat1(a, sizeof(a), edit.min);
		(void)snprintf(l0, sizeof(l0), "%c min: %s %s", varLetter[sel], a, varUnit[sel]);
		(void)snprintf(l1, sizeof(l1), "OK:%c  BACK:%c", ICON_NEXT, ICON_BACK);
		break;

	case UI_SET_MAX:
		fmtFloat1(a, sizeof(a), edit.max);
		(void)snprintf(l0, sizeof(l0), "%c max: %s %s", varLetter[sel], a, varUnit[sel]);
		if (note != NULL) {
			(void)snprintf(l1, sizeof(l1), "%s", note);
		} else {
			(void)snprintf(l1, sizeof(l1), "OK:%c  BACK:%c", ICON_SAVE, ICON_BACK);
		}
		break;

	case UI_ERROR:
		(void)snprintf(l0, sizeof(l0), "Error sensor");
		(void)snprintf(l1, sizeof(l1), "Reintentando...");
		break;

	default:
		(void)snprintf(l0, sizeof(l0), "Est. ambiental");
		(void)snprintf(l1, sizeof(l1), "Iniciando...");
		break;
	}

	uiLine(0U, l0);
	uiLine(1U, l1);
}

/** @brief Pantalla principal. */
static void uiRenderShow(char *l0, char *l1)
{
	char t[FMT_FLOAT1_LEN];
	char text[UI_TEXT_COLS + 1U];
	char icon;

	/* La humedad va sin decimal: el sensor tiene ±3 %HR de precisión, y así
	 * "T:-10.5°C H:100%" entra justo en las 16 columnas. */
	if (measValid) {
		uint8_t h = (uint8_t)(meas.humidity + 0.5f);
		fmtFloat1(t, sizeof(t), meas.temperature);
		(void)snprintf(l0, UI_LINE_LEN, "T:%.5s" LCD_DEGREE "C H:%3u%%", t, (unsigned)h);
	} else {
		(void)snprintf(l0, UI_LINE_LEN, "T:--.-" LCD_DEGREE "C H: --%%");
	}

	if (note != NULL && noteTicks > 0U) {
		(void)snprintf(l1, UI_LINE_LEN, "%s", note);
		noteTicks--;
		dirty = true;   /**< se sigue refrescando hasta que venza el aviso */
		return;
	}
	note = NULL;

	switch (alarmGetState()) {
	case ALARM_ACTIVE:
		uiAlarmText(text, sizeof(text));
		icon  = (((portGetTickMs() / UI_BLINK_MS) % 2U) == 0U) ? ICON_BELL : ' ';
		dirty = true;   /**< para que la campana parpadee */
		break;
	case ALARM_ARMED:
		icon = ICON_BELL;
		break;
	default:
		icon = ICON_BELL_OFF;
		break;
	}
	if (alarmGetState() != ALARM_ACTIVE) {
		fmtFloat1(t, sizeof(t), meas.pressure);
		(void)snprintf(text, sizeof(text), "P: %s hPa", measValid ? t : "--.-");
	}
	(void)snprintf(l1, UI_LINE_LEN, "%-*.*s%c", (int)UI_TEXT_COLS, (int)UI_TEXT_COLS, text, icon);
}

/** @brief Qué disparó la alarma: "ALARMA T ALTA", "ALARMA T y H", etc. */
static void uiAlarmText(char *buf, size_t len)
{
	bool_t tOut = alarmIsOutOfRange(VAR_TEMP);
	bool_t hOut = alarmIsOutOfRange(VAR_HUM);

	if (tOut && hOut) {
		(void)snprintf(buf, len, "ALARMA T y H");
	} else if (tOut || hOut) {
		variable_t v  = tOut ? VAR_TEMP : VAR_HUM;
		bool_t     hi = (uiValue(v) > alarmGetLimits(v).max);
		(void)snprintf(buf, len, "ALARMA %c %s", varLetter[v], hi ? "ALTA" : "BAJA");
	} else {
		/* Ya volvió al rango pero todavía está dentro de la histéresis. */
		(void)snprintf(buf, len, "ALARMA activa");
	}
}

/** @brief Escribe una fila completa, con espacios al final para tapar lo anterior. */
static void uiLine(uint8_t row, const char *text)
{
	char padded[UI_LINE_LEN];

	(void)snprintf(padded, sizeof(padded), "%-*s", (int)LCD_COLS, text);
	if (strcmp(padded, shown[row]) == 0) {
		return;
	}
	if (lcdSetCursor(row, 0U) != LCD_OK || lcdPrint(padded) != LCD_OK) {
		lcdOk = false;
		uiPrint("AVISO: LCD dejo de responder\r\n");
		return;
	}
	(void)memcpy(shown[row], padded, sizeof(padded));
}

/** @brief Informa por la consola cada cambio de estado de la alarma. */
static void uiLogAlarm(void)
{
	static alarmState_t last = ALARM_DISABLED;
	alarmState_t now = alarmGetState();

	if (now != last) {
		(void)snprintf(msg, sizeof(msg), "ALARMA: %s -> %s\r\n", alarmName[last], alarmName[now]);
		uiPrint(msg);
		last  = now;
		dirty = true;
	}
}

static void uiPrint(const char *text)
{
	uartSendString((uint8_t *)text);
}

static float uiValue(variable_t v)
{
	return (v == VAR_TEMP) ? meas.temperature : meas.humidity;
}

static float uiClamp(float x, float lo, float hi)
{
	if (x < lo) {
		return lo;
	}
	return (x > hi) ? hi : x;
}

/* Comandos de consola -------------------------------------------------------*/

static cmdStatus_t cmdGet(uint8_t argc, char *argv[])
{
	char t[FMT_FLOAT1_LEN];
	char h[FMT_FLOAT1_LEN];
	char p[FMT_FLOAT1_LEN];
	(void)argc;
	(void)argv;

	if (!measValid) {
		uiPrint("ERROR: sin medicion\r\n");
		return CMD_OK;
	}
	fmtFloat1(t, sizeof(t), meas.temperature);
	fmtFloat1(h, sizeof(h), meas.humidity);
	fmtFloat1(p, sizeof(p), meas.pressure);
	(void)snprintf(msg, sizeof(msg), "T=%s C  H=%s %%  P=%s hPa\r\n", t, h, p);
	uiPrint(msg);
	return CMD_OK;
}

static cmdStatus_t cmdLim(uint8_t argc, char *argv[])
{
	char a[FMT_FLOAT1_LEN];
	char b[FMT_FLOAT1_LEN];
	(void)argc;
	(void)argv;

	for (uint8_t i = 0U; i < (uint8_t)VAR_COUNT; i++) {
		limits_t l = alarmGetLimits((variable_t)i);
		fmtFloat1(a, sizeof(a), l.min);
		fmtFloat1(b, sizeof(b), l.max);
		(void)snprintf(msg, sizeof(msg), "%c: %s .. %s %s\r\n", varLetter[i], a, b, varUnit[i]);
		uiPrint(msg);
	}
	return CMD_OK;
}

static cmdStatus_t cmdSet(uint8_t argc, char *argv[])
{
	float min;
	float max;
	(void)argc;

	if (argv[1][1] != '\0') {
		return CMD_ERR_ARG;   /**< la variable es una sola letra */
	}
	for (uint8_t i = 0U; i < (uint8_t)VAR_COUNT; i++) {
		if (argv[1][0] == varLetter[i]) {
			if (!cmdParseFloat(argv[2], &min) || !cmdParseFloat(argv[3], &max) ||
			    !alarmSetLimits((variable_t)i, min, max)) {
				return CMD_ERR_ARG;
			}
			dirty = true;
			uiPrint("OK\r\n");
			return CMD_OK;
		}
	}
	return CMD_ERR_ARG;
}

static cmdStatus_t cmdAlarm(uint8_t argc, char *argv[])
{
	(void)argc;

	if (strcmp(argv[1], "ON") == 0) {
		alarmEnable(true);
	} else if (strcmp(argv[1], "OFF") == 0) {
		alarmEnable(false);
	} else {
		return CMD_ERR_ARG;
	}
	dirty = true;
	uiPrint("OK\r\n");
	return CMD_OK;
}

static cmdStatus_t cmdStatus(uint8_t argc, char *argv[])
{
	(void)argc;
	(void)argv;

	(void)snprintf(msg, sizeof(msg), "UI=%s ALARM=%s LCD=%s\r\n",
	               stateName[state], alarmName[alarmGetState()], lcdOk ? "OK" : "NO");
	uiPrint(msg);
	return CMD_OK;
}
