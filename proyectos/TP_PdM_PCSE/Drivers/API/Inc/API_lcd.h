/*
 * API_lcd.h
 *
 *  Created on: 4 oct 2026
 *      Author: elvisdt
 *
 *  LCD 16x2 HD44780 en modo 4 bits a través del expansor I2C PCF8574 (0x27).
 */

#ifndef API_INC_API_LCD_H_
#define API_INC_API_LCD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

#define LCD_I2C_ADDR   0x27U   /* algunos módulos vienen en 0x3F */
#define LCD_ROWS       2U
#define LCD_COLS       16U

typedef enum {
	LCD_OK = 0,
	LCD_ERR_I2C,
	LCD_ERR_PARAM
} lcdStatus_t;

lcdStatus_t lcdInit(void);                          /* secuencia de init en 4 bits     */
lcdStatus_t lcdClear(void);                         /* borra la pantalla               */
lcdStatus_t lcdSetCursor(uint8_t row, uint8_t col); /* fila 0..1, columna 0..15        */
lcdStatus_t lcdPrint(const char *text);             /* escribe desde la posición actual */

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_LCD_H_ */
