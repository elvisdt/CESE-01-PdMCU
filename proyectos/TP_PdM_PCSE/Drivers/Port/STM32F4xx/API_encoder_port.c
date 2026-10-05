/*
 * API_encoder_port.c
 *
 *  Created on: 5 oct 2026
 *      Author: elvisdt
 *
 *  Port del encoder para NUCLEO-F446RE: TIM3 en modo encoder (PA6/PA7),
 *  inicializado por API_port.
 */

#include "API_encoder_port.h"
#include "API_port.h"

bool_t encoderPortInit(void)
{
	return portEncoderInit();
}

uint16_t encoderPortGetCount(void)
{
	return portEncoderGetCount();
}
