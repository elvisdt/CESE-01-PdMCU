/**
 * @file    API_alarm.h
 * @brief   Alarma por temperatura y humedad fuera de rango.
 * @author  Elvis de la Torre
 * @date    4 oct 2026
 *
 * Es una MEF de Moore de tres estados: DISABLED, ARMED y ACTIVE. Las salidas
 * dependen solo del estado: en ACTIVE el LED queda encendido y el buzzer
 * suena intermitente; en los otros dos, ambos apagados.
 *
 * Cada medición nueva se evalúa con alarmUpdate(). El buzzer lo maneja
 * alarmTask(), que se llama en cada vuelta del lazo y no bloquea.
 *
 * Solo la temperatura y la humedad tienen límites. La presión se muestra
 * pero no dispara la alarma.
 */

#ifndef API_INC_API_ALARM_H_
#define API_INC_API_ALARM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"
#include "API_bme280.h"

/** @brief Estados de la alarma. */
typedef enum {
	ALARM_DISABLED = 0,   /**< No evalúa. */
	ALARM_ARMED,          /**< Evalúa cada medición. */
	ALARM_ACTIVE          /**< Alguna variable salió de su rango. */
} alarmState_t;

/** @brief Variables que tienen límites. */
typedef enum {
	VAR_TEMP = 0,   /**< Temperatura, °C. */
	VAR_HUM,        /**< Humedad, %HR. */
	VAR_COUNT       /**< Cantidad de variables. */
} variable_t;

/** @brief Un rango cerrado [min, max]. */
typedef struct {
	float min;   /**< Límite inferior. */
	float max;   /**< Límite superior. */
} limits_t;

/** @brief Arranca deshabilitada, con los límites por defecto y las salidas apagadas. */
void alarmInit(void);

/**
 * @brief Evalúa una medición nueva.
 *
 * Entra en ACTIVE si alguna variable sale de su rango. Sale recién cuando
 * todas vuelven con un margen de histéresis, para que una medición que
 * oscila justo en el borde no haga sonar y callar el buzzer cada segundo.
 *
 * @param data Medición (no NULL).
 */
void alarmUpdate(const bme280Data_t *data);

/**
 * @brief Avisa que no hay medición válida (sensor desconectado).
 *
 * Una alarma activa vuelve a ARMED y apaga las salidas: sin dato no se
 * puede afirmar que la variable siga fuera de rango.
 */
void alarmSensorLost(void);

/** @brief Hace sonar el buzzer en ACTIVE. Se llama en cada vuelta del lazo. */
void alarmTask(void);

/**
 * @brief Habilita (ARMED) o deshabilita (DISABLED) la alarma.
 * @param on true para habilitarla.
 */
void alarmEnable(bool_t on);

/** @brief Alterna entre habilitada y deshabilitada. */
void alarmToggle(void);

/**
 * @brief  Cambia los límites de una variable.
 * @return false si min >= max o si quedan fuera de lo que mide el sensor.
 */
bool_t alarmSetLimits(variable_t var, float min, float max);

/** @brief Límites actuales de una variable. */
limits_t alarmGetLimits(variable_t var);

/** @brief Rango que puede medir el sensor para esa variable. */
limits_t alarmGetRange(variable_t var);

/** @brief true si en la última evaluación esa variable estaba fuera de rango. */
bool_t alarmIsOutOfRange(variable_t var);

/** @brief Estado actual de la alarma. */
alarmState_t alarmGetState(void);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_ALARM_H_ */
