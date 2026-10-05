/*
 * API_bme280.c
 *
 *  Created on: 4 oct 2026
 *      Author: elvisdt
 */

#include "API_bme280.h"
#include "API_port.h"

#include <stddef.h>

/* Private defines -----------------------------------------------------------*/
#define BME280_REG_ID       0xD0U
#define BME280_I2C_TIMEOUT  10U

/* Public functions ----------------------------------------------------------*/

bme280Status_t bme280ReadChipId(uint8_t *id)
{
	if (id == NULL) {
		return BME280_ERR_PARAM;
	}
	if (portI2cMemRead(BME280_I2C_ADDR, BME280_REG_ID, id, 1U, BME280_I2C_TIMEOUT) != PORT_OK) {
		return BME280_ERR_I2C;
	}
	return BME280_OK;
}

bme280Status_t bme280Init(void)
{
	uint8_t id = 0U;
	bme280Status_t st = bme280ReadChipId(&id);

	if (st != BME280_OK) {
		return st;
	}
	if (id != BME280_CHIP_ID) {
		return BME280_ERR_ID;
	}
	/* TODO: leer calibración (0x88..0xA1 y 0xE1..0xE7),
	 *       ctrl_hum (0xF2), config (0xF5, t_standby 1000 ms), ctrl_meas (0xF4, modo normal) */
	return BME280_OK;
}

bme280Status_t bme280Read(bme280Data_t *data)
{
	if (data == NULL) {
		return BME280_ERR_PARAM;
	}
	/* TODO: leer 0xF7..0xFE (8 bytes) y aplicar la compensación del datasheet */
	return BME280_ERR_I2C;
}
