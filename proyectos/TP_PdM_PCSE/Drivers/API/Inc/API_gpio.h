/**
 * @file    API_gpio.h
 * @brief   Salidas de la estación: LED de alarma y buzzer.
 * @author  Elvis de la Torre
 * @date    1 oct 2026
 *
 * El LED es LD2 de la placa (PA5) y el buzzer está en PA10. Los pulsadores,
 * que también son GPIO, se leen en API_debounce.
 */

#ifndef API_INC_API_GPIO_H_
#define API_INC_API_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

/** @brief Enciende el LED de alarma. */
void gpioLedOn(void);

/** @brief Apaga el LED de alarma. */
void gpioLedOff(void);

/** @brief Invierte el LED de alarma. */
void gpioLedToggle(void);

/**
 * @brief  Estado del LED de alarma.
 * @return true si está encendido.
 */
bool_t gpioLedIsOn(void);

/**
 * @brief Enciende o apaga el buzzer.
 * @param on true para encenderlo.
 */
void gpioBuzzerWrite(bool_t on);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_GPIO_H_ */
