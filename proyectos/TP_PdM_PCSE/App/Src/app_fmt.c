/*
 * app_fmt.c
 *
 *  Created on: 5 oct 2026
 *      Author: elvisdt
 */

#include "app_fmt.h"

#include <stdio.h>
#include <stdint.h>

void fmtFloat1(char *buf, size_t len, float v)
{
	if (buf == NULL || len == 0U) {
		return;
	}
	int32_t x10 = (int32_t)((v < 0.0f) ? (v * 10.0f - 0.5f) : (v * 10.0f + 0.5f));
	const char *sign = (x10 < 0) ? "-" : "";
	if (x10 < 0) {
		x10 = -x10;
	}
	snprintf(buf, len, "%s%ld.%ld", sign, (long)(x10 / 10), (long)(x10 % 10));
}
