/**
 * @file    API_bme280.h
 * @brief   Sensor BME280: temperatura, humedad y presión.
 * @author  Elvis de la Torre
 * @date    4 oct 2026
 *
 * El sensor trabaja en modo normal: mide solo una vez por segundo y guarda
 * el resultado en sus registros. bme280Read() lee el último valor, así que
 * no espera ninguna conversión y no bloquea.
 *
 * El acceso al bus lo resuelve API_bme280_port.
 */

#ifndef API_INC_API_BME280_H_
#define API_INC_API_BME280_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

#define BME280_CHIP_ID    0x60U   /**< Valor del registro de ID de un BME280. */

/** @brief Una medición ya compensada. */
typedef struct {
	float temperature;   /**< °C */
	float humidity;      /**< %HR */
	float pressure;      /**< hPa */
} bme280Data_t;

/** @brief Resultado de las funciones del sensor. */
typedef enum {
	BME280_OK = 0,        /**< Terminó bien. */
	BME280_ERR_PORT,      /**< El bus no respondió. */
	BME280_ERR_ID,        /**< Respondió algo que no es un BME280. */
	BME280_ERR_PARAM,     /**< Puntero NULL. */
	BME280_ERR_NOT_INIT   /**< Falta bme280Init(), o el sensor se reinició y hay que volver a llamarla. */
} bme280Status_t;

/**
 * @brief Verifica el ID, lee la calibración y pone el sensor en modo normal.
 *
 * Bloquea unos 30 ms entre el reset y la primera medición.
 */
bme280Status_t bme280Init(void);

/**
 * @brief Lee la última medición y la compensa.
 * @param data Destino (no NULL).
 */
bme280Status_t bme280Read(bme280Data_t *data);

/**
 * @brief Lee el registro de ID. Sirve para comprobar la conexión.
 * @param id Destino (no NULL).
 */
bme280Status_t bme280ReadChipId(uint8_t *id);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_BME280_H_ */
