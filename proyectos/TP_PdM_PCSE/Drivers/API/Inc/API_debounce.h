/*
 * API_debounce.h
 *
 *  Created on: 17 sept 2026
 *      Author: elvisdt
 *
 *  Antirrebote por MEF (práctica 4) adaptado a varios pulsadores:
 *  una instancia de la MEF por pulsador.
 */

#ifndef API_INC_API_DEBOUNCE_H_
#define API_INC_API_DEBOUNCE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

/* Pulsadores de la aplicación */
typedef enum {
	BTN_OK = 0,
	BTN_BACK,
	BTN_ENC_SW,
	BTN_COUNT
} button_t;

/**
 * @brief  Carga el estado inicial (BUTTON_UP) de la MEF de cada pulsador.
 */
void debounceFSM_init(void);

/**
 * @brief  Lee los pines, resuelve la MEF de cada pulsador y actualiza sus flags.
 *         Llamar en cada vuelta del lazo principal.
 */
void debounceFSM_update(void);

/**
 * @brief  Indica si hubo una pulsación confirmada. Se resetea al leerla.
 * @param  btn: pulsador a consultar.
 * @retval true si había una pulsación pendiente.
 */
bool_t readKey(button_t btn);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_DEBOUNCE_H_ */
