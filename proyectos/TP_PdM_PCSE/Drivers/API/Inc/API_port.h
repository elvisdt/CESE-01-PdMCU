/*
 * API_port.h
 *
 *  Created on: 1 oct 2026
 *      Author: elvisdt
 *
 *  Capa de portabilidad (port): única interfaz entre los módulos API_* y el
 *  hardware. Los API_* solo incluyen este header, nunca la HAL del fabricante.
 *
 *  Implementación para NUCLEO-F446RE: Drivers/Port/STM32F4xx/port_stm32f4xx.c
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
	PORT_TIMEOUT,    /* no se completó en el tiempo dado                  */
	PORT_BUSY,       /* periférico ocupado                                */
	PORT_ERROR       /* error de hardware/driver o periférico no habilitado */
} port_status_t;

/* Pulsadores de la aplicación */
typedef enum {
	PORT_BTN_OK = 0,
	PORT_BTN_BACK,
	PORT_BTN_ENC_SW,
	PORT_BTN_COUNT
} port_button_t;

/* Sistema -------------------------------------------------------------------*/

/** @brief Milisegundos desde el arranque (contador libre, puede desbordar). */
uint32_t portGetTickMs(void);

/** @brief Retardo bloqueante. Usar SOLO en inicializaciones (p. ej. LCD). */
void portDelayMs(uint32_t ms);

/** @brief Error irrecuperable: deshabilita interrupciones y detiene el sistema. */
void portFatalError(void);

/* GPIO ----------------------------------------------------------------------*/

/** @brief Escribe el LED de alarma / LD2 (true = encendido). */
void portLedWrite(bool on);

/** @brief Invierte el LED. */
void portLedToggle(void);

/** @brief Lee el LED (true = encendido). */
bool portLedRead(void);

/** @brief Escribe el buzzer (true = encendido). */
void portBuzzerWrite(bool on);

/** @brief Lee un pulsador sin antirrebote (true = presionado). */
bool portButtonRead(port_button_t btn);

/* UART (consola, 8N1) -------------------------------------------------------*/

bool          portUartInit(uint32_t baudrate);
bool          portUartDeInit(void);
port_status_t portUartWrite(const uint8_t *data, uint16_t size, uint32_t timeoutMs);
port_status_t portUartRead(uint8_t *data, uint16_t size, uint32_t timeoutMs);

/* I2C (bus compartido BME280 + LCD) -----------------------------------------*/
/* Direcciones en 7 bits (0x76, 0x27). El port las desplaza para la HAL.     */

/** @brief Inicializa el bus I2C (100 kHz). */
bool          portI2cInit(void);

/** @brief true si el dispositivo responde con ACK en esa dirección. */
bool          portI2cIsReady(uint8_t addr7);

/** @brief Escritura directa (sin registro), usada por el PCF8574 del LCD. */
port_status_t portI2cWrite(uint8_t addr7, const uint8_t *data, uint16_t size, uint32_t timeoutMs);

/** @brief Lee 'size' bytes a partir del registro 'reg' (BME280). */
port_status_t portI2cMemRead(uint8_t addr7, uint8_t reg, uint8_t *data, uint16_t size, uint32_t timeoutMs);

/** @brief Escribe 'size' bytes a partir del registro 'reg' (BME280). */
port_status_t portI2cMemWrite(uint8_t addr7, uint8_t reg, const uint8_t *data, uint16_t size, uint32_t timeoutMs);

/* Encoder (timer en modo encoder) -------------------------------------------*/

/** @brief Inicializa y arranca el timer en modo encoder. */
bool     portEncoderInit(void);

/** @brief Valor actual del contador del timer (16 bits, da la vuelta). */
uint16_t portEncoderGetCount(void);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_PORT_H_ */
