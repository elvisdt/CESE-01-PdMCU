/*
 * API_uart.h
 *
 *  Created on: 24 sept 2026
 *      Author: elvisdt
 */

#ifndef API_INC_API_UART_H_
#define API_INC_API_UART_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "API_common.h"

#define UART_MIN_SIZE   1U
#define UART_MAX_SIZE   256U

/**
 * @brief  Inicializa USART2 (115200 8N1) e imprime su configuración.
 * @retval true si la inicialización fue exitosa, false si no.
 */
bool_t uartInit(void);

/**
 * @brief  Envía un string completo hasta '\0'.
 * @param  pstring: string a enviar (no NULL, 1..UART_MAX_SIZE caracteres).
 */
void uartSendString(uint8_t * pstring);

/**
 * @brief  Envía 'size' bytes del buffer.
 * @param  pstring: buffer a enviar (no NULL).
 * @param  size: cantidad de bytes (1..UART_MAX_SIZE).
 */
void uartSendStringSize(uint8_t * pstring, uint16_t size);

/**
 * @brief  Recibe 'size' bytes en modo polling (retorna si vence el timeout).
 * @param  pstring: buffer destino (no NULL).
 * @param  size: cantidad de bytes (1..UART_MAX_SIZE).
 */
void uartReceiveStringSize(uint8_t * pstring, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_UART_H_ */
