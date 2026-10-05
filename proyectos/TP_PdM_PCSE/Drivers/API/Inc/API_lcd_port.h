/*
 * API_lcd_port.h
 *
 *  Created on: 5 oct 2026
 *      Author: elvisdt
 *
 *  Capa port del LCD: cómo llega un byte al PCF8574. API_lcd.c solo usa
 *  estas funciones; para otro micro o para un LCD en paralelo por GPIO se
 *  reescribe únicamente API_lcd_port.c.
 */

#ifndef API_INC_API_LCD_PORT_H_
#define API_INC_API_LCD_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

#define LCD_I2C_ADDR   0x27U   /* algunos módulos vienen en 0x3F */

/** @brief true si el PCF8574 responde en LCD_I2C_ADDR (el bus ya debe estar iniciado). */
bool_t lcdPortInit(void);

/** @brief Escribe un byte en los pines P0..P7 del PCF8574. */
bool_t lcdPortWriteByte(uint8_t value);

/** @brief Retardo bloqueante, solo para inicialización y clear. */
void   lcdPortDelayMs(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_LCD_PORT_H_ */
