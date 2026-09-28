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
#define UART_BAUDRATE        115200U // init baud
#define UART_TX_TIMEOUT_MS   1000U
#define UART_RX_TIMEOUT_MS   10U
#define UART_CFG_MSG_LEN     256U

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

	/* HAL_TIMEOUT: no llegaron datos.
	 * HAL_ERROR: overrun (se perdió un byte); la HAL ya limpió el flag,
	 * no se bloquea el sistema por eso. Solo HAL_BUSY es un error real. */
	if (status == HAL_BUSY) {
		Error_API_Handler();
	}
}


//---------------------------------------------//
uint32_t uartGetBaudrate(void)
{
	return uartHandle.Init.BaudRate;
}

bool_t uartSetBaudrate(uint32_t baudrate)
{
	if (baudrate < UART_BAUD_MIN || baudrate > UART_BAUD_MAX || !uartReady) {
		return false;
	}

	uint32_t oldBaud = uartHandle.Init.BaudRate;

	if (HAL_UART_DeInit(&uartHandle) != HAL_OK) {
		Error_API_Handler();
	}

	uartHandle.Init.BaudRate = baudrate;

	if (HAL_UART_Init(&uartHandle) != HAL_OK) {
		/* no se pudo: se vuelve al baudrate anterior */
		uartHandle.Init.BaudRate = oldBaud;
		if (HAL_UART_Init(&uartHandle) != HAL_OK) {
			uartReady = false;
			Error_API_Handler();
		}
		return false;
	}

	uartPrintConfig();
	return true;
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

/* Imprime los parámetros de configuración de la UART (leídos del handle) */
static void uartPrintConfig(void)
{
	static uint8_t msg[UART_CFG_MSG_LEN];
	UART_InitTypeDef *cfg = &uartHandle.Init;

	int n = snprintf((char *)msg, sizeof(msg),
			"\r\n--- UART2 init OK ---\r\n"
			"  Baudrate     : %lu\r\n"
			"  Word length  : %s\r\n"
			"  Parity       : %s\r\n"
			"  Stop bits    : %s\r\n"
			"  Flow control : %s\r\n"
			"  Mode         : %s\r\n",
			(unsigned long)cfg->BaudRate,
			(cfg->WordLength == UART_WORDLENGTH_9B) ? "9 bits" : "8 bits",
			(cfg->Parity == UART_PARITY_NONE) ? "none" :
			(cfg->Parity == UART_PARITY_EVEN) ? "even" : "odd",
			(cfg->StopBits == UART_STOPBITS_2) ? "2" : "1",
			(cfg->HwFlowCtl == UART_HWCONTROL_NONE) ? "none" : "RTS/CTS",
			(cfg->Mode == UART_MODE_TX_RX) ? "TX/RX" :
			(cfg->Mode == UART_MODE_TX) ? "TX" : "RX");

	if (n > 0) {
		uartSendString(msg);
	}
}
