/*
 * app_ui.c
 *
 *  Created on: 4 oct 2026
 *      Author: elvisdt
 *
 *  MEF de interfaz de usuario:
 *
 *    UI_INIT ──(sensor OK)──> UI_SHOW ──OK──> UI_SEL_VAR ──OK──> UI_SET_MIN ──OK──> UI_SET_MAX
 *       │                     ▲ ▲  ▲            │                  │                  │  │
 *       │                     │ │  └───BACK─────┘                  │                  │  │
 *       │                     │ │                 UI_SEL_VAR <─BACK┘   UI_SET_MIN <─BACK┘  │
 *       │                     │ └──────────────OK [mín < máx] / guarda────────────────────┘
 *       └─(sensor falla)──> UI_ERROR <─(falla I2C desde cualquier estado)
 *                              └──(lectura OK)──> UI_SHOW
 *
 *  Pantalla principal (UI_SHOW):
 *    fila 1: temperatura y humedad, siempre ("T:24.8°C H: 55%").
 *    fila 2: presión, y en la última columna un ícono con el estado de la
 *            alarma (campana llena / vacía, ver app_icons.h). Con la alarma
 *            disparada muestra qué variable está fuera de rango y la campana
 *            parpadea.
 *
 *  Medición, alarma y consola corren en todos los estados.
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

/* Private defines -----------------------------------------------------------*/
#define UI_SAMPLE_MS   1000U
#define UI_RETRY_MS    2000U
#define UI_RENDER_MS   200U
#define UI_SPLASH_MS   1500U   /* pantalla de bienvenida antes de arrancar el sensor */
#define UI_BLINK_MS    500U    /* parpadeo de la campana con la alarma disparada     */
#define UI_NOTE_TICKS  5U      /* refrescos que dura un mensaje corto (~1 s)         */
#define UI_LINE_LEN    (LCD_COLS + 1U)
#define UI_MSG_LEN     96U

#define LCD_DEGREE     "\xDF"  /* símbolo ° en la ROM del HD44780 */

/* Private variables ---------------------------------------------------------*/
static const char  *varName[VAR_COUNT]   = { "Temperatura", "Humedad" };
static const char  *varUnit[VAR_COUNT]   = { "C", "%" };
static const char   varLetter[VAR_COUNT] = { 'T', 'H' };
static const float  varStep[VAR_COUNT]   = { 0.5f, 1.0f };
static const char  *alarmLong[]          = { "DISABLED", "ARMED", "ACTIVE" };
static const char  *stateName[]          = { "INIT", "SHOW", "SEL_VAR", "SET_MIN", "SET_MAX", "ERROR" };

static uiState_t    state = UI_INIT;
static variable_t   sel   = VAR_TEMP;    /* variable cuyos límites se editan */
static limits_t     edit;                /* límites en edición               */
static bme280Data_t meas;
static bool_t       measValid  = false;
static bool_t       lcdOk      = false;
static bool_t       lcdStarted = false;  /* UI_INIT: primero LCD, después sensor */
static bool_t       dirty      = true;   /* hay que redibujar el LCD         */
static const char  *note       = NULL;   /* mensaje corto en la fila 2       */
static uint8_t      noteTicks  = 0;

static delay_t dSample;
static delay_t dRetry;
static delay_t dRender;
static delay_t dSplash;

static char msg[UI_MSG_LEN];

/* Private function prototypes -----------------------------------------------*/
static void        uiEnter(uiState_t next);
static void        uiNote(const char *text);
static void        uiSample(void);
static void        uiRender(void);
static void        uiRenderShow(char *l0, char *l1);
static void        uiAlarmText(char *buf, size_t len);
static void        uiLoadIcons(void);
static void        uiLine(uint8_t row, const char *text);
static float       uiValue(variable_t v);
static void        uiLogAlarm(void);

static cmdStatus_t cmdGet(uint8_t argc, char *argv[]);
static cmdStatus_t cmdLim(uint8_t argc, char *argv[]);
static cmdStatus_t cmdSet(uint8_t argc, char *argv[]);
static cmdStatus_t cmdAlarm(uint8_t argc, char *argv[]);
static cmdStatus_t cmdStatus(uint8_t argc, char *argv[]);

static const cmdEntry_t uiCommands[] = {
	{ "GET",    0, 0, cmdGet,    "GET                  mediciones actuales" },
	{ "LIM?",   0, 0, cmdLim,    "LIM?                 limites de alarma (T y H)" },
	{ "SET",    3, 3, cmdSet,    "SET <T|H> <min> <max>" },
	{ "ALARM",  1, 1, cmdAlarm,  "ALARM <ON|OFF>" },
	{ "STATUS", 0, 0, cmdStatus, "STATUS               estado de la MEF y la alarma" },
};

/* Public functions ----------------------------------------------------------*/

void uiInit(void)
{
	debounceFSM_init();
	alarmInit();
	cmdParserInit(uiCommands, (uint8_t)(sizeof(uiCommands) / sizeof(uiCommands[0])));
	delayInit(&dSample, UI_SAMPLE_MS);
	delayInit(&dRetry, UI_RETRY_MS);
	delayInit(&dRender, UI_RENDER_MS);
	delayInit(&dSplash, UI_SPLASH_MS);
	lcdStarted = false;
	state = UI_INIT;
}

void uiUpdate(void)
{
	/* entradas: se leen siempre para que no se acumulen eventos viejos */
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
		/* Paso 1: bus, encoder y LCD con la pantalla de bienvenida */
		if (!lcdStarted) {
			uartSendString((uint8_t *)"\r\nEstacion ambiental - TP PdM\r\n");
			if (!portI2cInit()) {
				uartSendString((uint8_t *)"ERROR: bus I2C\r\n");
			}
			if (!encoderInit()) {
				uartSendString((uint8_t *)"AVISO: encoder no inicializado\r\n");
			}
			lcdOk = (lcdInit() == LCD_OK);
			if (lcdOk) {
				uiLoadIcons();
			}
			uartSendString(lcdOk ? (uint8_t *)"LCD OK\r\n"
			                     : (uint8_t *)"AVISO: LCD no responde, sigo solo por UART\r\n");
			lcdStarted = true;
			dirty      = true;
			delayRead(&dSplash);                 /* arranca la espera */
			break;
		}
		/* Paso 2: pasada la bienvenida, el sensor y la primera lectura */
		if (!delayRead(&dSplash)) {
			break;
		}
		if (bme280Init() == BME280_OK) {
			uartSendString((uint8_t *)"BME280 OK\r\n");
			uiSample();                          /* si falla ya pasa a UI_ERROR */
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
			edit.min += (float)steps * varStep[sel];
			dirty = true;
		}
		if (okKey) {
			uiEnter(UI_SET_MAX);
		} else if (backKey) {
			uiEnter(UI_SEL_VAR);                 /* descarta */
		}
		break;

	case UI_SET_MAX:
		if (steps != 0) {
			edit.max += (float)steps * varStep[sel];
			note  = NULL;
			dirty = true;
		}
		if (okKey) {
			if (alarmSetLimits(sel, edit.min, edit.max)) {
				uiEnter(UI_SHOW);
				uiNote("Guardado");
			} else {
				note  = "Rango invalido";        /* min >= max o fuera del sensor */
				dirty = true;
			}
		} else if (backKey) {
			uiEnter(UI_SET_MIN);
		}
		break;

	case UI_ERROR:
		if (delayRead(&dRetry)) {
			if (bme280Init() == BME280_OK) {
				uiSample();
				if (measValid) {                 /* vuelve solo con una lectura buena */
					uiEnter(UI_SHOW);
				}
			}
		}
		break;

	default:
		uiInit();
		break;
	}

	/* medición periódica en todos los estados de operación */
	if (state != UI_INIT && state != UI_ERROR && delayRead(&dSample)) {
		uiSample();
	}

	if (delayRead(&dRender) && dirty) {
		uiRender();
	}
}

/* Private functions ---------------------------------------------------------*/

static void uiEnter(uiState_t next)
{
	if (next == UI_ERROR && state != UI_ERROR) {
		uartSendString((uint8_t *)"ERROR: BME280 no responde, reintentando\r\n");
	}
	if (next != state) {
		snprintf(msg, sizeof(msg), "UI: %s -> %s\r\n", stateName[state], stateName[next]);
		uartSendString((uint8_t *)msg);
	}
	state = next;
	note  = NULL;
	dirty = true;
}

/* Mensaje corto en la fila 2 de UI_SHOW durante ~1 s */
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

static void uiRender(void)
{
	char l0[UI_LINE_LEN];
	char l1[UI_LINE_LEN];
	char a[8];

	dirty = false;
	if (!lcdOk) {
		return;
	}

	switch (state) {
	case UI_SHOW:
		uiRenderShow(l0, l1);
		break;

	case UI_SEL_VAR:
		snprintf(l0, sizeof(l0), "Editar limites");
		snprintf(l1, sizeof(l1), "> %s", varName[sel]);
		break;

	case UI_SET_MIN:
		fmtFloat1(a, sizeof(a), edit.min);
		snprintf(l0, sizeof(l0), "%c min: %s %s", varLetter[sel], a, varUnit[sel]);
		snprintf(l1, sizeof(l1), "OK sig BACK vol");
		break;

	case UI_SET_MAX:
		fmtFloat1(a, sizeof(a), edit.max);
		snprintf(l0, sizeof(l0), "%c max: %s %s", varLetter[sel], a, varUnit[sel]);
		snprintf(l1, sizeof(l1), "%s", (note != NULL) ? note : "OK guarda");
		break;

	case UI_ERROR:
		snprintf(l0, sizeof(l0), "Error sensor");
		snprintf(l1, sizeof(l1), "Reintentando...");
		break;

	default:                                    /* UI_INIT: bienvenida */
		snprintf(l0, sizeof(l0), "Est. ambiental");
		snprintf(l1, sizeof(l1), "Iniciando...");
		break;
	}

	uiLine(0, l0);
	uiLine(1, l1);
}

/* Pantalla principal: T y H arriba, presión / alarma abajo */
static void uiRenderShow(char *l0, char *l1)
{
	char t[8];

	/* Humedad sin decimal: el sensor tiene ±3 %HR, el decimal no aporta
	 * y así "T:-10.5°C H:100%" entra justo en 16 columnas. */
	if (measValid) {
		uint8_t h = (uint8_t)(meas.humidity + 0.5f);
		fmtFloat1(t, sizeof(t), meas.temperature);
		snprintf(l0, UI_LINE_LEN, "T:%.5s" LCD_DEGREE "C H:%3u%%", t, (unsigned)h);
	} else {
		snprintf(l0, UI_LINE_LEN, "T:--.-" LCD_DEGREE "C H: --%%");
	}

	if (note != NULL && noteTicks > 0U) {       /* mensaje corto: tiene prioridad */
		snprintf(l1, UI_LINE_LEN, "%s", note);
		noteTicks--;
		dirty = true;                           /* sigue refrescando hasta que venza */
		return;
	}
	note = NULL;

	/* columnas 0..14: texto, columna 15: ícono de la alarma */
	char text[LCD_COLS];
	char icon;

	switch (alarmGetState()) {
	case ALARM_ACTIVE:
		uiAlarmText(text, sizeof(text));
		icon  = (((portGetTickMs() / UI_BLINK_MS) % 2U) == 0U) ? ICON_BELL : ' ';
		dirty = true;                           /* sigue refrescando para parpadear */
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
		snprintf(text, sizeof(text), "P: %s hPa", measValid ? t : "--.-");
	}
	snprintf(l1, UI_LINE_LEN, "%-15.15s%c", text, icon);
}

/* Carga los íconos de app_icons.h en la CGRAM del LCD */
static void uiLoadIcons(void)
{
	if (lcdCreateChar((uint8_t)ICON_BELL, iconBell) != LCD_OK ||
	    lcdCreateChar((uint8_t)ICON_BELL_OFF, iconBellOff) != LCD_OK) {
		uartSendString((uint8_t *)"AVISO: no se cargaron los iconos del LCD\r\n");
	}
}

/* Qué variable disparó la alarma: "ALARMA T ALTA", "ALARMA T y H", ... */
static void uiAlarmText(char *buf, size_t len)
{
	bool_t tOut = alarmIsOutOfRange(VAR_TEMP);
	bool_t hOut = alarmIsOutOfRange(VAR_HUM);

	if (tOut && hOut) {
		snprintf(buf, len, "ALARMA T y H");
	} else if (tOut || hOut) {
		variable_t v  = tOut ? VAR_TEMP : VAR_HUM;
		bool_t     hi = (uiValue(v) > alarmGetLimits(v).max);
		snprintf(buf, len, "ALARMA %c %s", varLetter[v], hi ? "ALTA" : "BAJA");
	} else {
		/* volvió al rango pero todavía dentro de la histéresis */
		snprintf(buf, len, "ALARMA activa");
	}
}

/* Escribe una fila completa, rellenando con espacios para borrar lo anterior */
static void uiLine(uint8_t row, const char *text)
{
	char padded[UI_LINE_LEN];
	snprintf(padded, sizeof(padded), "%-16s", text);
	if (lcdSetCursor(row, 0) != LCD_OK || lcdPrint(padded) != LCD_OK) {
		lcdOk = false;                          /* deja de usar el LCD */
		uartSendString((uint8_t *)"AVISO: LCD dejo de responder\r\n");
	}
}

/* Informa por UART cada cambio de estado de la alarma */
static void uiLogAlarm(void)
{
	static alarmState_t last = ALARM_DISABLED;
	alarmState_t now = alarmGetState();

	if (now != last) {
		snprintf(msg, sizeof(msg), "ALARMA: %s -> %s\r\n", alarmLong[last], alarmLong[now]);
		uartSendString((uint8_t *)msg);
		last  = now;
		dirty = true;
	}
}

static float uiValue(variable_t v)
{
	return (v == VAR_TEMP) ? meas.temperature : meas.humidity;
}

/* Comandos de consola -------------------------------------------------------*/

static cmdStatus_t cmdGet(uint8_t argc, char *argv[])
{
	char t[8], h[8], p[8];
	(void)argc; (void)argv;

	if (!measValid) {
		uartSendString((uint8_t *)"ERROR: sin medicion\r\n");
		return CMD_OK;
	}
	fmtFloat1(t, sizeof(t), meas.temperature);
	fmtFloat1(h, sizeof(h), meas.humidity);
	fmtFloat1(p, sizeof(p), meas.pressure);
	snprintf(msg, sizeof(msg), "T=%s C  H=%s %%  P=%s hPa\r\n", t, h, p);
	uartSendString((uint8_t *)msg);
	return CMD_OK;
}

static cmdStatus_t cmdLim(uint8_t argc, char *argv[])
{
	char a[8], b[8];
	(void)argc; (void)argv;

	for (uint8_t i = 0; i < (uint8_t)VAR_COUNT; i++) {
		limits_t l = alarmGetLimits((variable_t)i);
		fmtFloat1(a, sizeof(a), l.min);
		fmtFloat1(b, sizeof(b), l.max);
		snprintf(msg, sizeof(msg), "%c: %s .. %s %s\r\n", varLetter[i], a, b, varUnit[i]);
		uartSendString((uint8_t *)msg);
	}
	return CMD_OK;
}

static cmdStatus_t cmdSet(uint8_t argc, char *argv[])
{
	float min, max;
	(void)argc;

	if (argv[1][1] != '\0') {
		return CMD_ERR_ARG;
	}
	for (uint8_t i = 0; i < (uint8_t)VAR_COUNT; i++) {
		if (argv[1][0] == varLetter[i]) {
			if (!cmdParseFloat(argv[2], &min) || !cmdParseFloat(argv[3], &max) ||
			    !alarmSetLimits((variable_t)i, min, max)) {
				return CMD_ERR_ARG;
			}
			dirty = true;
			uartSendString((uint8_t *)"OK\r\n");
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
	uartSendString((uint8_t *)"OK\r\n");
	return CMD_OK;
}

static cmdStatus_t cmdStatus(uint8_t argc, char *argv[])
{
	(void)argc; (void)argv;

	snprintf(msg, sizeof(msg), "UI=%s ALARM=%s LCD=%s\r\n",
			stateName[state], alarmLong[alarmGetState()], lcdOk ? "OK" : "NO");
	uartSendString((uint8_t *)msg);
	return CMD_OK;
}
