/**
 * @file    API_encoder.c
 * @brief   Encoder rotativo KY-040.
 * @author  Elvis de la Torre
 * @date    4 oct 2026
 */

#include "API_encoder.h"
#include "API_encoder_port.h"

/* Configuración privada -----------------------------------------------------*/

/** Cuentas del timer por cada clic. En modo TI12 se cuentan los flancos de
 *  los dos canales, 4 por clic en el KY-040. */
#define ENC_COUNTS_PER_STEP   4

/* Variables privadas --------------------------------------------------------*/

static uint16_t lastCount = 0U;
static int16_t  pending   = 0;   /**< cuentas que todavía no llegan a un clic */

/* Funciones públicas --------------------------------------------------------*/

bool_t encoderInit(void)
{
	if (!encoderPortInit()) {
		return false;
	}
	lastCount = encoderPortGetCount();
	pending   = 0;
	return true;
}

int16_t encoderGetDelta(void)
{
	uint16_t now = encoderPortGetCount();

	/* El contador es de 16 bits y da la vuelta. Restando en 16 bits y
	 * pasando a signed, 65535 -> 0 da +1 y no -65535. */
	int16_t diff = (int16_t)(uint16_t)(now - lastCount);
	lastCount = now;

	/* Las cuentas que no completan un clic quedan para la próxima: si se
	 * descartaran, girar despacio no avanzaría nunca. */
	pending += diff;
	int16_t steps = (int16_t)(pending / ENC_COUNTS_PER_STEP);
	pending = (int16_t)(pending - steps * ENC_COUNTS_PER_STEP);

	return steps;
}
