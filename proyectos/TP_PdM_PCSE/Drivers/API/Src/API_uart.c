/**
 * @file    API_uart.c
 * @brief   UART de consola (práctica 5).
 * @author  Elvis de la Torre
 * @date    24 sept 2026
 */

#include "API_uart.h"
#include "API_port.h"

#include <stdio.h>
#include <stddef.h>

/* Configuración privada -----------------------------------------------------*/

#define UART_TX_TIMEOUT_MS    1000U
#define UART_RX_TIMEOUT_MS    10U     /**< uartReceiveStringSize() */
#define UART_RX_BYTE_WAIT_MS  0U      /**< uartReceiveByte() no frena el lazo */
#define UART_CFG_MSG_LEN      256U

/* Variables privadas --------------------------------------------------------*/

static uint32_t uartBaud  = 0U;
static bool_t   uartReady = false;

/* Funciones privadas --------------------------------------------------------*/

static bool_t   uartIsValidSize(uint32_t size);
static uint32_t uartStrLen(const uint8_t *pstring, uint32_t maxLen);
static void     uartTransmit(const uint8_t *pdata, uint16_t size);
static void     uartPrintConfig(void);

/* Funciones públicas --------------------------------------------------------*/

bool_t uartInit(void)
{
	if (!portUartInit(UART_BAUD_INIT)) {
		uartReady = false;
		return false;
	}

	uartBaud  = UART_BAUD_INIT;
	uartReady = true;
	uartPrintConfig();
	return true;
}

void uartSendString(uint8_t *pstring)
{
	if (pstring == NULL) {
		return;
	}

	/* Se mide con tope: si faltara el '\0' no se recorre toda la memoria. */
	uint32_t len = uartStrLen(pstring, UART_MAX_SIZE + 1U);

	if (uartIsValidSize(len)) {
		uartTransmit(pstring, (uint16_t)len);
	}
}

void uartSendStringSize(uint8_t *pstring, uint16_t size)
{
	if (pstring == NULL || !uartIsValidSize(size)) {
		return;
	}

	uartTransmit(pstring, size);
}

void uartReceiveStringSize(uint8_t *pstring, uint16_t size)
{
	if (pstring == NULL || !uartIsValidSize(size) || !uartReady) {
		return;
	}

	/* Que no llegue nada a tiempo es normal en una consola. Solo BUSY indica
	 * un problema del driver. */
	if (portUartRead(pstring, size, UART_RX_TIMEOUT_MS) == PORT_BUSY) {
		Error_API_Handler();
	}
}

bool_t uartReceiveByte(uint8_t *pbyte)
{
	if (pbyte == NULL || !uartReady) {
		return false;
	}

	port_status_t st = portUartRead(pbyte, 1U, UART_RX_BYTE_WAIT_MS);

	if (st == PORT_BUSY) {
		Error_API_Handler();
	}
	return (st == PORT_OK);
}

uint32_t uartGetBaudrate(void)
{
	return uartBaud;
}

bool_t uartSetBaudrate(uint32_t baudrate)
{
	if (baudrate < UART_BAUD_MIN || baudrate > UART_BAUD_MAX || !uartReady) {
		return false;
	}

	if (!portUartDeInit()) {
		Error_API_Handler();
	}

	if (!portUartInit(baudrate)) {
		/* Se vuelve al anterior para no quedarse sin consola. */
		if (!portUartInit(uartBaud)) {
			uartReady = false;
			Error_API_Handler();
		}
		return false;
	}

	uartBaud = baudrate;
	uartPrintConfig();
	return true;
}

/* Funciones privadas --------------------------------------------------------*/

/** @brief true si size está entre UART_MIN_SIZE y UART_MAX_SIZE. */
static bool_t uartIsValidSize(uint32_t size)
{
	return (size >= UART_MIN_SIZE) && (size <= UART_MAX_SIZE);
}

/** @brief Largo del string hasta el '\0', sin pasar de maxLen. */
static uint32_t uartStrLen(const uint8_t *pstring, uint32_t maxLen)
{
	uint32_t len = 0U;

	while ((len < maxLen) && (pstring[len] != '\0')) {
		len++;
	}
	return len;
}

/** @brief Transmite y detiene el sistema si el port falla. */
static void uartTransmit(const uint8_t *pdata, uint16_t size)
{
	if (!uartReady) {
		return;
	}

	if (portUartWrite(pdata, size, UART_TX_TIMEOUT_MS) != PORT_OK) {
		Error_API_Handler();
	}
}

/** @brief Imprime la configuración, como pide la práctica 5. */
static void uartPrintConfig(void)
{
	static uint8_t msg[UART_CFG_MSG_LEN];

	int n = snprintf((char *)msg, sizeof(msg),
	                 "\r\n--- UART init OK ---\r\n"
	                 "  Baudrate     : %lu\r\n"
	                 "  Word length  : 8 bits\r\n"
	                 "  Parity       : none\r\n"
	                 "  Stop bits    : 1\r\n"
	                 "  Flow control : none\r\n"
	                 "  Mode         : TX/RX\r\n",
	                 (unsigned long)uartBaud);

	if (n > 0) {
		uartSendString(msg);
	}
}
