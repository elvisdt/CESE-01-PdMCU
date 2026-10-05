/*
 * API_alarm.h
 *
 *  Created on: 4 oct 2026
 *      Author: elvisdt
 *
 *  MEF de alarma: DISABLED -> ARMED -> ACTIVE.
 *   - alarmUpdate(): evalúa cada nueva medición contra sus límites.
 *   - alarmTask():   maneja LED y buzzer (no bloqueante, llamar en cada vuelta).
 */

#ifndef API_INC_API_ALARM_H_
#define API_INC_API_ALARM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"
#include "API_bme280.h"

typedef enum { ALARM_DISABLED = 0, ALARM_ARMED, ALARM_ACTIVE } alarmState_t;
typedef enum { VAR_TEMP = 0, VAR_HUM, VAR_PRESS, VAR_COUNT } variable_t;
typedef struct { float min; float max; } limits_t;

void         alarmInit(void);                                      /* DISABLED, límites por defecto */
void         alarmUpdate(const bme280Data_t *data);                /* evalúa una medición nueva     */
void         alarmTask(void);                                      /* LED y buzzer                  */
void         alarmEnable(bool_t on);                               /* habilita / deshabilita        */
void         alarmToggle(void);
bool_t       alarmSetLimits(variable_t var, float min, float max); /* valida min < max y rango      */
limits_t     alarmGetLimits(variable_t var);
bool_t       alarmIsOutOfRange(variable_t var);                    /* última evaluación de esa var  */
alarmState_t alarmGetState(void);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_ALARM_H_ */
