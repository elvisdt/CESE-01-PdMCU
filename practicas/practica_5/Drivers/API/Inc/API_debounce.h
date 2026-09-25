/*
 * API_debounce.h
 *
 *  Created on: 17 sept 2026
 *      Author: elvisdt
 */

#ifndef API_INC_API_DEBOUNCE_H_
#define API_INC_API_DEBOUNCE_H_

#include "API_delay.h"   /* trae bool_t */

/**
 * @brief  Carga el estado inicial de la MEF antirrebote.
 * @retval None
 */
void debounceFSM_init(void);

/**
 * @brief  Lee el pin del botón, resuelve la MEF y actualiza el flag interno.
 *         Debe llamarse periódicamente.
 * @retval None
 */
void debounceFSM_update(void);

/**
 * @brief  Indica si hubo una pulsación confirmada. Se resetea sola al leerla.
 * @retval bool_t: true si había una pulsación pendiente, false si no
 */
bool_t readKey(void);

#endif /* API_INC_API_DEBOUNCE_H_ */
