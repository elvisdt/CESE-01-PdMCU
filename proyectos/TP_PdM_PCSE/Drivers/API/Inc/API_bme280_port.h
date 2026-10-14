/**
 * @file    API_bme280_port.h
 * @brief   Port del BME280: acceso a sus registros.
 * @author  Elvis de la Torre
 * @date    5 oct 2026
 *
 * API_bme280.c usa solo estas funciones. Para otro micro, o para usar el
 * sensor por SPI, se reescribe únicamente API_bme280_port.c.
 */

#ifndef API_INC_API_BME280_PORT_H_
#define API_INC_API_BME280_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

#define BME280_I2C_ADDR   0x76U   /**< Con SDO a GND. Es 0x77 si SDO va a VCC. */

/**
 * @brief  Verifica que el sensor responda. El bus ya tiene que estar iniciado.
 * @return true si respondió.
 */
bool_t bme280PortInit(void);

/**
 * @brief  Lee varios registros seguidos.
 * @param  reg Primer registro.
 * @param  buf Destino (no NULL).
 * @param  len Cantidad (mayor que 0).
 * @return true si la lectura terminó bien.
 */
bool_t bme280PortRead(uint8_t reg, uint8_t *buf, uint16_t len);

/**
 * @brief  Escribe un registro.
 * @return true si la escritura terminó bien.
 */
bool_t bme280PortWrite(uint8_t reg, uint8_t value);

/** @brief Retardo bloqueante, solo para la inicialización. */
void bme280PortDelayMs(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_BME280_PORT_H_ */
