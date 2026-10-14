/**
 * @file    app_fmt.c
 * @brief   Números con un decimal sin printf de float.
 * @author  Elvis de la Torre
 * @date    5 oct 2026
 */

#include "app_fmt.h"

#include <stdio.h>
#include <stdint.h>

#define FMT_SCALE   10   /**< un decimal */

void fmtFloat1(char *buf, size_t len, float v)
{
	if (buf == NULL || len == 0U) {
		return;
	}

	/* Se redondea alejándose de cero para que -0.05 dé "-0.1" y no "0.0". */
	int32_t x10 = (int32_t)((v < 0.0f) ? (v * FMT_SCALE - 0.5f) : (v * FMT_SCALE + 0.5f));
	const char *sign = (x10 < 0) ? "-" : "";
	if (x10 < 0) {
		x10 = -x10;
	}
	(void)snprintf(buf, len, "%s%ld.%ld", sign, (long)(x10 / FMT_SCALE), (long)(x10 % FMT_SCALE));
}
