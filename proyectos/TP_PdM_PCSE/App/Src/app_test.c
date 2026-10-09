/*
 * app_test.c
 *
 *  Created on: 4 oct 2026
 *      Author: elvisdt
 *
 *  Una prueba por módulo. Se elige con APP_TEST en app_test.h.
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
#include "API_lcd_port.h"
#include "API_bme280.h"
#include "API_alarm.h"
#include "API_port.h"

#include <stdio.h>
#include <string.h>

#define TEST_MSG_LEN   96U

/* no todas las pruebas usan todo: se marcan como "unused" para evitar warnings */
static char    msg[TEST_MSG_LEN] __attribute__((unused));
static delay_t period __attribute__((unused));

__attribute__((unused)) static void print(const char *s)
{
	uartSendString((uint8_t *)s);
}

/* -------------------------------------------------------------------------- */
#if APP_TEST == TEST_DELAY

void appTestInit(void)
{
	print("TEST_DELAY: LD2 a 1 Hz\r\n");
	delayInit(&period, 500U);
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

/* comandos de la práctica 5 sobre el parser generalizado */
static cmdStatus_t cmdLed(uint8_t argc, char *argv[])
{
	(void)argc;
	if      (strcmp(argv[1], "ON") == 0)     gpioLedOn();
	else if (strcmp(argv[1], "OFF") == 0)    gpioLedOff();
	else if (strcmp(argv[1], "TOGGLE") == 0) gpioLedToggle();
	else return CMD_ERR_ARG;
	print("OK\r\n");
	return CMD_OK;
}

static cmdStatus_t cmdStatus(uint8_t argc, char *argv[])
{
	(void)argc; (void)argv;
	print(gpioLedIsOn() ? "LED is ON\r\n" : "LED is OFF\r\n");
	return CMD_OK;
}

static const cmdEntry_t testCmds[] = {
	{ "LED",    1, 1, cmdLed,    "LED <ON|OFF|TOGGLE>" },
	{ "STATUS", 0, 0, cmdStatus, "STATUS" },
};

void appTestInit(void)
{
	print("TEST_UART: consola\r\n");
	cmdParserInit(testCmds, (uint8_t)(sizeof(testCmds) / sizeof(testCmds[0])));
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
	print("TEST_DEBOUNCE: presionar OK / BACK / SW (sin labels, OK = B1)\r\n");
	debounceFSM_init();
}

void appTestUpdate(void)
{
	static const char *name[BTN_COUNT] = { "OK", "BACK", "SW" };

	debounceFSM_update();
	for (uint8_t i = 0; i < (uint8_t)BTN_COUNT; i++) {
		if (readKey((button_t)i)) {
			count[i]++;
			snprintf(msg, sizeof(msg), "%s: %u\r\n", name[i], count[i]);
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
	                    : "TEST_ENCODER: ERROR, habilitar TIM3 en CubeMX\r\n");
	debounceFSM_init();
}

void appTestUpdate(void)
{
	int16_t d = encoderGetDelta();
	if (d != 0) {
		total += d;
		snprintf(msg, sizeof(msg), "delta=%d total=%ld\r\n", d, (long)total);
		print(msg);
	}

	debounceFSM_update();
	if (readKey(BTN_ENC_SW)) {
		print("SW presionado\r\n");
	}
}

/* -------------------------------------------------------------------------- */
#elif APP_TEST == TEST_I2C_SCAN

void appTestInit(void)
{
	print("TEST_I2C_SCAN\r\n");
	if (!portI2cInit()) {
		print("ERROR: habilitar I2C1 en CubeMX\r\n");
		return;
	}
	uint8_t found = 0;
	for (uint8_t a = 0x08U; a <= 0x77U; a++) {
		if (portI2cIsReady(a)) {
			snprintf(msg, sizeof(msg), "  dispositivo en 0x%02X\r\n", a);
			print(msg);
			found++;
		}
	}
	snprintf(msg, sizeof(msg), "%u dispositivo(s). Esperados: 0x27 (LCD), 0x76 (BME280)\r\n", found);
	print(msg);
}

void appTestUpdate(void)
{
}

/* -------------------------------------------------------------------------- */
#elif APP_TEST == TEST_LCD

static uint32_t seconds = 0;

void appTestInit(void)
{
	print("TEST_LCD\r\n");
	if (!portI2cInit() || lcdInit() != LCD_OK) {
		print("ERROR: el LCD no responde (0x27). Probar 0x3F en API_lcd_port.h\r\n");
		return;
	}
	lcdSetCursor(0, 0);
	lcdPrint("Hola PdM");
	lcdSetCursor(1, 0);
	lcdPrint("CESE - FIUBA");
	print("OK: en la fila 2 corre un contador de segundos\r\n");
	delayInit(&period, 1000U);
}

void appTestUpdate(void)
{
	if (delayRead(&period)) {
		seconds++;
		snprintf(msg, sizeof(msg), "%5lu", (unsigned long)seconds);
		lcdSetCursor(1, 11);
		lcdPrint(msg);
	}
}

/* -------------------------------------------------------------------------- */
#elif APP_TEST == TEST_BME280

void appTestInit(void)
{
	uint8_t id = 0;

	print("TEST_BME280\r\n");
	if (!portI2cInit() || bme280ReadChipId(&id) != BME280_OK) {
		print("ERROR: el BME280 no responde (0x76). Probar 0x77 en API_bme280_port.h\r\n");
		return;
	}
	snprintf(msg, sizeof(msg), "chip ID = 0x%02X (esperado 0x60)\r\n", id);
	print(msg);
	if (bme280Init() != BME280_OK) {
		print("ERROR: bme280Init()\r\n");
	}
	delayInit(&period, 1000U);
}

void appTestUpdate(void)
{
	bme280Data_t d;
	char t[8], h[8], p[8];

	if (delayRead(&period)) {
		if (bme280Read(&d) == BME280_OK) {
			fmtFloat1(t, sizeof(t), d.temperature);
			fmtFloat1(h, sizeof(h), d.humidity);
			fmtFloat1(p, sizeof(p), d.pressure);
			snprintf(msg, sizeof(msg), "T=%s C  H=%s %%  P=%s hPa\r\n", t, h, p);
			print(msg);
		} else {
			print("ERROR: bme280Read()\r\n");
		}
	}
}

/* -------------------------------------------------------------------------- */
#elif APP_TEST == TEST_ALARM

static uint8_t ticks = 0;

void appTestInit(void)
{
	print("TEST_ALARM: OK habilita/deshabilita. Dato simulado: 5 s fuera de rango, 5 s dentro\r\n");
	debounceFSM_init();
	alarmInit();
	delayInit(&period, 1000U);
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
		/* temperatura simulada: 40 °C (fuera de 10..35) y luego 25 °C */
		bme280Data_t fake = { (ticks < 5U) ? 40.0f : 25.0f, 50.0f, 1000.0f };
		ticks = (uint8_t)((ticks + 1U) % 10U);
		alarmUpdate(&fake);
		snprintf(msg, sizeof(msg), "T=%d  estado: %s\r\n", (int)fake.temperature, name[alarmGetState()]);
		print(msg);
	}
}

/* -------------------------------------------------------------------------- */
#else  /* TEST_NONE: no se usa este archivo */

void appTestInit(void) {}
void appTestUpdate(void) {}

#endif
