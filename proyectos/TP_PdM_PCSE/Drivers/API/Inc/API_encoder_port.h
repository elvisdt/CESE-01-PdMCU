/**
 * @file    API_encoder_port.h
 * @brief   Port del encoder: un contador que avanza con el giro.
 * @author  Elvis de la Torre
 * @date    5 oct 2026
 *
 * En la NUCLEO el contador es un timer en modo encoder. En un micro sin esa
 * función podría ser una interrupción por GPIO que cuente flancos; solo
 * habría que reescribir API_encoder_port.c.
 */

#ifndef API_INC_API_ENCODER_PORT_H_
#define API_INC_API_ENCODER_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

/** @brief Inicializa y arranca el contador. */
bool_t encoderPortInit(void);

/** @brief Valor del contador. Es de 16 bits y da la vuelta. */
uint16_t encoderPortGetCount(void);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_ENCODER_PORT_H_ */
