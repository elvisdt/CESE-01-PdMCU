/*
 * app_test.c
 *
 *  Created on: 4 oct 2026
 *      Author: elvisdt
 */

#include "app_test.h"

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

void appTestInit(void)
{
	print("TEST_UART: consola de la practica 5\r\n");
	cmdParserInit();
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
	print(encoderInit() ? "TEST_ENCODER: girar el encoder\r\n"
	                    : "TEST_ENCODER: ERROR, habilitar TIM3 en CubeMX\r\n");
}

void appTestUpdate(void)
{
	int16_t d = encoderGetDelta();
	if (d != 0) {
		total += d;
		snprintf(msg, sizeof(msg), "delta=%d total=%ld\r\n", d, (long)total);
		print(msg);
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

void appTestInit(void)
{
	print("TEST_LCD\r\n");
	if (!portI2cInit() || lcdInit() != LCD_OK) {
		print("ERROR: LCD no responde o lcdInit() sin implementar\r\n");
		return;
	}
	lcdSetCursor(0, 0);
	lcdPrint("Hola PdM");
	lcdSetCursor(1, 0);
	lcdPrint("CESE - FIUBA");
	print("OK\r\n");
}

void appTestUpdate(void)
{
}

/* -------------------------------------------------------------------------- */
#elif APP_TEST == TEST_BME280

void appTestInit(void)
{
	uint8_t id = 0;

	print("TEST_BME280\r\n");
	if (!portI2cInit() || bme280ReadChipId(&id) != BME280_OK) {
		print("ERROR: BME280 no responde en 0x76\r\n");
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

	if (delayRead(&period)) {
		if (bme280Read(&d) == BME280_OK) {
			/* printf de float requiere -u _printf_float; se imprime en décimas */
			snprintf(msg, sizeof(msg), "T=%ld.%ld C  H=%ld %%  P=%ld hPa\r\n",
					(long)(d.temperature), (long)(d.temperature * 10) % 10,
					(long)d.humidity, (long)d.pressure);
			print(msg);
		} else {
			print("bme280Read(): sin implementar o error\r\n");
		}
	}
}

/* -------------------------------------------------------------------------- */
#elif APP_TEST == TEST_ALARM

void appTestInit(void)
{
	print("TEST_ALARM: OK habilita/deshabilita\r\n");
	debounceFSM_init();
	alarmInit();
	delayInit(&period, 1000U);
}

void appTestUpdate(void)
{
	static const char *name[] = { "DISABLED", "ARMED", "ACTIVE" };
	/* medición simulada fuera de rango para forzar ACTIVE */
	bme280Data_t fake = { 40.0f, 50.0f, 1000.0f };

	debounceFSM_update();
	if (readKey(BTN_OK)) {
		alarmToggle();
	}
	if (delayRead(&period)) {
		alarmUpdate(&fake);
		snprintf(msg, sizeof(msg), "estado: %s\r\n", name[alarmGetState()]);
		print(msg);
	}
}

/* -------------------------------------------------------------------------- */
#else  /* TEST_NONE: no se usa este archivo */

void appTestInit(void) {}
void appTestUpdate(void) {}

#endif
