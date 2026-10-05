/*
 * app_ui.c
 *
 *  Created on: 4 oct 2026
 *      Author: elvisdt
 */

#include "app_ui.h"

#include "API_delay.h"
#include "API_debounce.h"
#include "API_encoder.h"
#include "API_lcd.h"
#include "API_bme280.h"
#include "API_alarm.h"
#include "API_cmdparser.h"

/* Private defines -----------------------------------------------------------*/
#define UI_SAMPLE_MS   1000U
#define UI_RETRY_MS    2000U
#define UI_RENDER_MS   200U

/* Private variables ---------------------------------------------------------*/
static uiState_t    state = UI_INIT;
static delay_t      dSample;
static delay_t      dRender;
static bme280Data_t meas;

/* Public functions ----------------------------------------------------------*/

void uiInit(void)
{
	debounceFSM_init();
	alarmInit();
	cmdParserInit();
	delayInit(&dSample, UI_SAMPLE_MS);
	delayInit(&dRender, UI_RENDER_MS);
	state = UI_INIT;
}

void uiUpdate(void)
{
	debounceFSM_update();
	cmdPoll();

	if (delayRead(&dSample)) {
		if (bme280Read(&meas) == BME280_OK) {
			alarmUpdate(&meas);
		}
		/* TODO: si falla la lectura -> UI_ERROR */
	}

	switch (state) {
	case UI_INIT:
		/* TODO: encoderInit(), lcdInit(), bme280Init(); OK -> UI_SHOW, falla -> UI_ERROR */
		state = UI_SHOW;
		break;

	case UI_SHOW:
		/* TODO: encoder cambia variable; OK -> UI_SET_MIN; SW -> alarmToggle() */
		break;

	case UI_SET_MIN:
		/* TODO: encoder ajusta mín; OK -> UI_SET_MAX; BACK -> UI_SHOW (descarta) */
		break;

	case UI_SET_MAX:
		/* TODO: encoder ajusta máx; OK y mín < máx -> guarda, UI_SHOW; BACK -> UI_SET_MIN */
		break;

	case UI_ERROR:
		/* TODO: reintentar bme280Init() cada UI_RETRY_MS; OK -> UI_SHOW */
		break;

	default:
		uiInit();
		break;
	}

	if (delayRead(&dRender)) {
		/* TODO: redibujar el LCD según el estado */
	}
}
