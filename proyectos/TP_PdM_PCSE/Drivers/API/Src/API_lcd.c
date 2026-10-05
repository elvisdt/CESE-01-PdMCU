/*
 * API_lcd.c
 *
 *  Created on: 4 oct 2026
 *      Author: elvisdt
 *
 *  Mapeo típico del PCF8574 en los módulos I2C de LCD:
 *    P0 = RS, P1 = RW, P2 = EN, P3 = backlight, P4..P7 = D4..D7
 */

#include "API_lcd.h"
#include "API_port.h"

#include <stddef.h>

/* Private defines -----------------------------------------------------------*/
#define LCD_RS          0x01U
#define LCD_EN          0x04U
#define LCD_BACKLIGHT   0x08U
#define LCD_I2C_TIMEOUT 10U

/* Private function prototypes -----------------------------------------------*/
static lcdStatus_t lcdSendNibble(uint8_t nibble, bool_t rs);
static lcdStatus_t lcdSendByte(uint8_t byte, bool_t rs);

/* Public functions ----------------------------------------------------------*/

lcdStatus_t lcdInit(void)
{
	if (!portI2cIsReady(LCD_I2C_ADDR)) {
		return LCD_ERR_I2C;
	}
	/* TODO: secuencia de inicialización HD44780 en 4 bits:
	 *   esperar >40 ms, 0x3 (x3 con 4.1 ms / 100 us), 0x2 -> modo 4 bits,
	 *   0x28 (2 líneas, 5x8), 0x0C (display ON), 0x06 (entry mode), 0x01 (clear).
	 *   Usar portDelayMs() solo acá, en la inicialización. */
	return LCD_ERR_I2C;
}

lcdStatus_t lcdClear(void)
{
	return lcdSendByte(0x01U, false);   /* TODO: esperar ~2 ms tras el clear */
}

lcdStatus_t lcdSetCursor(uint8_t row, uint8_t col)
{
	if (row >= LCD_ROWS || col >= LCD_COLS) {
		return LCD_ERR_PARAM;
	}
	static const uint8_t rowAddr[LCD_ROWS] = { 0x00U, 0x40U };
	return lcdSendByte((uint8_t)(0x80U | (rowAddr[row] + col)), false);
}

lcdStatus_t lcdPrint(const char *text)
{
	if (text == NULL) {
		return LCD_ERR_PARAM;
	}
	for (; *text != '\0'; text++) {
		lcdStatus_t st = lcdSendByte((uint8_t)*text, true);
		if (st != LCD_OK) {
			return st;
		}
	}
	return LCD_OK;
}

/* Private functions ---------------------------------------------------------*/

/* TODO: implementar. Escribe el nibble en P4..P7 con EN=1 y luego EN=0. */
static lcdStatus_t lcdSendNibble(uint8_t nibble, bool_t rs)
{
	(void)nibble;
	(void)rs;
	return LCD_ERR_I2C;
}

static lcdStatus_t lcdSendByte(uint8_t byte, bool_t rs)
{
	lcdStatus_t st = lcdSendNibble((uint8_t)(byte >> 4), rs);
	if (st != LCD_OK) {
		return st;
	}
	return lcdSendNibble((uint8_t)(byte & 0x0FU), rs);
}
