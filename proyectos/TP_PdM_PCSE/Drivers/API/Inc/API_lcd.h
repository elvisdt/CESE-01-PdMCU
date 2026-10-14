/*
 * API_lcd.h
 *
 *  Created on: 4 oct 2026
 *      Author: elvisdt
 *
 *  LCD 16x2 HD44780 en modo 4 bits a través de un expansor PCF8574.
 *  Basado en API_lcd de Israel Pavelek (PCSE), con validación de parámetros
 *  y retorno de estado. Portable: el acceso al bus lo resuelve API_lcd_port.
 */

#ifndef API_INC_API_LCD_H_
#define API_INC_API_LCD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

#define LCD_ROWS   2U
#define LCD_COLS   16U
#define LCD_CUSTOM_CHARS  8U   /* caracteres propios en CGRAM (códigos 0..7) */
#define LCD_CHAR_ROWS     8U   /* filas de 5 puntos de cada caracter          */

typedef enum {
	LCD_OK = 0,
	LCD_ERR_PORT,      /* el port no pudo escribir (I2C sin ACK, timeout...) */
	LCD_ERR_PARAM,     /* parámetro inválido (NULL, fila/columna fuera de rango) */
	LCD_ERR_NOT_INIT   /* se llamó antes de lcdInit()                         */
} lcdStatus_t;

/**
 * @brief  Inicializa el LCD en modo 4 bits, 2 líneas, cursor apagado.
 *         Bloqueante (~60 ms): llamar solo al arrancar.
 */
lcdStatus_t lcdInit(void);

/** @brief Borra la pantalla y vuelve el cursor a (0,0). Bloquea ~2 ms. */
lcdStatus_t lcdClear(void);

/** @brief Posiciona el cursor. row: 0..LCD_ROWS-1, col: 0..LCD_COLS-1. */
lcdStatus_t lcdSetCursor(uint8_t row, uint8_t col);

/** @brief Escribe un caracter en la posición actual. */
lcdStatus_t lcdWriteChar(char c);

/** @brief Escribe un string (hasta '\0' o LCD_COLS caracteres). */
lcdStatus_t lcdPrint(const char *text);

/** @brief Muestra u oculta el cursor parpadeante. */
lcdStatus_t lcdCursor(bool_t on);

/**
 * @brief  Carga un caracter propio en la CGRAM del HD44780.
 * @param  slot: 0..LCD_CUSTOM_CHARS-1. Después se imprime con el código slot
 *         (conviene no usar el 0, que en un string de C es el fin de cadena).
 * @param  pattern: LCD_CHAR_ROWS filas, los 5 bits bajos de cada una son los puntos.
 * @note   Deja el cursor en la CGRAM: llamar a lcdSetCursor() antes de escribir texto.
 */
lcdStatus_t lcdCreateChar(uint8_t slot, const uint8_t pattern[LCD_CHAR_ROWS]);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_LCD_H_ */
