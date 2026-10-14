/**
 * @file    API_encoder.h
 * @brief   Encoder rotativo KY-040.
 * @author  Elvis de la Torre
 * @date    4 oct 2026
 *
 * El conteo lo hace el hardware (TIM3 en modo encoder), así que no se
 * pierden pasos aunque el lazo esté ocupado. Este módulo convierte las
 * cuentas del timer en pasos del encoder, los "clics" que se sienten al
 * girarlo.
 */

#ifndef API_INC_API_ENCODER_H_
#define API_INC_API_ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

/**
 * @brief  Arranca el conteo.
 * @return false si el timer no se pudo iniciar.
 */
bool_t encoderInit(void);

/**
 * @brief  Pasos girados desde la llamada anterior.
 * @return Positivo en un sentido, negativo en el otro, 0 si no se movió.
 */
int16_t encoderGetDelta(void);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_ENCODER_H_ */
