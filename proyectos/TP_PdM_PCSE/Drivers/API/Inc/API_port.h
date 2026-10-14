/**
 * @file    API_port.h
 * @brief   Capa de portabilidad: todo lo que los módulos API piden al hardware.
 * @author  Elvis de la Torre
 * @date    1 oct 2026
 *
 * Ningún módulo API incluye la HAL del fabricante; solo este header. Para
 * llevar el proyecto a otro micro alcanza con reescribir su implementación,
 * que para la NUCLEO-F446RE está en Drivers/Port/STM32F4xx/port_stm32f4xx.c.
 *
 * Las direcciones I2C se pasan en 7 bits (0x27, 0x76); el port las ajusta
 * a lo que pida la HAL.
 */

#ifndef API_INC_API_PORT_H_
#define API_INC_API_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/** @brief Resultado de una operación de entrada/salida. */
typedef enum {
	PORT_OK = 0,     /**< Terminó bien. */
	PORT_TIMEOUT,    /**< No terminó en el tiempo pedido. */
	PORT_BUSY,       /**< El periférico estaba ocupado. */
	PORT_ERROR       /**< Error del periférico, o periférico no habilitado. */
} port_status_t;

/** @brief Pulsadores de la placa. */
typedef enum {
	PORT_BTN_OK = 0,   /**< Confirmar. */
	PORT_BTN_BACK,     /**< Volver. */
	PORT_BTN_ENC_SW,   /**< Eje del encoder. */
	PORT_BTN_COUNT     /**< Cantidad de pulsadores. */
} port_button_t;

/* Sistema -------------------------------------------------------------------*/

/** @brief Milisegundos desde el arranque. Es un contador libre que da la vuelta. */
uint32_t portGetTickMs(void);

/** @brief Retardo bloqueante. Solo para inicializaciones cortas. */
void portDelayMs(uint32_t ms);

/** @brief Detiene el sistema con las interrupciones deshabilitadas. */
void portFatalError(void);

/* GPIO ----------------------------------------------------------------------*/

/** @brief Enciende (true) o apaga el LED de alarma. */
void portLedWrite(bool on);

/** @brief Invierte el LED de alarma. */
void portLedToggle(void);

/** @brief Estado del LED de alarma (true = encendido). */
bool portLedRead(void);

/** @brief Enciende (true) o apaga el buzzer. */
void portBuzzerWrite(bool on);

/**
 * @brief  Lee un pulsador tal cual está, sin antirrebote.
 * @return true mientras está apretado.
 */
bool portButtonRead(port_button_t btn);

/* UART de consola (8N1) -----------------------------------------------------*/

/** @brief Inicializa la UART y su recepción por interrupción. */
bool portUartInit(uint32_t baudrate);

/** @brief Libera la UART (para cambiar el baudrate). */
bool portUartDeInit(void);

/** @brief Transmite y espera a que termine, como máximo timeoutMs. */
port_status_t portUartWrite(const uint8_t *data, uint16_t size, uint32_t timeoutMs);

/**
 * @brief  Toma bytes ya recibidos.
 *
 * La recepción la hace la interrupción, que guarda cada byte en un buffer.
 * Esta función los saca de ahí; con timeoutMs = 0 no espera nada.
 */
port_status_t portUartRead(uint8_t *data, uint16_t size, uint32_t timeoutMs);

/* I2C: bus compartido por el LCD y el BME280 --------------------------------*/

/** @brief Inicializa el bus a 100 kHz. */
bool portI2cInit(void);

/**
 * @brief Libera el bus si un esclavo quedó reteniendo SDA y lo reinicializa.
 *
 * Pasa cuando un módulo se desconecta o se resetea en medio de una
 * transferencia: el esclavo espera más pulsos de reloj y deja SDA en bajo.
 * Se le dan hasta 9 pulsos en SCL y una condición de STOP.
 */
bool portI2cRecover(void);

/** @brief true si el dispositivo responde con ACK en esa dirección. */
bool portI2cIsReady(uint8_t addr7);

/** @brief Escritura directa, sin registro (la usa el PCF8574 del LCD). */
port_status_t portI2cWrite(uint8_t addr7, const uint8_t *data, uint16_t size, uint32_t timeoutMs);

/** @brief Lee size bytes a partir del registro reg (la usa el BME280). */
port_status_t portI2cMemRead(uint8_t addr7, uint8_t reg, uint8_t *data, uint16_t size, uint32_t timeoutMs);

/** @brief Escribe size bytes a partir del registro reg (la usa el BME280). */
port_status_t portI2cMemWrite(uint8_t addr7, uint8_t reg, const uint8_t *data, uint16_t size, uint32_t timeoutMs);

/* Encoder -------------------------------------------------------------------*/

/** @brief Arranca el timer en modo encoder. */
bool portEncoderInit(void);

/** @brief Contador del timer. Es de 16 bits y da la vuelta. */
uint16_t portEncoderGetCount(void);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_PORT_H_ */
