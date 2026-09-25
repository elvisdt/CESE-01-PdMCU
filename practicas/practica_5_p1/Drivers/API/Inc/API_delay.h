/*
 * API_delay.h
 *
 *  Created on: 10 sept 2026
 *      Author: elvisdt
 */

#ifndef API_API_DELAY_H_
#define API_API_DELAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>


typedef uint32_t tick_t;	// use stdint.h
typedef bool bool_t;		// use stdbool.h

typedef struct{
	tick_t startTime;
	tick_t duration;
	bool_t running;
} delay_t;

/**
 * @brief  Inicializa el retardo con una duración (ms) y lo deja detenido.
 * @param  delay: puntero a la estructura delay_t
 * @param  duration: duración en milisegundos
 * @retval None
 */
void delayInit( delay_t * delay, tick_t duration );

/**
 * @brief  Retardo no bloqueante: arranca al primer llamado, luego avisa si venció.
 * @param  delay: puntero a la estructura delay_t
 * @retval bool_t: true si se cumplió la duración, false si sigue corriendo
 */
bool_t delayRead( delay_t * delay );

/**
 * @brief  Cambia la duración del retardo, solo si no está corriendo.
 * @param  delay: puntero a la estructura delay_t
 * @param  duration: nueva duración en milisegundos
 * @retval None
 */
void delayWrite( delay_t * delay, tick_t duration );

/**
 * @brief  Indica si el retardo está corriendo.
 * @param  delay: puntero a la estructura delay_t
 * @retval bool_t: true si está corriendo, false si no
 */
bool_t delayIsRunning(delay_t * delay);



void Error_APIdelay_Handler(void);


#ifdef __cplusplus
}
#endif


#include <stdint.h>
#include <stdbool.h>

#endif /* API_API_DELAY_H_ */
