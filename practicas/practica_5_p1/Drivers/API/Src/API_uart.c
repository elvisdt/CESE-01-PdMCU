/*
 * API_uart.c
 *
 *  Created on: 24 sept 2026
 *      Author: elvisdt
 */

#include "API_uart.h"
#include "stm32f4xx_hal.h"
#include <string.h>


static UART_HandleTypeDef UartHandle;

bool_t uartInit(){
	UartHandle.Instance = USART1;
	UartHandle.Init.BaudRate = 115200;
	UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits = UART_STOPBITS_1;
	UartHandle.Init.Parity = UART_PARITY_ODD;
	UartHandle.Init.Mode = UART_MODE_TX_RX;
	UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&UartHandle) != HAL_OK){
	    return false;
	}
	return true;
}



void uartSendString(uint8_t *pstring)
{
    if (pstring == NULL) {
        return;
    }

    size_t len_data = strlen((const char *)pstring);

    if (len_data<1 || len_data >256 ) {
        return;
    }

    HAL_UART_Transmit(&UartHandle, pstring, (uint16_t)len_data, HAL_MAX_DELAY);
}


void uartSendStringSize(uint8_t * pstring, uint16_t size){
	if (pstring == NULL) {
	   return;
	}

	if (size<1 || size >256 ) {
	    return;
	}

	HAL_UART_Transmit(&UartHandle, pstring, size, HAL_MAX_DELAY);
}

void uartReceiveStringSize(uint8_t *pstring, uint16_t size)
{
    if (pstring == NULL || size == 0) {
        return;
    }

    HAL_UART_Receive(&UartHandle, pstring, size, HAL_MAX_DELAY);
}
