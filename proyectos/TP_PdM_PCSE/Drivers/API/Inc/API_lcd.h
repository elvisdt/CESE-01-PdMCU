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

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_LCD_H_ */
