/**
 * @file    app_test.c
 * @brief   Pruebas de un módulo por vez.
 * @author  Elvis de la Torre
 * @date    4 oct 2026
 *
 * Solo se compila la prueba que indica APP_TEST.
 */

#include "app_test.h"
#include "app_fmt.h"

#include "API_delay.h"
#include "API_uart.h"
#include "API_cmdparser.h"
#include "API_gpio.h"
#include "API_debounce.h"
#include "API_encoder.h"
#include "API_lcd.h"
#include "API_bme280.h"
#include "API_alarm.h"
#include "API_port.h"

#include <stdio.h>
#include <string.h>

#define TEST_MSG_LEN     96U
#define TEST_PERIOD_MS   1000U

/* No todas las pruebas usan todo. */
static char    msg[TEST_MSG_LEN] __attribute__((unused));
static delay_t period __attribute__((unused));

__attribute__((unused)) static void print(const char *s)
{
	uartSendString((uint8_t *)s);
}

/* -------------------------------------------------------------------------- */
#if APP_TEST == TEST_DELAY

#define BLINK_HALF_MS   500U

void appTestInit(void)
{
	print("TEST_DELAY: LD2 a 1 Hz\r\n");
	delayInit(&period, BLINK_HALF_MS);
}

void appTestUpdate(void)
{
	if (delayRead(&period)) {
		gpioLedToggle();
		print(gpioLedIsOn() ? "tick ON\r\n" : "tick OFF\r\n");
	}
}

/* -------------------------------------------------------------------------- */
#elif APP_TEST == TEST_UART

/* Los comandos de la práctica 5, ahora sobre el parser con tabla. */
static cmdStatus_t cmdLed(uint8_t argc, char *argv[])
{
	(void)argc;
	if (strcmp(argv[1], "ON") == 0) {
		gpioLedOn();
	} else if (strcmp(argv[1], "OFF") == 0) {
		gpioLedOff();
	} else if (strcmp(argv[1], "TOGGLE") == 0) {
		gpioLedToggle();
	} else {
		return CMD_ERR_ARG;
	}
	print("OK\r\n");
	return CMD_OK;
}

static cmdStatus_t cmdStatus(uint8_t argc, char *argv[])
{
	(void)argc;
	(void)argv;
	print(gpioLedIsOn() ? "LED is ON\r\n" : "LED is OFF\r\n");
	return CMD_OK;
}

static const cmdEntry_t testCmds[] = {
	{ "LED",    1U, 1U, cmdLed,    "LED <ON|OFF|TOGGLE>" },
	{ "STATUS", 0U, 0U, cmdStatus, "STATUS" },
};

void appTestInit(void)
{
	print("TEST_UART: consola\r\n");
	(void)cmdParserInit(testCmds, (uint8_t)(sizeof(testCmds) / sizeof(testCmds[0])));
}

void appTestUpdate(void)
{
	cmdPoll();
}

/* -------------------------------------------------------------------------- */
#elif APP_TEST == TEST_DEBOUNCE

static uint16_t count[BTN_COUNT];

void appTestInit(void)
{
	print("TEST_DEBOUNCE: presionar OK / BACK / SW\r\n");
	debounceFSM_init();
}

void appTestUpdate(void)
{
	static const char *name[BTN_COUNT] = { "OK", "BACK", "SW" };

	debounceFSM_update();
	for (uint8_t i = 0U; i < (uint8_t)BTN_COUNT; i++) {
		if (readKey((button_t)i)) {
			count[i]++;
			(void)snprintf(msg, sizeof(msg), "%s: %u\r\n", name[i], count[i]);
			print(msg);
		}
	}
}

/* -------------------------------------------------------------------------- */
#elif APP_TEST == TEST_ENCODER

static int32_t total = 0;

void appTestInit(void)
{
	print(encoderInit() ? "TEST_ENCODER: girar el encoder y presionar su eje\r\n"
	                    : "TEST_ENCODER: ERROR, TIM3 no inicializado\r\n");
	debounceFSM_init();
}

void appTestUpdate(void)
{
	int16_t d = encoderGetDelta();
	if (d != 0) {
		total += d;
		(void)snprintf(msg, sizeof(msg), "delta=%d total=%ld\r\n", d, (long)total);
		print(msg);
	}

	debounceFSM_update();
	if (readKey(BTN_ENC_SW)) {
		print("SW presionado\r\n");
	}
}

/* -------------------------------------------------------------------------- */
#elif APP_TEST == TEST_I2C_SCAN

#define I2C_ADDR_FIRST   0x08U   /**< las anteriores y posteriores están reservadas */
#define I2C_ADDR_LAST    0x77U

void appTestInit(void)
{
	uint8_t found = 0U;

	print("TEST_I2C_SCAN\r\n");
	if (!portI2cInit()) {
		print("ERROR: I2C1 no inicializado\r\n");
		return;
	}
	for (uint8_t a = I2C_ADDR_FIRST; a <= I2C_ADDR_LAST; a++) {
		if (portI2cIsReady(a)) {
			(void)snprintf(msg, sizeof(msg), "  dispositivo en 0x%02X\r\n", a);
			print(msg);
			found++;
		}
	}
	(void)snprintf(msg, sizeof(msg), "%u dispositivo(s). Esperados: 0x27 (LCD), 0x76 (BME280)\r\n", found);
	print(msg);
}

void appTestUpdate(void)
{
}

/* -------------------------------------------------------------------------- */
#elif APP_TEST == TEST_LCD

#define COUNTER_COL   11U

static uint32_t seconds = 0U;

void appTestInit(void)
{
	print("TEST_LCD\r\n");
	if (!portI2cInit() || lcdInit() != LCD_OK ||
	    lcdSetCursor(0U, 0U) != LCD_OK || lcdPrint("Hola PdM") != LCD_OK ||
	    lcdSetCursor(1U, 0U) != LCD_OK || lcdPrint("CESE - FIUBA") != LCD_OK) {
		print("ERROR: el LCD no responde en 0x27\r\n");
		return;
	}
	print("OK: en la fila 2 corre un contador de segundos\r\n");
	delayInit(&period, TEST_PERIOD_MS);
}

void appTestUpdate(void)
{
	if (delayRead(&period)) {
		seconds++;
		(void)snprintf(msg, sizeof(msg), "%5lu", (unsigned long)seconds);
		if (lcdSetCursor(1U, COUNTER_COL) != LCD_OK || lcdPrint(msg) != LCD_OK) {
			print("ERROR: el LCD dejo de responder\r\n");
		}
	}
}

/* -------------------------------------------------------------------------- */
#elif APP_TEST == TEST_BME280

void appTestInit(void)
{
	uint8_t id = 0U;

	print("TEST_BME280\r\n");
	if (!portI2cInit() || bme280ReadChipId(&id) != BME280_OK) {
		print("ERROR: el BME280 no responde en 0x76\r\n");
		return;
	}
	(void)snprintf(msg, sizeof(msg), "chip ID = 0x%02X (esperado 0x60)\r\n", id);
	print(msg);
	if (bme280Init() != BME280_OK) {
		print("ERROR: bme280Init()\r\n");
	}
	delayInit(&period, TEST_PERIOD_MS);
}

void appTestUpdate(void)
{
	bme280Data_t d;
	char t[FMT_FLOAT1_LEN];
	char h[FMT_FLOAT1_LEN];
	char p[FMT_FLOAT1_LEN];

	if (!delayRead(&period)) {
		return;
	}
	if (bme280Read(&d) == BME280_OK) {
		fmtFloat1(t, sizeof(t), d.temperature);
		fmtFloat1(h, sizeof(h), d.humidity);
		fmtFloat1(p, sizeof(p), d.pressure);
		(void)snprintf(msg, sizeof(msg), "T=%s C  H=%s %%  P=%s hPa\r\n", t, h, p);
		print(msg);
	} else {
		print("ERROR: bme280Read(), reinicializando el sensor\r\n");
		if (bme280Init() == BME280_OK) {
			print("BME280 reinicializado\r\n");
		}
	}
}

/* -------------------------------------------------------------------------- */
#elif APP_TEST == TEST_ALARM

#define FAKE_CYCLE_S    10U      /**< 5 s fuera de rango y 5 s dentro */
#define FAKE_HOT_S      5U
#define FAKE_HOT_C      40.0f
#define FAKE_OK_C       25.0f
#define FAKE_HUM        50.0f
#define FAKE_PRESS      1000.0f

static uint8_t ticks = 0U;

void appTestInit(void)
{
	print("TEST_ALARM: OK habilita/deshabilita. Dato simulado: 5 s fuera de rango, 5 s dentro\r\n");
	debounceFSM_init();
	alarmInit();
	delayInit(&period, TEST_PERIOD_MS);
}

void appTestUpdate(void)
{
	static const char *name[] = { "DISABLED", "ARMED", "ACTIVE" };

	debounceFSM_update();
	if (readKey(BTN_OK)) {
		alarmToggle();
	}
	alarmTask();

	if (delayRead(&period)) {
		bme280Data_t fake = { (ticks < FAKE_HOT_S) ? FAKE_HOT_C : FAKE_OK_C, FAKE_HUM, FAKE_PRESS };
		ticks = (uint8_t)((ticks + 1U) % FAKE_CYCLE_S);
		alarmUpdate(&fake);
		(void)snprintf(msg, sizeof(msg), "T=%d  estado: %s\r\n", (int)fake.temperature, name[alarmGetState()]);
		print(msg);
	}
}

/* -------------------------------------------------------------------------- */
#else  /* TEST_NONE: corre app_ui y este archivo queda vacío. */

void appTestInit(void)
{
}

void appTestUpdate(void)
{
}

#endif
