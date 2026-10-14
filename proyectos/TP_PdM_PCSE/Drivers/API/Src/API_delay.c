/**
 * @file    API_delay.c
 * @brief   Retardos no bloqueantes.
 * @author  Elvis de la Torre
 * @date    10 sept 2026
 */

#include "API_delay.h"
#include "API_port.h"

#include <stddef.h>

void delayInit(delay_t *delay, tick_t duration)
{
	if (delay == NULL || duration == 0U) {
		Error_API_Handler();
		return;
	}

	delay->duration  = duration;
	delay->running   = false;
	delay->startTime = 0U;
}

bool_t delayRead(delay_t *delay)
{
	if (delay == NULL) {
		Error_API_Handler();
		return false;
	}

	if (!delay->running) {
		delay->startTime = portGetTickMs();
		delay->running   = true;
		return false;
	}

	/* La resta en tick_t da bien aunque el contador haya dado la vuelta. */
	tick_t elapsed = portGetTickMs() - delay->startTime;

	if (elapsed < delay->duration) {
		return false;
	}

	delay->running = false;
	return true;
}

void delayWrite(delay_t *delay, tick_t duration)
{
	if (delay == NULL || duration == 0U) {
		Error_API_Handler();
		return;
	}

	/* Cambiarla a mitad de la cuenta mezclaría dos duraciones distintas. */
	if (!delay->running) {
		delay->duration = duration;
	}
}

bool_t delayIsRunning(delay_t *delay)
{
	if (delay == NULL) {
		Error_API_Handler();
		return false;
	}

	return delay->running;
}
