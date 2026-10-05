/*
 * API_bme280.h
 *
 *  Created on: 4 oct 2026
 *      Author: elvisdt
 *
 *  Sensor BME280: temperatura, humedad y presión. Modo normal con
 *  t_standby = 1000 ms y oversampling x1: el sensor mide solo y bme280Read()
 *  lee el último valor sin esperar (no bloquea).
 *  Portable: el acceso al bus lo resuelve API_bme280_port.
 */

#ifndef API_INC_API_BME280_H_
#define API_INC_API_BME280_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

#define BME280_CHIP_ID    0x60U

typedef struct {
	float temperature;   /* °C  */
	float humidity;      /* %HR */
	float pressure;      /* hPa */
} bme280Data_t;

typedef enum {
	BME280_OK = 0,
	BME280_ERR_PORT,     /* el bus no respondió                    */
	BME280_ERR_ID,       /* respondió pero no es un BME280 (0x60)  */
	BME280_ERR_PARAM,    /* puntero NULL                           */
	BME280_ERR_NOT_INIT  /* se llamó a bme280Read() sin bme280Init() */
} bme280Status_t;

/** @brief Verifica el chip ID, lee la calibración y arranca el modo normal. */
bme280Status_t bme280Init(void);

/** @brief Lee la última medición y aplica la compensación del datasheet. */
bme280Status_t bme280Read(bme280Data_t *data);

/** @brief Lee el registro de ID (0xD0). Útil para la primera prueba. */
bme280Status_t bme280ReadChipId(uint8_t *id);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_BME280_H_ */
