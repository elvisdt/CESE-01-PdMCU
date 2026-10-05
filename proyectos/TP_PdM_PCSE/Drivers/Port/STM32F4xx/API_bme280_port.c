/*
 * API_bme280_port.c
 *
 *  Created on: 5 oct 2026
 *      Author: elvisdt
 *
 *  Port del BME280 para NUCLEO-F446RE: bus I2C1 compartido con el LCD,
 *  manejado por API_port.
 */

#include "API_bme280_port.h"
#include "API_port.h"

#include <stddef.h>

#define BME280_I2C_TIMEOUT_MS   10U

bool_t bme280PortInit(void)
{
	return portI2cIsReady(BME280_I2C_ADDR);
}

bool_t bme280PortRead(uint8_t reg, uint8_t *buf, uint16_t len)
{
	if (buf == NULL || len == 0U) {
		return false;
	}
	return (portI2cMemRead(BME280_I2C_ADDR, reg, buf, len, BME280_I2C_TIMEOUT_MS) == PORT_OK);
}

bool_t bme280PortWrite(uint8_t reg, uint8_t value)
{
	return (portI2cMemWrite(BME280_I2C_ADDR, reg, &value, 1U, BME280_I2C_TIMEOUT_MS) == PORT_OK);
}

void bme280PortDelayMs(uint32_t ms)
{
	portDelayMs(ms);
}
