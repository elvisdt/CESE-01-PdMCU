/*
 * API_bme280.h
 *
 *  Created on: 4 oct 2026
 *      Author: elvisdt
 *
 *  Sensor BME280 por I2C (0x76): temperatura, humedad y presión.
 */

#ifndef API_INC_API_BME280_H_
#define API_INC_API_BME280_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

#define BME280_I2C_ADDR   0x76U   /* 0x77 si SDO está a VCC */
#define BME280_CHIP_ID    0x60U

typedef struct {
	float temperature;   /* °C  */
	float humidity;      /* %HR */
	float pressure;      /* hPa */
} bme280Data_t;

typedef enum {
	BME280_OK = 0,
	BME280_ERR_I2C,
	BME280_ERR_ID,
	BME280_ERR_PARAM
} bme280Status_t;

bme280Status_t bme280Init(void);                 /* chip ID, calibración, modo normal */
bme280Status_t bme280Read(bme280Data_t *data);   /* lectura compensada                */
bme280Status_t bme280ReadChipId(uint8_t *id);    /* útil para la primera prueba       */

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_BME280_H_ */
