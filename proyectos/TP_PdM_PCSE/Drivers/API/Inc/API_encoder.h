/*
 * API_encoder.h
 *
 *  Created on: 4 oct 2026
 *      Author: elvisdt
 *
 *  Encoder rotativo KY-040 leído por hardware (TIM3 en modo encoder).
 */

#ifndef API_INC_API_ENCODER_H_
#define API_INC_API_ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

/**
 * @brief  Arranca el timer en modo encoder.
 * @retval true si se inicializó correctamente.
 */
bool_t  encoderInit(void);

/**
 * @brief  Pasos (detents) girados desde la última llamada.
 * @retval > 0 sentido horario, < 0 antihorario, 0 sin movimiento.
 */
int16_t encoderGetDelta(void);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_ENCODER_H_ */
