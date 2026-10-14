/**
 * @file    API_debounce.h
 * @brief   Antirrebote de los pulsadores.
 * @author  Elvis de la Torre
 * @date    17 sept 2026
 *
 * Es la MEF de las prácticas con una instancia por pulsador. Cada una
 * confirma el cambio solo si el pin sigue igual 40 ms después, y deja una
 * marca que readKey() consume una vez. Mantener apretado un botón genera un
 * solo evento.
 */

#ifndef API_INC_API_DEBOUNCE_H_
#define API_INC_API_DEBOUNCE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

/** @brief Pulsadores de la estación. */
typedef enum {
	BTN_OK = 0,   /**< Confirmar (PB10). */
	BTN_BACK,     /**< Volver (PB4). */
	BTN_ENC_SW,   /**< Eje del encoder (PB5). */
	BTN_COUNT     /**< Cantidad de pulsadores. */
} button_t;

/** @brief Deja todas las MEF en BUTTON_UP. */
void debounceFSM_init(void);

/** @brief Lee los pines y avanza las MEF. Se llama en cada vuelta del lazo. */
void debounceFSM_update(void);

/**
 * @brief  Indica si hubo una pulsación confirmada y la consume.
 * @param  btn Pulsador a consultar.
 * @return true una sola vez por pulsación.
 */
bool_t readKey(button_t btn);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_DEBOUNCE_H_ */
