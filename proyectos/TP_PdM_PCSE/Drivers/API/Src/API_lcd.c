/**
 * @file    API_lcd.c
 * @brief   LCD 16x2 (HD44780) en modo 4 bits a través de un PCF8574.
 * @author  Elvis de la Torre
 * @date    4 oct 2026
 *
 * Conexión del módulo I2C con el display:
 * P0 = RS, P1 = RW (siempre 0), P2 = EN, P3 = luz de fondo, P4 a P7 = D4 a D7.
 *
 * Cada byte del LCD se manda en dos mitades y cada mitad necesita un pulso
 * en EN: son cuatro escrituras al PCF8574. Antes iba cada una en su propia
 * transferencia I2C; ahora se arman en un buffer y salen juntas, y un texto
 * completo sale en una sola transferencia. Entre un byte I2C y el siguiente
 * pasan unos 90 µs a 100 kHz, más que los 37 µs que necesita el HD44780
 * para cada caracter, así que no hacen falta retardos.
 */

#include "API_lcd.h"
#include "API_lcd_port.h"

#include <stddef.h>

/* Pines del PCF8574 ---------------------------------------------------------*/

#define PIN_RS            0x01U
#define PIN_EN            0x04U
#define PIN_BACKLIGHT     0x08U

/* Instrucciones del HD44780 -------------------------------------------------*/

#define CMD_CLEAR         0x01U
#define CMD_ENTRY_MODE    0x04U
#define CMD_DISPLAY_CTRL  0x08U
#define CMD_FUNCTION_SET  0x20U
#define CMD_SET_CGRAM     0x40U
#define CMD_SET_DDRAM     0x80U

#define ENTRY_INCREMENT   0x02U
#define DISPLAY_ON        0x04U
#define CURSOR_ON         0x02U
#define CURSOR_BLINK      0x01U
#define FUNCTION_4BIT_2L  0x08U   /**< 4 bits, 2 líneas, caracteres de 5x8 */

#define INIT_NIBBLE_8BIT  0x03U   /**< reset por software del datasheet */
#define INIT_NIBBLE_4BIT  0x02U
#define CGRAM_ROW_MASK    0x1FU   /**< cada fila de un caracter tiene 5 puntos */

/* Tiempos del datasheet (ms) ------------------------------------------------*/

#define T_POWER_ON_MS     50U     /**< más de 40 ms desde que se alimenta */
#define T_INIT1_MS        5U      /**< más de 4,1 ms */
#define T_INIT2_MS        1U      /**< más de 100 µs */
#define T_CLEAR_MS        2U      /**< clear tarda 1,52 ms */

/* Tamaños -------------------------------------------------------------------*/

#define WRITES_PER_NIBBLE 2U      /**< EN arriba y EN abajo */
#define WRITES_PER_BYTE   (2U * WRITES_PER_NIBBLE)
#define TEXT_BUF_LEN      (LCD_COLS * WRITES_PER_BYTE)

/* Variables privadas --------------------------------------------------------*/

static const uint8_t rowAddr[LCD_ROWS] = { 0x00U, 0x40U };   /**< DDRAM de cada fila */
static bool_t        lcdReady = false;

/* Funciones privadas --------------------------------------------------------*/

static uint8_t     lcdPackNibble(uint8_t *buf, uint8_t nibble, bool_t rs);
static uint8_t     lcdPackByte(uint8_t *buf, uint8_t value, bool_t rs);
static lcdStatus_t lcdSendNibble(uint8_t nibble);
static lcdStatus_t lcdSendByte(uint8_t value, bool_t rs);
static lcdStatus_t lcdCommand(uint8_t cmd);

/* Funciones públicas --------------------------------------------------------*/

lcdStatus_t lcdInit(void)
{
	lcdReady = false;

	if (!lcdPortInit()) {
		return LCD_ERR_PORT;
	}

	/* Reset por software (figura 24 del datasheet): no se sabe en qué modo
	 * quedó el display, se lo lleva a 8 bits y después a 4. */
	lcdPortDelayMs(T_POWER_ON_MS);
	if (lcdSendNibble(INIT_NIBBLE_8BIT) != LCD_OK) {
		return LCD_ERR_PORT;
	}
	lcdPortDelayMs(T_INIT1_MS);
	if (lcdSendNibble(INIT_NIBBLE_8BIT) != LCD_OK) {
		return LCD_ERR_PORT;
	}
	lcdPortDelayMs(T_INIT2_MS);
	if (lcdSendNibble(INIT_NIBBLE_8BIT) != LCD_OK ||
	    lcdSendNibble(INIT_NIBBLE_4BIT) != LCD_OK) {
		return LCD_ERR_PORT;
	}

	static const uint8_t initCmds[] = {
		CMD_FUNCTION_SET | FUNCTION_4BIT_2L,
		CMD_DISPLAY_CTRL,                   /* apagado mientras se configura */
		CMD_ENTRY_MODE | ENTRY_INCREMENT,   /* el cursor avanza a la derecha */
		CMD_DISPLAY_CTRL | DISPLAY_ON       /* encendido, sin cursor         */
	};
	for (uint8_t i = 0U; i < (uint8_t)sizeof(initCmds); i++) {
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
	uint8_t buf[TEXT_BUF_LEN];
	uint8_t len = 0U;

	if (text == NULL) {
		return LCD_ERR_PARAM;
	}
	if (!lcdReady) {
		return LCD_ERR_NOT_INIT;
	}
	for (uint8_t n = 0U; (n < LCD_COLS) && (text[n] != '\0'); n++) {
		len = (uint8_t)(len + lcdPackByte(&buf[len], (uint8_t)text[n], true));
	}
	if (len == 0U) {
		return LCD_OK;
	}
	return lcdPortWrite(buf, len) ? LCD_OK : LCD_ERR_PORT;
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

lcdStatus_t lcdCreateChar(uint8_t slot, const uint8_t pattern[LCD_CHAR_ROWS])
{
	uint8_t buf[LCD_CHAR_ROWS * WRITES_PER_BYTE];
	uint8_t len = 0U;

	if (pattern == NULL || slot >= LCD_CUSTOM_CHARS) {
		return LCD_ERR_PARAM;
	}
	if (!lcdReady) {
		return LCD_ERR_NOT_INIT;
	}

	/* Cada caracter ocupa LCD_CHAR_ROWS bytes de CGRAM a partir de slot * 8. */
	if (lcdCommand((uint8_t)(CMD_SET_CGRAM | (slot * LCD_CHAR_ROWS))) != LCD_OK) {
		return LCD_ERR_PORT;
	}
	for (uint8_t i = 0U; i < LCD_CHAR_ROWS; i++) {
		len = (uint8_t)(len + lcdPackByte(&buf[len], (uint8_t)(pattern[i] & CGRAM_ROW_MASK), true));
	}
	return lcdPortWrite(buf, len) ? LCD_OK : LCD_ERR_PORT;
}

/* Funciones privadas --------------------------------------------------------*/

/**
 * @brief  Arma las dos escrituras de medio byte: con EN en alto y en bajo.
 *         El HD44780 toma el dato en el flanco de bajada de EN.
 * @return Bytes agregados a buf.
 */
static uint8_t lcdPackNibble(uint8_t *buf, uint8_t nibble, bool_t rs)
{
	uint8_t pins = (uint8_t)(((nibble & 0x0FU) << 4) | PIN_BACKLIGHT | (rs ? PIN_RS : 0U));

	buf[0] = (uint8_t)(pins | PIN_EN);
	buf[1] = pins;
	return WRITES_PER_NIBBLE;
}

/** @brief Arma las cuatro escrituras de un byte, primero la mitad alta. */
static uint8_t lcdPackByte(uint8_t *buf, uint8_t value, bool_t rs)
{
	uint8_t n = lcdPackNibble(buf, (uint8_t)(value >> 4), rs);
	return (uint8_t)(n + lcdPackNibble(&buf[n], value, rs));
}

/** @brief Manda medio byte de instrucción. Solo se usa en el reset. */
static lcdStatus_t lcdSendNibble(uint8_t nibble)
{
	uint8_t buf[WRITES_PER_NIBBLE];
	uint8_t len = lcdPackNibble(buf, nibble, false);
	return lcdPortWrite(buf, len) ? LCD_OK : LCD_ERR_PORT;
}

/** @brief Manda un byte completo en una sola transferencia. */
static lcdStatus_t lcdSendByte(uint8_t value, bool_t rs)
{
	uint8_t buf[WRITES_PER_BYTE];
	uint8_t len = lcdPackByte(buf, value, rs);
	return lcdPortWrite(buf, len) ? LCD_OK : LCD_ERR_PORT;
}

static lcdStatus_t lcdCommand(uint8_t cmd)
{
	return lcdSendByte(cmd, false);
}
