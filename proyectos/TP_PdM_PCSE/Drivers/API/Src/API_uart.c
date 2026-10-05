/*
 * API_uart.c
 *
 *  Created on: 24 sept 2026
 *      Author: elvisdt
 *
 *  UART de consola en modo polling. Portable: el acceso al periférico
 *  lo resuelve API_port (formato fijo 8N1, sin control de flujo).
 */

#include "API_uart.h"
#include "API_port.h"

#include <stdio.h>
#include <stddef.h>

/* Private defines -----------------------------------------------------------*/
#define UART_TX_TIMEOUT_MS   1000U
#define UART_RX_TIMEOUT_MS   10U
#define UART_CFG_MSG_LEN     256U

/* Private variables ---------------------------------------------------------*/
static uint32_t uartBaud  = 0U;
static bool_t   uartReady = false;

/* Private function prototypes -----------------------------------------------*/
static bool_t   uartIsValidSize(uint32_t size);
static uint32_t uartStrLen(const uint8_t * pstring, uint32_t maxLen);
static void     uartTransmit(const uint8_t * pdata, uint16_t size);
static void     uartPrintConfig(void);

/* Public functions ----------------------------------------------------------*/

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

	port_status_t st = portUartRead(pstring, size, UART_RX_TIMEOUT_MS);

	/* TIMEOUT: no llegaron datos.
	 * ERROR: overrun (se perdió un byte); el driver ya limpió el flag,
	 * no se bloquea el sistema por eso. Solo BUSY es un error real. */
	if (st == PORT_BUSY) {
		Error_API_Handler();
	}
}

bool_t uartReceiveByte(uint8_t * pbyte)
{
	if (pbyte == NULL || !uartReady) {
		return false;
	}

	port_status_t st = portUartRead(pbyte, 1U, UART_RX_TIMEOUT_MS);

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
		/* no se pudo: se vuelve al baudrate anterior */
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

/* Private functions ---------------------------------------------------------*/

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

/* Transmite y verifica el resultado del port */
static void uartTransmit(const uint8_t * pdata, uint16_t size)
{
	if (!uartReady) {
		return;
	}

	if (portUartWrite(pdata, size, UART_TX_TIMEOUT_MS) != PORT_OK) {
		Error_API_Handler();
	}
}

/* Imprime la configuración de la UART (formato fijo 8N1 del port) */
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
