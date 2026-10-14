/**
 * @file    API_lcd.h
 * @brief   LCD 16x2 (HD44780) en modo 4 bits a través de un PCF8574.
 * @author  Elvis de la Torre
 * @date    4 oct 2026
 *
 * Parte del API_lcd del ejemplo de Israel Pavelek en Protocolos. Respecto
 * de ese ejemplo, cada función valida sus parámetros y devuelve un estado,
 * y cada byte del LCD viaja en una sola transferencia I2C en vez de cuatro.
 *
 * El acceso al bus lo resuelve API_lcd_port, así que el mismo driver sirve
 * para un LCD por GPIO o por SPI cambiando solo el port.
 */

#ifndef API_INC_API_LCD_H_
#define API_INC_API_LCD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

#define LCD_ROWS          2U   /**< Filas del display. */
#define LCD_COLS          16U  /**< Columnas del display. */
#define LCD_CUSTOM_CHARS  8U   /**< Caracteres propios en CGRAM, códigos 0 a 7. */
#define LCD_CHAR_ROWS     8U   /**< Filas de puntos de cada caracter. */

/** @brief Resultado de las funciones del LCD. */
typedef enum {
	LCD_OK = 0,        /**< Terminó bien. */
	LCD_ERR_PORT,      /**< El port no pudo escribir: el PCF8574 no respondió. */
	LCD_ERR_PARAM,     /**< Parámetro inválido. */
	LCD_ERR_NOT_INIT   /**< Se llamó antes de lcdInit(). */
} lcdStatus_t;

/**
 * @brief  Inicializa el display: 4 bits, 2 líneas, sin cursor, pantalla limpia.
 *
 * Bloquea unos 60 ms por los tiempos que pide el HD44780 al encender; se
 * llama una sola vez al arrancar.
 */
lcdStatus_t lcdInit(void);

/** @brief Borra la pantalla y lleva el cursor a (0, 0). Bloquea 2 ms. */
lcdStatus_t lcdClear(void);

/**
 * @brief Mueve el cursor.
 * @param row Fila, de 0 a LCD_ROWS - 1.
 * @param col Columna, de 0 a LCD_COLS - 1.
 */
lcdStatus_t lcdSetCursor(uint8_t row, uint8_t col);

/** @brief Escribe un caracter donde está el cursor. */
lcdStatus_t lcdWriteChar(char c);

/**
 * @brief Escribe un texto desde el cursor.
 * @param text Texto (no NULL). Se escriben como mucho LCD_COLS caracteres.
 */
lcdStatus_t lcdPrint(const char *text);

/** @brief Muestra (true) u oculta el cursor parpadeante. */
lcdStatus_t lcdCursor(bool_t on);

/**
 * @brief Carga un caracter propio en la CGRAM.
 *
 * Después se imprime como cualquier otro, con el código slot. El 0 conviene
 * no usarlo porque en un string de C marca el final.
 *
 * @param slot    De 0 a LCD_CUSTOM_CHARS - 1.
 * @param pattern LCD_CHAR_ROWS filas; los 5 bits bajos de cada una son los
 *                puntos, de izquierda a derecha.
 * @note  Deja el cursor apuntando a la CGRAM: hay que llamar a
 *        lcdSetCursor() antes de volver a escribir texto.
 */
lcdStatus_t lcdCreateChar(uint8_t slot, const uint8_t pattern[LCD_CHAR_ROWS]);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_LCD_H_ */
