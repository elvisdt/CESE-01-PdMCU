/*
 * API_commun.h
 *
 *  Created on: 24 sept 2026
 *      Author: elvisdt
 */

#ifndef API_INC_API_COMMON_H_
#define API_INC_API_COMMON_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>


typedef uint32_t tick_t;	// use stdint.h
typedef bool bool_t;		// use stdbool.h


void Error_API_Handler(void);

#endif /* API_INC_API_COMMON_H_ */
