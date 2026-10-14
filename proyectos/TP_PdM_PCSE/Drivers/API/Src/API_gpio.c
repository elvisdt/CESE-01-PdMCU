/**
 * @file    API_gpio.c
 * @brief   Salidas de la estación: LED de alarma y buzzer.
 * @author  Elvis de la Torre
 * @date    1 oct 2026
 */

#include "API_gpio.h"
#include "API_port.h"

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

void gpioBuzzerWrite(bool_t on)
{
	portBuzzerWrite(on);
}
