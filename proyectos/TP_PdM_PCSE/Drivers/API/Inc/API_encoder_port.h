/*
 * API_encoder_port.h
 *
 *  Created on: 5 oct 2026
 *      Author: elvisdt
 *
 *  Capa port del encoder: un contador de 16 bits que avanza con el giro.
 *  En STM32 es un timer en modo encoder; en otro micro podría ser una
 *  interrupción por GPIO que cuente pasos.
 */

#ifndef API_INC_API_ENCODER_PORT_H_
#define API_INC_API_ENCODER_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

/** @brief Inicializa y arranca el contador del encoder. */
bool_t   encoderPortInit(void);

/** @brief Valor actual del contador (16 bits, da la vuelta). */
uint16_t encoderPortGetCount(void);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_ENCODER_PORT_H_ */
