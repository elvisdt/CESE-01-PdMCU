/*
 * API_uart.c
 *
 *  Created on: 24 sept 2026
 *      Author: elvisdt
 */

#include "API_uart.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <stddef.h>

#define UART_INSTANCE        USART2
#define UART_BAUDRATE        115200U
#define UART_TX_TIMEOUT_MS   1000U
#define UART_RX_TIMEOUT_MS   10U
#define UART_CFG_MSG_LEN     64U

static UART_HandleTypeDef uartHandle;
static bool_t             uartReady = false;

/* Funciones privadas */
static bool_t   uartIsValidSize(uint32_t size);
static uint32_t uartStrLen(const uint8_t * pstring, uint32_t maxLen);
static void     uartTransmit(uint8_t * pdata, uint16_t size);
static void     uartPrintConfig(void);

/* ------------------------------------------------------------------------- */

bool_t uartInit(void)
{
	uartHandle.Instance          = UART_INSTANCE;
	uartHandle.Init.BaudRate     = UART_BAUDRATE;
	uartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
	uartHandle.Init.StopBits     = UART_STOPBITS_1;
	uartHandle.Init.Parity       = UART_PARITY_NONE;
	uartHandle.Init.Mode         = UART_MODE_TX_RX;
	uartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
	uartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

	if (HAL_UART_Init(&uartHandle) != HAL_OK) {
		uartReady = false;
		return false;
	}

	uartReady = true;
	uartPrintConfig();
	return true;
}

void uartSendString(uint8_t * pstring)
{
	if (pstring == NULL) {
		return;
	}

	/* strlen acotado: evita recorrer memoria si falta el '\0' */
	uint32_t len = uartStrLen(pstring, UART_MAX_SIZE + 1U);

	if (!uartIsValidSize(len)) {
		return;
	}

	uartTransmit(pstring, (uint16_t)len);
}

void uartSendStringSize(uint8_t * pstring, uint16_t size)
{
	if (pstring == NULL || !uartIsValidSize(size)) {
		return;
	}

	uartTransmit(pstring, size);
}

void uartReceiveStringSize(uint8_t * pstring, uint16_t size)
{
	if (pstring == NULL || !uartIsValidSize(size) || !uartReady) {
		return;
	}

	HAL_StatusTypeDef status = HAL_UART_Receive(&uartHandle, pstring, size, UART_RX_TIMEOUT_MS);

	/* HAL_TIMEOUT = no llegaron datos, no es error */
	if (status != HAL_OK && status != HAL_TIMEOUT) {
		Error_API_Handler();
	}
}

/* ------------------------------------------------------------------------- */

/* true si size está en [UART_MIN_SIZE, UART_MAX_SIZE] */
static bool_t uartIsValidSize(uint32_t size)
{
	return (size >= UART_MIN_SIZE) && (size <= UART_MAX_SIZE);
}

/* Longitud hasta '\0' o hasta maxLen */
static uint32_t uartStrLen(const uint8_t * pstring, uint32_t maxLen)
{
	uint32_t len = 0U;

	while ((len < maxLen) && (pstring[len] != '\0')) {
		len++;
	}
	return len;
}

/* Transmite y verifica el retorno de la HAL */
static void uartTransmit(uint8_t * pdata, uint16_t size)
{
	if (!uartReady) {
		return;
	}

	if (HAL_UART_Transmit(&uartHandle, pdata, size, UART_TX_TIMEOUT_MS) != HAL_OK) {
		Error_API_Handler();
	}
}

/* Imprime la configuración de la UART */
static void uartPrintConfig(void)
{
	static uint8_t msg[UART_CFG_MSG_LEN];

	int n = snprintf((char *)msg, sizeof(msg),
			"\r\nHello! UART2 init: %lu baudios, 8N1\r\n",
			(unsigned long)uartHandle.Init.BaudRate);

	if (n > 0) {
		uartSendString(msg);
	}
}
