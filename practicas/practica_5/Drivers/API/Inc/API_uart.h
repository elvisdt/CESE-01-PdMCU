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

#include "API_common.h"

#define UART_MIN_SIZE   1U
#define UART_MAX_SIZE   256U
#define UART_BAUD_MIN   9600U
#define UART_BAUD_MAX   921600U
#define UART_BAUD_INIT  115200U

/**
 * @brief  Inicializa la UART de consola (UART_BAUD_INIT, 8N1) e imprime su configuración.
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

/**
 * @brief  Intenta recibir un byte (polling con timeout corto).
 * @param  pbyte: destino del byte (no NULL).
 * @retval true si llegó un byte, false si venció el timeout o hubo error.
 */
bool_t uartReceiveByte(uint8_t * pbyte);

/**
 * @brief  Devuelve el baudrate actual de la UART.
 */
uint32_t uartGetBaudrate(void);

/**
 * @brief  Cambia el baudrate y reinicia la UART.
 * @param  baudrate: nuevo valor (UART_BAUD_MIN..UART_BAUD_MAX).
 * @retval true si se aplicó, false si está fuera de rango o falló la HAL
 *         (en ese caso se mantiene el baudrate anterior).
 */
bool_t uartSetBaudrate(uint32_t baudrate);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_UART_H_ */
