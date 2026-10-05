/*
 * API_alarm.c
 *
 *  Created on: 4 oct 2026
 *      Author: elvisdt
 */

#include "API_alarm.h"
#include "API_gpio.h"

/* Private variables ---------------------------------------------------------*/
static alarmState_t state = ALARM_DISABLED;
static limits_t     limits[VAR_COUNT] = {
	{ 10.0f,   35.0f  },   /* temperatura °C (valores por defecto) */
	{ 20.0f,   80.0f  },   /* humedad %HR                          */
	{ 950.0f,  1050.0f}    /* presión hPa                          */
};

/* Public functions ----------------------------------------------------------*/

void alarmInit(void)
{
	state = ALARM_DISABLED;
	gpioLedOff();
	gpioBuzzerWrite(false);
}

void alarmUpdate(const bme280Data_t *data)
{
	(void)data;
	/* TODO: MEF
	 *   ARMED  -> ACTIVE si alguna variable sale de [min, max]
	 *   ACTIVE -> ARMED  si todas vuelven al rango (con histéresis)
	 *   ACTIVE: LED encendido, buzzer intermitente con delay_t de 500 ms */
}

void alarmToggle(void)
{
	if (state == ALARM_DISABLED) {
		state = ALARM_ARMED;
	} else {
		state = ALARM_DISABLED;
		gpioLedOff();
		gpioBuzzerWrite(false);
	}
}

bool_t alarmSetLimits(variable_t var, float min, float max)
{
	if (var >= VAR_COUNT || !(min < max)) {
		return false;
	}
	limits[var].min = min;
	limits[var].max = max;
	return true;
}

limits_t alarmGetLimits(variable_t var)
{
	if (var >= VAR_COUNT) {
		limits_t none = { 0.0f, 0.0f };
		return none;
	}
	return limits[var];
}

alarmState_t alarmGetState(void)
{
	return state;
}
