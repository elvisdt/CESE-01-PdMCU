/*
 * API_bme280.c
 *
 *  Created on: 4 oct 2026
 *      Author: elvisdt
 *
 *  Compensación: fórmulas en enteros del datasheet de Bosch (BST-BME280-DS002,
 *  sección 4.2.3 y 8.1/8.2).
 */

#include "API_bme280.h"
#include "API_bme280_port.h"

#include <stddef.h>

/* Registros -----------------------------------------------------------------*/
#define REG_CALIB_TP      0x88U    /* 0x88..0xA1: dig_T1..dig_H1 (26 bytes) */
#define REG_CHIP_ID       0xD0U
#define REG_RESET         0xE0U
#define REG_CALIB_H       0xE1U    /* 0xE1..0xE7: dig_H2..dig_H6 (7 bytes)  */
#define REG_CTRL_HUM      0xF2U
#define REG_STATUS        0xF3U
#define REG_CTRL_MEAS     0xF4U
#define REG_CONFIG        0xF5U
#define REG_DATA          0xF7U    /* 0xF7..0xFE: press, temp, hum (8 bytes) */

#define CALIB_TP_LEN      26U
#define CALIB_H_LEN       7U
#define DATA_LEN          8U

#define RESET_CMD         0xB6U
#define STATUS_IM_UPDATE  0x01U

/* Configuración: oversampling x1 en las tres, modo normal, t_sb 1000 ms */
#define CTRL_HUM_VALUE    0x01U    /* osrs_h = x1                          */
#define CTRL_MEAS_VALUE   0x27U    /* osrs_t = x1, osrs_p = x1, modo normal */
#define CONFIG_VALUE      0xA0U    /* t_sb = 1000 ms, filtro off           */

#define T_RESET_MS        3U
#define RESET_RETRIES     5U

/* Coeficientes de calibración */
typedef struct {
	uint16_t T1; int16_t T2; int16_t T3;
	uint16_t P1; int16_t P2; int16_t P3; int16_t P4; int16_t P5;
	int16_t  P6; int16_t P7; int16_t P8; int16_t P9;
	uint8_t  H1; int16_t H2; uint8_t H3; int16_t H4; int16_t H5; int8_t H6;
} bme280Calib_t;

/* Private variables ---------------------------------------------------------*/
static bme280Calib_t calib;
static int32_t       tFine = 0;      /* compartido entre las compensaciones */
static bool_t        ready = false;

/* Private function prototypes -----------------------------------------------*/
static bool_t   bme280ReadCalibration(void);
static int32_t  bme280CompensateT(int32_t adcT);
static uint32_t bme280CompensateP(int32_t adcP);
static uint32_t bme280CompensateH(int32_t adcH);

/* Public functions ----------------------------------------------------------*/

bme280Status_t bme280ReadChipId(uint8_t *id)
{
	if (id == NULL) {
		return BME280_ERR_PARAM;
	}
	if (!bme280PortRead(REG_CHIP_ID, id, 1U)) {
		return BME280_ERR_PORT;
	}
	return BME280_OK;
}

bme280Status_t bme280Init(void)
{
	uint8_t id = 0U;
	uint8_t status = 0U;

	ready = false;

	if (!bme280PortInit()) {
		return BME280_ERR_PORT;
	}

	bme280Status_t st = bme280ReadChipId(&id);
	if (st != BME280_OK) {
		return st;
	}
	if (id != BME280_CHIP_ID) {
		return BME280_ERR_ID;
	}

	/* reset y esperar a que copie la calibración de la NVM */
	if (!bme280PortWrite(REG_RESET, RESET_CMD)) {
		return BME280_ERR_PORT;
	}
	for (uint8_t i = 0; i < RESET_RETRIES; i++) {
		bme280PortDelayMs(T_RESET_MS);
		if (!bme280PortRead(REG_STATUS, &status, 1U)) {
			return BME280_ERR_PORT;
		}
		if ((status & STATUS_IM_UPDATE) == 0U) {
			break;
		}
	}

	if (!bme280ReadCalibration()) {
		return BME280_ERR_PORT;
	}

	/* ctrl_hum solo se aplica después de escribir ctrl_meas (datasheet 5.4.3) */
	if (!bme280PortWrite(REG_CTRL_HUM, CTRL_HUM_VALUE) ||
	    !bme280PortWrite(REG_CONFIG, CONFIG_VALUE) ||
	    !bme280PortWrite(REG_CTRL_MEAS, CTRL_MEAS_VALUE)) {
		return BME280_ERR_PORT;
	}

	ready = true;
	return BME280_OK;
}

bme280Status_t bme280Read(bme280Data_t *data)
{
	uint8_t raw[DATA_LEN];

	if (data == NULL) {
		return BME280_ERR_PARAM;
	}
	if (!ready) {
		return BME280_ERR_NOT_INIT;
	}
	if (!bme280PortRead(REG_DATA, raw, DATA_LEN)) {
		return BME280_ERR_PORT;
	}

	int32_t adcP = (int32_t)(((uint32_t)raw[0] << 12) | ((uint32_t)raw[1] << 4) | ((uint32_t)raw[2] >> 4));
	int32_t adcT = (int32_t)(((uint32_t)raw[3] << 12) | ((uint32_t)raw[4] << 4) | ((uint32_t)raw[5] >> 4));
	int32_t adcH = (int32_t)(((uint32_t)raw[6] << 8)  |  (uint32_t)raw[7]);

	/* la temperatura va primero: calcula tFine que usan P y H */
	data->temperature = (float)bme280CompensateT(adcT) / 100.0f;        /* 0.01 °C      */
	data->pressure    = (float)bme280CompensateP(adcP) / 25600.0f;      /* Q24.8 Pa -> hPa */
	data->humidity    = (float)bme280CompensateH(adcH) / 1024.0f;       /* Q22.10 %HR   */

	return BME280_OK;
}

/* Private functions ---------------------------------------------------------*/

static bool_t bme280ReadCalibration(void)
{
	uint8_t tp[CALIB_TP_LEN];
	uint8_t h[CALIB_H_LEN];

	if (!bme280PortRead(REG_CALIB_TP, tp, CALIB_TP_LEN) ||
	    !bme280PortRead(REG_CALIB_H, h, CALIB_H_LEN)) {
		return false;
	}

	calib.T1 = (uint16_t)(tp[1] << 8 | tp[0]);
	calib.T2 = (int16_t)(tp[3] << 8 | tp[2]);
	calib.T3 = (int16_t)(tp[5] << 8 | tp[4]);
	calib.P1 = (uint16_t)(tp[7] << 8 | tp[6]);
	calib.P2 = (int16_t)(tp[9] << 8 | tp[8]);
	calib.P3 = (int16_t)(tp[11] << 8 | tp[10]);
	calib.P4 = (int16_t)(tp[13] << 8 | tp[12]);
	calib.P5 = (int16_t)(tp[15] << 8 | tp[14]);
	calib.P6 = (int16_t)(tp[17] << 8 | tp[16]);
	calib.P7 = (int16_t)(tp[19] << 8 | tp[18]);
	calib.P8 = (int16_t)(tp[21] << 8 | tp[20]);
	calib.P9 = (int16_t)(tp[23] << 8 | tp[22]);
	calib.H1 = tp[25];                                        /* 0xA1 */

	calib.H2 = (int16_t)(h[1] << 8 | h[0]);                   /* 0xE1/0xE2 */
	calib.H3 = h[2];                                          /* 0xE3      */
	calib.H4 = (int16_t)((int16_t)(int8_t)h[3] * 16 | (h[4] & 0x0F));   /* 0xE4[11:4] 0xE5[3:0] */
	calib.H5 = (int16_t)((int16_t)(int8_t)h[5] * 16 | (h[4] >> 4));     /* 0xE6[11:4] 0xE5[7:4] */
	calib.H6 = (int8_t)h[6];                                  /* 0xE7      */

	return true;
}

/* Resultado en 0.01 °C ("5123" = 51.23 °C) */
static int32_t bme280CompensateT(int32_t adcT)
{
	int32_t var1 = ((((adcT >> 3) - ((int32_t)calib.T1 << 1))) * (int32_t)calib.T2) >> 11;
	int32_t var2 = (((((adcT >> 4) - (int32_t)calib.T1) * ((adcT >> 4) - (int32_t)calib.T1)) >> 12)
	               * (int32_t)calib.T3) >> 14;
	tFine = var1 + var2;
	return (tFine * 5 + 128) >> 8;
}

/* Resultado en Pa en formato Q24.8 ("24674867" = 24674867/256 = 96386.2 Pa) */
static uint32_t bme280CompensateP(int32_t adcP)
{
	int64_t var1 = (int64_t)tFine - 128000;
	int64_t var2 = var1 * var1 * (int64_t)calib.P6;
	var2 = var2 + ((var1 * (int64_t)calib.P5) << 17);
	var2 = var2 + ((int64_t)calib.P4 << 35);
	var1 = ((var1 * var1 * (int64_t)calib.P3) >> 8) + ((var1 * (int64_t)calib.P2) << 12);
	var1 = ((((int64_t)1) << 47) + var1) * (int64_t)calib.P1 >> 33;
	if (var1 == 0) {
		return 0U;                   /* evita división por cero */
	}
	int64_t p = 1048576 - adcP;
	p = (((p << 31) - var2) * 3125) / var1;
	var1 = ((int64_t)calib.P9 * (p >> 13) * (p >> 13)) >> 25;
	var2 = ((int64_t)calib.P8 * p) >> 19;
	p = ((p + var1 + var2) >> 8) + ((int64_t)calib.P7 << 4);
	return (uint32_t)p;
}

/* Resultado en %HR en formato Q22.10 ("47445" = 47445/1024 = 46.33 %HR) */
static uint32_t bme280CompensateH(int32_t adcH)
{
	int32_t v = tFine - (int32_t)76800;
	v = (((((adcH << 14) - ((int32_t)calib.H4 << 20) - ((int32_t)calib.H5 * v)) + (int32_t)16384) >> 15)
	     * (((((((v * (int32_t)calib.H6) >> 10) * (((v * (int32_t)calib.H3) >> 11) + (int32_t)32768)) >> 10)
	          + (int32_t)2097152) * (int32_t)calib.H2 + 8192) >> 14));
	v = v - (((((v >> 15) * (v >> 15)) >> 7) * (int32_t)calib.H1) >> 4);
	v = (v < 0) ? 0 : v;
	v = (v > 419430400) ? 419430400 : v;
	return (uint32_t)(v >> 12);
}
