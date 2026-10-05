/*
 * API_lcd.c
 *
 *  Created on: 4 oct 2026
 *      Author: elvisdt
 *
 *  HD44780 en 4 bits sobre PCF8574. Mapeo del módulo I2C:
 *    P0 = RS, P1 = RW (siempre 0), P2 = EN, P3 = backlight, P4..P7 = D4..D7
 *
 *  Referencia: API_lcd.c de Israel Pavelek (PCSE). Cambios: validación de
 *  parámetros, cada envío devuelve estado y se propaga el error, sin
 *  retardos de 1 ms por nibble (la transferencia I2C ya dura más que lo
 *  que pide el HD44780).
 */

#include "API_lcd.h"
#include "API_lcd_port.h"

#include <stddef.h>

/* Bits del PCF8574 ----------------------------------------------------------*/
#define LCD_RS            0x01U
#define LCD_EN            0x04U
#define LCD_BACKLIGHT     0x08U

/* Comandos HD44780 ----------------------------------------------------------*/
#define CMD_CLEAR         0x01U
#define CMD_ENTRY_MODE    0x04U
#define CMD_DISPLAY_CTRL  0x08U
#define CMD_FUNCTION_SET  0x20U
#define CMD_SET_DDRAM     0x80U

#define ENTRY_INCREMENT   0x02U
#define DISPLAY_ON        0x04U
#define CURSOR_ON         0x02U
#define CURSOR_BLINK      0x01U
#define FUNCTION_4BIT_2L  0x08U        /* DL=0 (4 bits), N=1 (2 líneas), F=0 (5x8) */

#define INIT_NIBBLE_8BIT  0x03U        /* secuencia de reset por software */
#define INIT_NIBBLE_4BIT  0x02U

/* Tiempos (ms) --------------------------------------------------------------*/
#define T_POWER_ON_MS     50U          /* > 40 ms tras alimentar          */
#define T_INIT1_MS        5U           /* > 4.1 ms                        */
#define T_INIT2_MS        1U           /* > 100 us                        */
#define T_CLEAR_MS        2U           /* clear / home: 1.52 ms           */

/* Direcciones DDRAM de cada fila */
static const uint8_t rowAddr[LCD_ROWS] = { 0x00U, 0x40U };

/* Private variables ---------------------------------------------------------*/
static bool_t lcdReady = false;

/* Private function prototypes -----------------------------------------------*/
static lcdStatus_t lcdSendNibble(uint8_t nibble, bool_t rs);
static lcdStatus_t lcdSendByte(uint8_t value, bool_t rs);
static lcdStatus_t lcdCommand(uint8_t cmd);

/* Public functions ----------------------------------------------------------*/

lcdStatus_t lcdInit(void)
{
	lcdReady = false;

	if (!lcdPortInit()) {
		return LCD_ERR_PORT;
	}

	/* Reset por software (datasheet HD44780, fig. 24) */
	lcdPortDelayMs(T_POWER_ON_MS);
	if (lcdSendNibble(INIT_NIBBLE_8BIT, false) != LCD_OK) return LCD_ERR_PORT;
	lcdPortDelayMs(T_INIT1_MS);
	if (lcdSendNibble(INIT_NIBBLE_8BIT, false) != LCD_OK) return LCD_ERR_PORT;
	lcdPortDelayMs(T_INIT2_MS);
	if (lcdSendNibble(INIT_NIBBLE_8BIT, false) != LCD_OK) return LCD_ERR_PORT;
	if (lcdSendNibble(INIT_NIBBLE_4BIT, false) != LCD_OK) return LCD_ERR_PORT;

	/* Configuración ya en 4 bits */
	static const uint8_t initCmds[] = {
		CMD_FUNCTION_SET | FUNCTION_4BIT_2L,
		CMD_DISPLAY_CTRL,                        /* display off            */
		CMD_ENTRY_MODE | ENTRY_INCREMENT,        /* cursor avanza a derecha */
		CMD_DISPLAY_CTRL | DISPLAY_ON            /* display on, sin cursor */
	};
	for (uint8_t i = 0; i < sizeof(initCmds); i++) {
		if (lcdCommand(initCmds[i]) != LCD_OK) {
			return LCD_ERR_PORT;
		}
	}

	lcdReady = true;
	return lcdClear();
}

lcdStatus_t lcdClear(void)
{
	if (!lcdReady) {
		return LCD_ERR_NOT_INIT;
	}
	lcdStatus_t st = lcdCommand(CMD_CLEAR);
	lcdPortDelayMs(T_CLEAR_MS);
	return st;
}

lcdStatus_t lcdSetCursor(uint8_t row, uint8_t col)
{
	if (!lcdReady) {
		return LCD_ERR_NOT_INIT;
	}
	if (row >= LCD_ROWS || col >= LCD_COLS) {
		return LCD_ERR_PARAM;
	}
	return lcdCommand((uint8_t)(CMD_SET_DDRAM | (rowAddr[row] + col)));
}

lcdStatus_t lcdWriteChar(char c)
{
	if (!lcdReady) {
		return LCD_ERR_NOT_INIT;
	}
	return lcdSendByte((uint8_t)c, true);
}

lcdStatus_t lcdPrint(const char *text)
{
	if (text == NULL) {
		return LCD_ERR_PARAM;
	}
	if (!lcdReady) {
		return LCD_ERR_NOT_INIT;
	}
	for (uint8_t n = 0; (n < LCD_COLS) && (text[n] != '\0'); n++) {
		lcdStatus_t st = lcdSendByte((uint8_t)text[n], true);
		if (st != LCD_OK) {
			return st;
		}
	}
	return LCD_OK;
}

lcdStatus_t lcdCursor(bool_t on)
{
	if (!lcdReady) {
		return LCD_ERR_NOT_INIT;
	}
	uint8_t cmd = CMD_DISPLAY_CTRL | DISPLAY_ON;
	if (on) {
		cmd |= CURSOR_ON | CURSOR_BLINK;
	}
	return lcdCommand(cmd);
}

/* Private functions ---------------------------------------------------------*/

/* Pone el nibble en D4..D7 y da un pulso en EN (flanco de bajada = captura) */
static lcdStatus_t lcdSendNibble(uint8_t nibble, bool_t rs)
{
	uint8_t data = (uint8_t)(((nibble & 0x0FU) << 4) | LCD_BACKLIGHT | (rs ? LCD_RS : 0U));

	if (!lcdPortWriteByte(data | LCD_EN)) {
		return LCD_ERR_PORT;
	}
	if (!lcdPortWriteByte(data)) {
		return LCD_ERR_PORT;
	}
	return LCD_OK;
}

static lcdStatus_t lcdSendByte(uint8_t value, bool_t rs)
{
	lcdStatus_t st = lcdSendNibble((uint8_t)(value >> 4), rs);   /* parte alta */
	if (st != LCD_OK) {
		return st;
	}
	return lcdSendNibble((uint8_t)(value & 0x0FU), rs);          /* parte baja */
}

static lcdStatus_t lcdCommand(uint8_t cmd)
{
	return lcdSendByte(cmd, false);
}
