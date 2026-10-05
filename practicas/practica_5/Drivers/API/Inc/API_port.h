/*
 * API_port.h
 *
 *  Created on: 1 oct 2026
 *      Author: elvisdt
 *
 *  Capa de portabilidad (port): única interfaz entre los módulos API_* y el
 *  hardware. Los API_* solo incluyen este header, nunca la HAL del fabricante.
 *
 *  Para llevar el código a otro micro/placa se escribe una nueva implementación
 *  de estas funciones (p. ej. Drivers/Port/<familia>/port_<familia>.c) y se
 *  compila esa en lugar de la de STM32F4xx. El resto de API_* no cambia.
 */

#ifndef API_INC_API_PORT_H_
#define API_INC_API_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* Resultado de las operaciones de I/O del port */
typedef enum {
	PORT_OK = 0,
	PORT_TIMEOUT,    /* no se completó en el tiempo dado (p. ej. no llegaron datos) */
	PORT_BUSY,       /* periférico ocupado                                          */
	PORT_ERROR       /* error de hardware/driver                                    */
} port_status_t;

/* Sistema -------------------------------------------------------------------*/

/**
 * @brief  Milisegundos desde el arranque (contador libre, puede desbordar).
 */
uint32_t portGetTickMs(void);

/**
 * @brief  Error irrecuperable: deshabilita interrupciones y detiene el sistema.
 */
void portFatalError(void);

/* GPIO ----------------------------------------------------------------------*/

/** @brief Escribe el LED de usuario (true = encendido). */
void portLedWrite(bool on);

/** @brief Invierte el LED de usuario. */
void portLedToggle(void);

/** @brief Lee el LED de usuario (true = encendido). */
bool portLedRead(void);

/** @brief Lee el pulsador de usuario, sin antirrebote (true = presionado). */
bool portButtonRead(void);

/* UART (consola) ------------------------------------------------------------*/
/* Formato fijo: 8 bits de datos, sin paridad, 1 stop, sin control de flujo.  */

/**
 * @brief  Inicializa la UART de consola con el baudrate dado (8N1).
 * @retval true si se inicializó correctamente.
 */
bool portUartInit(uint32_t baudrate);

/**
 * @brief  Libera la UART (para reconfigurarla con portUartInit).
 * @retval true si se liberó correctamente.
 */
bool portUartDeInit(void);

/**
 * @brief  Transmite 'size' bytes en modo bloqueante.
 */
port_status_t portUartWrite(const uint8_t *data, uint16_t size, uint32_t timeoutMs);

/**
 * @brief  Recibe 'size' bytes en modo bloqueante (con timeout).
 */
port_status_t portUartRead(uint8_t *data, uint16_t size, uint32_t timeoutMs);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_PORT_H_ */
