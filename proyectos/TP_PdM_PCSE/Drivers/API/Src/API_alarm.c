/*
 * API_alarm.c
 *
 *  Created on: 4 oct 2026
 *      Author: elvisdt
 */

#include "API_alarm.h"
#include "API_gpio.h"
#include "API_delay.h"

#include <stddef.h>

/* Private defines -----------------------------------------------------------*/
#define BUZZER_PERIOD_MS   500U

/* Private types -------------------------------------------------------------*/
typedef struct {
	float lo;      /* rango físico del sensor (para validar límites) */
	float hi;
	float hyst;    /* histéresis para salir de alarma                */
} varRange_t;

/* Private variables ---------------------------------------------------------*/
static const varRange_t range[VAR_COUNT] = {
	{ -40.0f,  85.0f,  0.5f },   /* temperatura °C */
	{   0.0f, 100.0f,  2.0f },   /* humedad %HR    */
	{ 300.0f, 1100.0f, 1.0f }    /* presión hPa    */
};

static limits_t limits[VAR_COUNT] = {
	{  10.0f,   35.0f },
	{  20.0f,   80.0f },
	{ 950.0f, 1050.0f }
};

static alarmState_t state = ALARM_DISABLED;
static bool_t       outOfRange[VAR_COUNT];
static delay_t      buzzerPeriod;
static bool_t       buzzerOn = false;

/* Private function prototypes -----------------------------------------------*/
static float  varValue(const bme280Data_t *d, variable_t v);
static void   outputsOff(void);

/* Public functions ----------------------------------------------------------*/

void alarmInit(void)
{
	state = ALARM_DISABLED;
	for (uint8_t i = 0; i < (uint8_t)VAR_COUNT; i++) {
		outOfRange[i] = false;
	}
	delayInit(&buzzerPeriod, BUZZER_PERIOD_MS);
	outputsOff();
}

void alarmUpdate(const bme280Data_t *data)
{
	if (data == NULL) {
		return;
	}

	bool_t anyOut = false;   /* alguna variable fuera de [min, max]        */
	bool_t allIn  = true;    /* todas dentro de [min + h, max - h]          */

	for (uint8_t i = 0; i < (uint8_t)VAR_COUNT; i++) {
		float x = varValue(data, (variable_t)i);
		float h = range[i].hyst;

		outOfRange[i] = (x < limits[i].min) || (x > limits[i].max);
		anyOut |= outOfRange[i];
		if ((x < limits[i].min + h) || (x > limits[i].max - h)) {
			allIn = false;
		}
	}

	switch (state) {
	case ALARM_DISABLED:
		break;                               /* no evalúa */

	case ALARM_ARMED:
		if (anyOut) {
			state = ALARM_ACTIVE;
			gpioLedOn();
			buzzerOn = true;
			gpioBuzzerWrite(true);
		}
		break;

	case ALARM_ACTIVE:
		if (allIn) {                         /* histéresis: evita oscilar en el borde */
			state = ALARM_ARMED;
			outputsOff();
		}
		break;

	default:
		alarmInit();
		break;
	}
}

void alarmTask(void)
{
	if (state != ALARM_ACTIVE) {
		return;
	}
	if (delayRead(&buzzerPeriod)) {
		buzzerOn = !buzzerOn;
		gpioBuzzerWrite(buzzerOn);
	}
}

void alarmEnable(bool_t on)
{
	if (on) {
		if (state == ALARM_DISABLED) {
			state = ALARM_ARMED;             /* se evalúa con la próxima medición */
		}
	} else {
		state = ALARM_DISABLED;
		outputsOff();
	}
}

void alarmToggle(void)
{
	alarmEnable(state == ALARM_DISABLED);
}

bool_t alarmSetLimits(variable_t var, float min, float max)
{
	if (var >= VAR_COUNT) {
		return false;
	}
	if (!(min < max) || min < range[var].lo || max > range[var].hi) {
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

bool_t alarmIsOutOfRange(variable_t var)
{
	return (var < VAR_COUNT) ? outOfRange[var] : false;
}

alarmState_t alarmGetState(void)
{
	return state;
}

/* Private functions ---------------------------------------------------------*/

static float varValue(const bme280Data_t *d, variable_t v)
{
	switch (v) {
	case VAR_TEMP:  return d->temperature;
	case VAR_HUM:   return d->humidity;
	default:        return d->pressure;
	}
}

static void outputsOff(void)
{
	buzzerOn = false;
	gpioLedOff();
	gpioBuzzerWrite(false);
}
