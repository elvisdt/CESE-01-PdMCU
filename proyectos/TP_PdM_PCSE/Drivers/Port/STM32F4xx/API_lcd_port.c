/**
 * @file    API_lcd_port.c
 * @brief   Port del LCD para la NUCLEO-F446RE: PCF8574 en el bus I2C1.
 * @author  Elvis de la Torre
 * @date    5 oct 2026
 *
 * El bus lo comparte con el BME280 y lo maneja API_port, que tiene el único
 * handle de I2C. Así ningún módulo necesita variables globales externas.
 */

#include "API_lcd_port.h"
#include "API_port.h"

#include <stddef.h>

/* Un texto de 16 caracteres son 64 bytes: unos 6 ms a 100 kHz. */
#define LCD_I2C_TIMEOUT_MS   20U

bool_t lcdPortInit(void)
{
	return portI2cIsReady(LCD_I2C_ADDR);
}

bool_t lcdPortWrite(const uint8_t *data, uint16_t len)
{
	if (data == NULL || len == 0U) {
		return false;
	}
	return (portI2cWrite(LCD_I2C_ADDR, data, len, LCD_I2C_TIMEOUT_MS) == PORT_OK);
}

void lcdPortDelayMs(uint32_t ms)
{
	portDelayMs(ms);
}
