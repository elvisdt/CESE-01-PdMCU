/*
 * API_common.h
 *
 *  Created on: 24 sept 2026
 *      Author: elvisdt
 *
 *  Tipos y utilidades comunes a todos los módulos API_* (sin dependencias de HW).
 */

#ifndef API_INC_API_COMMON_H_
#define API_INC_API_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t tick_t;	// use stdint.h
typedef bool bool_t;		// use stdbool.h

/**
 * @brief  Error irrecuperable de un módulo API: detiene el sistema.
 */
void Error_API_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_COMMON_H_ */
