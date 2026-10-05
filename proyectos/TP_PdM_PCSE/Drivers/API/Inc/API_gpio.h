/*
 * API_gpio.h
 *
 *  Created on: 1 oct 2026
 *      Author: elvisdt
 *
 *  Salidas de la aplicación: LED de alarma (LD2, PA5) y buzzer (PB10).
 *  Los pulsadores se leen en API_debounce.
 *  Portable: el acceso al hardware lo resuelve API_port.
 */

#ifndef API_INC_API_GPIO_H_
#define API_INC_API_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

/**
 * @brief  Enciende el LED LD2.
 */
void gpioLedOn(void);

/**
 * @brief  Apaga el LED LD2.
 */
void gpioLedOff(void);

/**
 * @brief  Invierte el estado del LED LD2.
 */
void gpioLedToggle(void);

/**
 * @brief  Lee el estado actual del LED LD2.
 * @retval true si está encendido, false si está apagado.
 */
bool_t gpioLedIsOn(void);

/**
 * @brief  Enciende o apaga el buzzer.
 */
void gpioBuzzerWrite(bool_t on);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_GPIO_H_ */
