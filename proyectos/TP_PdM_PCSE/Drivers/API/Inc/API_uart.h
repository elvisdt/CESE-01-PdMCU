/**
 * @file    API_uart.h
 * @brief   UART de consola (práctica 5).
 * @author  Elvis de la Torre
 * @date    24 sept 2026
 *
 * Formato fijo 8N1 sin control de flujo. La transmisión espera a que salga
 * el último byte; la recepción no espera: el port junta lo que llega por
 * interrupción y uartReceiveByte() devuelve lo que ya está.
 */

#ifndef API_INC_API_UART_H_
#define API_INC_API_UART_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

#define UART_MIN_SIZE   1U        /**< Menor envío o recepción permitido. */
#define UART_MAX_SIZE   256U      /**< Mayor envío o recepción permitido. */
#define UART_BAUD_MIN   9600U     /**< Menor baudrate aceptado. */
#define UART_BAUD_MAX   921600U   /**< Mayor baudrate aceptado. */
#define UART_BAUD_INIT  115200U   /**< Baudrate al arrancar. */

/**
 * @brief  Inicializa la UART a UART_BAUD_INIT e imprime su configuración.
 * @return true si se pudo inicializar.
 */
bool_t uartInit(void);

/**
 * @brief Envía un string hasta el '\0'.
 * @param pstring String de 1 a UART_MAX_SIZE caracteres (no NULL).
 */
void uartSendString(uint8_t *pstring);

/**
 * @brief Envía una cantidad fija de bytes.
 * @param pstring Datos (no NULL).
 * @param size    Cantidad, de UART_MIN_SIZE a UART_MAX_SIZE.
 */
void uartSendStringSize(uint8_t *pstring, uint16_t size);

/**
 * @brief Espera hasta 10 ms a que lleguen size bytes.
 * @param pstring Destino (no NULL).
 * @param size    Cantidad, de UART_MIN_SIZE a UART_MAX_SIZE.
 */
void uartReceiveStringSize(uint8_t *pstring, uint16_t size);

/**
 * @brief  Toma un byte recibido, si hay alguno. No espera.
 * @param  pbyte Destino (no NULL).
 * @return true si había un byte.
 */
bool_t uartReceiveByte(uint8_t *pbyte);

/** @brief Baudrate actual. */
uint32_t uartGetBaudrate(void);

/**
 * @brief  Cambia el baudrate y reinicia la UART.
 * @param  baudrate Entre UART_BAUD_MIN y UART_BAUD_MAX.
 * @return true si se aplicó. Si falla, queda el baudrate anterior.
 */
bool_t uartSetBaudrate(uint32_t baudrate);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_UART_H_ */
