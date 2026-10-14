/**
 * @file    API_encoder_port.c
 * @brief   Port del encoder para la NUCLEO-F446RE: TIM3 en PA6/PA7.
 * @author  Elvis de la Torre
 * @date    5 oct 2026
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
