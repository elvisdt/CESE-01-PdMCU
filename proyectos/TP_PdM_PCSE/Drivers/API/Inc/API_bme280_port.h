/*
 * API_bme280_port.h
 *
 *  Created on: 5 oct 2026
 *      Author: elvisdt
 *
 *  Capa port del BME280: acceso a sus registros. API_bme280.c solo usa estas
 *  funciones; para otro micro o para SPI se reescribe API_bme280_port.c.
 */

#ifndef API_INC_API_BME280_PORT_H_
#define API_INC_API_BME280_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

#define BME280_I2C_ADDR   0x76U   /* 0x77 si SDO está a VCC */

/** @brief true si el sensor responde (el bus ya debe estar iniciado). */
bool_t bme280PortInit(void);

/** @brief Lee 'len' bytes a partir del registro 'reg'. */
bool_t bme280PortRead(uint8_t reg, uint8_t *buf, uint16_t len);

/** @brief Escribe un registro. */
bool_t bme280PortWrite(uint8_t reg, uint8_t value);

/** @brief Retardo bloqueante, solo para la inicialización. */
void   bme280PortDelayMs(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_BME280_PORT_H_ */
