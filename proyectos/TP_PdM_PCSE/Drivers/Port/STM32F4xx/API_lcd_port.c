/*
 * API_lcd_port.c
 *
 *  Created on: 5 oct 2026
 *      Author: elvisdt
 *
 *  Implementación del port del LCD para NUCLEO-F446RE: el PCF8574 cuelga del
 *  bus I2C1 compartido con el BME280, que maneja API_port (un solo handle,
 *  sin variables globales externas).
 */

#include "API_lcd_port.h"
#include "API_port.h"

#define LCD_I2C_TIMEOUT_MS   10U

bool_t lcdPortInit(void)
{
	return portI2cIsReady(LCD_I2C_ADDR);
}

bool_t lcdPortWriteByte(uint8_t value)
{
	return (portI2cWrite(LCD_I2C_ADDR, &value, 1U, LCD_I2C_TIMEOUT_MS) == PORT_OK);
}

void lcdPortDelayMs(uint32_t ms)
{
	portDelayMs(ms);
}
