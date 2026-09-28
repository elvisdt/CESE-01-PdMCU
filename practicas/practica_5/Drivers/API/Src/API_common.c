/*
 * API_common.c
 *
 *  Created on: Sep 27, 2026
 *      Author: elvisdt
 */

#include "API_common.h"


void Error_API_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}
