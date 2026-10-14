/**
 * @file    API_alarm.c
 * @brief   Alarma por temperatura y humedad fuera de rango.
 * @author  Elvis de la Torre
 * @date    4 oct 2026
 */

#include "API_alarm.h"
#include "API_gpio.h"
#include "API_delay.h"

#include <stddef.h>

/* Configuración privada -----------------------------------------------------*/

#define BUZZER_PERIOD_MS   500U

#define TEMP_RANGE_MIN    -40.0f   /**< lo que mide el BME280 */
#define TEMP_RANGE_MAX     85.0f
#define HUM_RANGE_MIN       0.0f
#define HUM_RANGE_MAX     100.0f

#define TEMP_HYST           0.3f   /**< unas décimas más que el ruido del sensor */
#define HUM_HYST            2.0f   /**< la humedad fluctúa bastante más */

#define TEMP_DEFAULT_MIN   10.0f
#define TEMP_DEFAULT_MAX   35.0f
#define HUM_DEFAULT_MIN    20.0f
#define HUM_DEFAULT_MAX    80.0f

/* Tipos privados ------------------------------------------------------------*/

typedef struct {
	limits_t sensor;   /**< rango del sensor: los límites no pueden salir de acá */
	float    hyst;     /**< margen para salir de ACTIVE */
} varSpec_t;

/* Variables privadas --------------------------------------------------------*/

static const varSpec_t spec[VAR_COUNT] = {
	[VAR_TEMP] = { { TEMP_RANGE_MIN, TEMP_RANGE_MAX }, TEMP_HYST },
	[VAR_HUM]  = { { HUM_RANGE_MIN,  HUM_RANGE_MAX  }, HUM_HYST  },
};

static const limits_t defaults[VAR_COUNT] = {
	[VAR_TEMP] = { TEMP_DEFAULT_MIN, TEMP_DEFAULT_MAX },
	[VAR_HUM]  = { HUM_DEFAULT_MIN,  HUM_DEFAULT_MAX  },
};

static limits_t     limits[VAR_COUNT];
static bool_t       outOfRange[VAR_COUNT];
static alarmState_t state    = ALARM_DISABLED;
static delay_t      buzzerTimer;
static bool_t       buzzerOn = false;

/* Funciones privadas --------------------------------------------------------*/

static float varValue(const bme280Data_t *d, variable_t v);
static void  outputsOn(void);
static void  outputsOff(void);

/* Funciones públicas --------------------------------------------------------*/

void alarmInit(void)
{
	for (uint8_t i = 0U; i < (uint8_t)VAR_COUNT; i++) {
		limits[i]     = defaults[i];
		outOfRange[i] = false;
	}
	state = ALARM_DISABLED;
	delayInit(&buzzerTimer, BUZZER_PERIOD_MS);
	outputsOff();
}

void alarmUpdate(const bme280Data_t *data)
{
	if (data == NULL) {
		return;
	}

	bool_t anyOut = false;   /**< alguna fuera de [min, max] */
	bool_t allIn  = true;    /**< todas dentro de [min + hyst, max - hyst] */

	for (uint8_t i = 0U; i < (uint8_t)VAR_COUNT; i++) {
		float x = varValue(data, (variable_t)i);
		float h = spec[i].hyst;

		outOfRange[i] = (x < limits[i].min) || (x > limits[i].max);
		anyOut = anyOut || outOfRange[i];
		if ((x < limits[i].min + h) || (x > limits[i].max - h)) {
			allIn = false;
		}
	}

	switch (state) {
	case ALARM_DISABLED:
		break;

	case ALARM_ARMED:
		if (anyOut) {
			state = ALARM_ACTIVE;
			outputsOn();
		}
		break;

	case ALARM_ACTIVE:
		if (allIn) {
			state = ALARM_ARMED;
			outputsOff();
		}
		break;

	default:
		alarmInit();
		break;
	}
}

void alarmSensorLost(void)
{
	for (uint8_t i = 0U; i < (uint8_t)VAR_COUNT; i++) {
		outOfRange[i] = false;
	}
	if (state == ALARM_ACTIVE) {
		state = ALARM_ARMED;
		outputsOff();
	}
}

void alarmTask(void)
{
	if (state != ALARM_ACTIVE) {
		return;
	}
	if (delayRead(&buzzerTimer)) {
		buzzerOn = !buzzerOn;
		gpioBuzzerWrite(buzzerOn);
	}
}

void alarmEnable(bool_t on)
{
	if (on) {
		if (state == ALARM_DISABLED) {
			state = ALARM_ARMED;   /**< se evalúa con la próxima medición */
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
	if (!(min < max) || min < spec[var].sensor.min || max > spec[var].sensor.max) {
		return false;
	}
	limits[var].min = min;
	limits[var].max = max;
	return true;
}

limits_t alarmGetLimits(variable_t var)
{
	static const limits_t none = { 0.0f, 0.0f };
	return (var < VAR_COUNT) ? limits[var] : none;
}

limits_t alarmGetRange(variable_t var)
{
	static const limits_t none = { 0.0f, 0.0f };
	return (var < VAR_COUNT) ? spec[var].sensor : none;
}

bool_t alarmIsOutOfRange(variable_t var)
{
	return (var < VAR_COUNT) ? outOfRange[var] : false;
}

alarmState_t alarmGetState(void)
{
	return state;
}

/* Funciones privadas --------------------------------------------------------*/

static float varValue(const bme280Data_t *d, variable_t v)
{
	return (v == VAR_TEMP) ? d->temperature : d->humidity;
}

/** @brief Salidas de ACTIVE: LED fijo y buzzer arrancando encendido. */
static void outputsOn(void)
{
	gpioLedOn();
	buzzerOn = true;
	gpioBuzzerWrite(true);
	delayInit(&buzzerTimer, BUZZER_PERIOD_MS);
}

/** @brief Salidas de DISABLED y ARMED: todo apagado. */
static void outputsOff(void)
{
	buzzerOn = false;
	gpioBuzzerWrite(false);
	gpioLedOff();
}
