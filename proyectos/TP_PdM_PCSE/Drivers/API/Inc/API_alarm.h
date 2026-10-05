/*
 * API_alarm.h
 *
 *  Created on: 4 oct 2026
 *      Author: elvisdt
 *
 *  MEF de alarma: DISABLED -> ARMED -> ACTIVE. Compara cada medición con sus
 *  límites y maneja LED y buzzer.
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

void         alarmInit(void);                                      /* estado y límites iniciales */
void         alarmUpdate(const bme280Data_t *data);                /* actualiza la MEF           */
void         alarmToggle(void);                                    /* habilita / deshabilita     */
bool_t       alarmSetLimits(variable_t var, float min, float max); /* valida (min < max) y guarda */
limits_t     alarmGetLimits(variable_t var);
alarmState_t alarmGetState(void);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_ALARM_H_ */
