/*
 * API_delay.c
 *
 *  Created on: 10 sept 2026
 *      Author: elvisdt
 */


#include "API_delay.h"
#include "API_common.h"

// carga duration, running en false, no arranca el conteo
void delayInit(delay_t * delay, tick_t duration) {

	// valid delay
	if (delay == NULL) {
		Error_API_Handler();
		return;
	}

	if (duration <= 0) {
		Error_API_Handler();
		return;
	}

	delay->duration  = duration;
	delay->running   = false;
	delay->startTime = 0;
}


// retardo no bloqueante, devuelve true cuando se cumple duration
bool_t delayRead(delay_t * delay) {

	if (delay == NULL) {
		Error_API_Handler();
		return false;
	}

	// primera lectura del ciclo: arranca el conteo y termina
	if (!delay->running) {
		delay->startTime = HAL_GetTick();
		delay->running   = true;
		return false;
	}

	// ya está corriendo: ver si se cumplió la duración
	tick_t elapsedTime = HAL_GetTick() - delay->startTime;

	if (elapsedTime < delay->duration) {
		return false;
	}

	delay->running = false;
	return true;
}



// cambia duration, solo si el delay no esta corriendo
void delayWrite(delay_t * delay, tick_t duration) {

	if (delay == NULL) {
		Error_API_Handler();
		return;
	}

	if (duration == 0) {
		Error_API_Handler();
		return;
	}

	if (!delay->running) {
		delay->duration = duration;
	}
}


// Punto 3: devuelve una copia del estado running del delay
bool_t delayIsRunning(delay_t * delay) {

	if (delay == NULL) {
		Error_API_Handler();
		return false;
	}

	return delay->running;
}

