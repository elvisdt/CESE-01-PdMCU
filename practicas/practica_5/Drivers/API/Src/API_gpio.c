/*
 * API_gpio.c
 *
 *  Created on: 1 oct 2026
 *      Author: elvisdt
 */

#include "API_gpio.h"
#include "API_port.h"

/* LED LD2 -------------------------------------------------------------------*/

void gpioLedOn(void)
{
	portLedWrite(true);
}

void gpioLedOff(void)
{
	portLedWrite(false);
}

void gpioLedToggle(void)
{
	portLedToggle();
}

bool_t gpioLedIsOn(void)
{
	return portLedRead();
}

/* Pulsador B1 ---------------------------------------------------------------*/

bool_t gpioButtonIsPressed(void)
{
	return portButtonRead();
}
