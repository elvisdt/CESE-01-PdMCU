/**
 * @file    API_delay.h
 * @brief   Retardos no bloqueantes (práctica 3).
 * @author  Elvis de la Torre
 * @date    10 sept 2026
 *
 * Un delay_t no frena el programa: se consulta en cada vuelta del lazo con
 * delayRead() y devuelve true una sola vez cuando se cumple el tiempo. En la
 * consulta siguiente vuelve a arrancar, así que sirve tanto para un evento
 * único como para algo periódico.
 */

#ifndef API_API_DELAY_H_
#define API_API_DELAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

/** @brief Estado de un retardo. Se usa solo a través de las funciones del módulo. */
typedef struct {
	tick_t startTime;   /**< Tick en que arrancó la cuenta. */
	tick_t duration;    /**< Duración en ms. */
	bool_t running;     /**< true mientras está contando. */
} delay_t;

/**
 * @brief Carga la duración y deja el retardo detenido.
 * @param delay    Retardo a inicializar (no NULL).
 * @param duration Duración en ms (mayor que 0).
 */
void delayInit(delay_t *delay, tick_t duration);

/**
 * @brief Consulta el retardo sin bloquear.
 *
 * La primera llamada arranca la cuenta. Las siguientes devuelven false hasta
 * que pasa la duración; ahí devuelve true y el retardo queda detenido.
 *
 * @param  delay Retardo (no NULL).
 * @return true si se cumplió el tiempo.
 */
bool_t delayRead(delay_t *delay);

/**
 * @brief Cambia la duración. Si el retardo está corriendo no hace nada.
 * @param delay    Retardo (no NULL).
 * @param duration Nueva duración en ms (mayor que 0).
 */
void delayWrite(delay_t *delay, tick_t duration);

/**
 * @brief Indica si el retardo está contando.
 * @param  delay Retardo (no NULL).
 * @return true si está corriendo.
 */
bool_t delayIsRunning(delay_t *delay);

#ifdef __cplusplus
}
#endif

#endif /* API_API_DELAY_H_ */
