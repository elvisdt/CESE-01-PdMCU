/**
 * @file    API_lcd_port.h
 * @brief   Port del LCD: cómo llegan los bytes al PCF8574.
 * @author  Elvis de la Torre
 * @date    5 oct 2026
 *
 * API_lcd.c usa solo estas funciones. Para otro micro, o para un display
 * conectado directo por GPIO, se reescribe únicamente API_lcd_port.c.
 */

#ifndef API_INC_API_LCD_PORT_H_
#define API_INC_API_LCD_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

#define LCD_I2C_ADDR   0x27U   /**< Dirección del PCF8574. Algunos módulos vienen en 0x3F. */

/**
 * @brief  Verifica que el PCF8574 responda. El bus ya tiene que estar iniciado.
 * @return true si respondió.
 */
bool_t lcdPortInit(void);

/**
 * @brief  Escribe varios bytes seguidos en los pines del PCF8574.
 * @param  data Bytes a escribir (no NULL).
 * @param  len  Cantidad (mayor que 0).
 * @return true si la transferencia terminó bien.
 */
bool_t lcdPortWrite(const uint8_t *data, uint16_t len);

/** @brief Retardo bloqueante para los tiempos de inicialización y de clear. */
void lcdPortDelayMs(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_LCD_PORT_H_ */
