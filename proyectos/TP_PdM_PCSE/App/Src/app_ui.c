/*
 * app_ui.c
 *
 *  Created on: 4 oct 2026
 *      Author: elvisdt
 *
 *  MEF de interfaz de usuario:
 *
 *    UI_INIT ──(sensor OK)──> UI_SHOW ──OK──> UI_SET_MIN ──OK──> UI_SET_MAX
 *       │                      ▲  ▲  │          │ ▲                │   │
 *       │                      │  │  │          └─┼─BACK──> SHOW   │   │
 *       │                      │  │  │            └──────BACK──────┘   │
 *       │                      │  └──┼──────OK [mín < máx] / guarda───┘
 *       └─(sensor falla)──> UI_ERROR <─(falla I2C desde cualquier estado)
 *                              └──(lectura OK)──> UI_SHOW
 *
 *  Medición, alarma y consola corren en todos los estados.
 */

#include "app_ui.h"
#include "app_fmt.h"

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
#define UI_LINE_LEN    (LCD_COLS + 1U)
#define UI_MSG_LEN     96U

/* Private variables ---------------------------------------------------------*/
static const char  *varName[VAR_COUNT]   = { "Temp", "Hum", "Pres" };
static const char  *varUnit[VAR_COUNT]   = { "C", "%", "hPa" };
static const char   varLetter[VAR_COUNT] = { 'T', 'H', 'P' };
static const float  varStep[VAR_COUNT]   = { 0.5f, 1.0f, 1.0f };
static const char  *alarmName[]          = { "OFF", "ARM", "ALR" };
static const char  *stateName[]          = { "INIT", "SHOW", "SET_MIN", "SET_MAX", "ERROR" };

static uiState_t    state = UI_INIT;
static variable_t   sel   = VAR_TEMP;
static limits_t     edit;                /* límites en edición             */
static bme280Data_t meas;
static bool_t       measValid = false;
static bool_t       lcdOk     = false;
static bool_t       dirty     = true;    /* hay que redibujar el LCD      */
static const char  *note      = NULL;    /* mensaje corto en la línea 2   */
static uint8_t      noteTicks = 0;       /* refrescos que dura el mensaje */

static delay_t dSample;
static delay_t dRetry;
static delay_t dRender;

static char msg[UI_MSG_LEN];

/* Private function prototypes -----------------------------------------------*/
static void        uiEnter(uiState_t next);
static void        uiSample(void);
static void        uiRender(void);
static void        uiLine(uint8_t row, const char *text);
static float       uiValue(variable_t v);

static cmdStatus_t cmdGet(uint8_t argc, char *argv[]);
static cmdStatus_t cmdLim(uint8_t argc, char *argv[]);
static cmdStatus_t cmdSet(uint8_t argc, char *argv[]);
static cmdStatus_t cmdAlarm(uint8_t argc, char *argv[]);
static cmdStatus_t cmdStatus(uint8_t argc, char *argv[]);

static const cmdEntry_t uiCommands[] = {
	{ "GET",    0, 0, cmdGet,    "GET                  mediciones actuales" },
	{ "LIM?",   0, 0, cmdLim,    "LIM?                 limites de alarma" },
	{ "SET",    3, 3, cmdSet,    "SET <T|H|P> <min> <max>" },
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

	switch (state) {
	case UI_INIT:
		if (!portI2cInit()) {
			uartSendString((uint8_t *)"ERROR: bus I2C\r\n");
		}
		if (!encoderInit()) {
			uartSendString((uint8_t *)"AVISO: encoder no inicializado\r\n");
		}
		lcdOk = (lcdInit() == LCD_OK);
		if (!lcdOk) {
			uartSendString((uint8_t *)"AVISO: LCD no responde, sigo solo por UART\r\n");
		}
		if (bme280Init() == BME280_OK) {
			uiSample();
			uiEnter(UI_SHOW);
		} else {
			uiEnter(UI_ERROR);
		}
		break;

	case UI_SHOW:
		if (steps != 0) {
			int16_t s = (int16_t)(((int16_t)sel + steps) % (int16_t)VAR_COUNT);
			sel = (variable_t)((s < 0) ? s + (int16_t)VAR_COUNT : s);
			dirty = true;
		}
		if (swKey) {
			alarmToggle();
			dirty = true;
		}
		if (okKey) {
			edit = alarmGetLimits(sel);
			uiEnter(UI_SET_MIN);
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
			uiEnter(UI_SHOW);                    /* descarta */
		}
		break;

	case UI_SET_MAX:
		if (steps != 0) {
			edit.max += (float)steps * varStep[sel];
			note = NULL;
			dirty = true;
		}
		if (okKey) {
			if (alarmSetLimits(sel, edit.min, edit.max)) {
				uiEnter(UI_SHOW);
				note      = "Guardado";
				noteTicks = 5U;                  /* ~1 s */
			} else {
				note = "Rango invalido";     /* min >= max o fuera del sensor */
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
				uiEnter(UI_SHOW);
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
	state = next;
	note  = NULL;
	dirty = true;
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
	char b[8];

	dirty = false;
	if (!lcdOk) {
		return;
	}

	switch (state) {
	case UI_SHOW:
		if (measValid) {
			fmtFloat1(a, sizeof(a), uiValue(sel));
		} else {
			snprintf(a, sizeof(a), "--.-");
		}
		snprintf(l0, sizeof(l0), "%-4s%7s %-3s%c", varName[sel], a, varUnit[sel],
				alarmIsOutOfRange(sel) ? '!' : ' ');
		fmtFloat1(a, sizeof(a), alarmGetLimits(sel).min);
		fmtFloat1(b, sizeof(b), alarmGetLimits(sel).max);
		if (note != NULL && noteTicks > 0U) {
			snprintf(l1, sizeof(l1), "%s", note);
			noteTicks--;
			dirty = true;                       /* sigue refrescando hasta que venza */
		} else {
			note = NULL;
			snprintf(l1, sizeof(l1), "%s-%s %s", a, b, alarmName[alarmGetState()]);
		}
		break;

	case UI_SET_MIN:
		fmtFloat1(a, sizeof(a), edit.min);
		snprintf(l0, sizeof(l0), "%s min: %s", varName[sel], a);
		snprintf(l1, sizeof(l1), "OK sig BACK sale");
		break;

	case UI_SET_MAX:
		fmtFloat1(a, sizeof(a), edit.max);
		snprintf(l0, sizeof(l0), "%s max: %s", varName[sel], a);
		snprintf(l1, sizeof(l1), "%s", (note != NULL) ? note : "OK guarda BACK");
		break;

	case UI_ERROR:
		snprintf(l0, sizeof(l0), "Error sensor");
		snprintf(l1, sizeof(l1), "Reintentando...");
		break;

	default:
		snprintf(l0, sizeof(l0), "Iniciando...");
		l1[0] = '\0';
		break;
	}

	uiLine(0, l0);
	uiLine(1, l1);
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

static float uiValue(variable_t v)
{
	switch (v) {
	case VAR_TEMP: return meas.temperature;
	case VAR_HUM:  return meas.humidity;
	default:       return meas.pressure;
	}
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
	static const char *alarmLong[] = { "DISABLED", "ARMED", "ACTIVE" };
	(void)argc; (void)argv;

	snprintf(msg, sizeof(msg), "UI=%s ALARM=%s LCD=%s\r\n",
			stateName[state], alarmLong[alarmGetState()], lcdOk ? "OK" : "NO");
	uartSendString((uint8_t *)msg);
	return CMD_OK;
}
