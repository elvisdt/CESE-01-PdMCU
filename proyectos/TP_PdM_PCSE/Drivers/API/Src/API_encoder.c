/*
 * API_encoder.c
 *
 *  Created on: 4 oct 2026
 *      Author: elvisdt
 */

#include "API_encoder.h"
#include "API_encoder_port.h"

/* Private defines -----------------------------------------------------------*/
/* Cuentas del timer por cada "clic" del KY-040 en modo TI12.
 * Si con un clic avanza de a 2 o se salta pasos, ajustar a 2. */
#define ENC_COUNTS_PER_STEP   4

/* Private variables ---------------------------------------------------------*/
static uint16_t lastCount = 0U;
static int16_t  remainder = 0;     /* cuentas que todavía no completan un paso */

/* Public functions ----------------------------------------------------------*/

bool_t encoderInit(void)
{
	if (!encoderPortInit()) {
		return false;
	}
	lastCount = encoderPortGetCount();
	remainder = 0;
	return true;
}

int16_t encoderGetDelta(void)
{
	uint16_t now = encoderPortGetCount();

	/* resta en 16 bits: el cast a int16_t resuelve el desborde del contador */
	int16_t diff = (int16_t)(uint16_t)(now - lastCount);
	lastCount = now;

	remainder += diff;
	int16_t steps = remainder / ENC_COUNTS_PER_STEP;
	remainder    -= steps * ENC_COUNTS_PER_STEP;

	return steps;
}
