/*
 * API_gpio.h
 *
 *  Created on: 1 oct 2026
 *      Author: elvisdt
 *
 *  GPIO de la aplicación: LED de usuario (LD2) y pulsador de usuario (B1).
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
 * @brief  Lee el pulsador B1 (activo en bajo), sin antirrebote.
 * @retval true si está presionado.
 */
bool_t gpioButtonIsPressed(void);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_GPIO_H_ */
