/*
 * API_debounce.c
 *
 *  Created on: 17 sept 2026
 *      Author: elvisdt
 *
 *  MEF de antirrebote de la práctica 4, ahora con una instancia por pulsador.
 */

#include "API_debounce.h"
#include "API_delay.h"
#include "API_port.h"

#include <stddef.h>

/* Private defines -----------------------------------------------------------*/
#define DEBOUNCE_TIME_MS   ((tick_t)40)

/* Private types -------------------------------------------------------------*/
typedef enum {
	BUTTON_UP,
	BUTTON_FALLING,
	BUTTON_DOWN,
	BUTTON_RISING,
} debounceState_t;

typedef struct {
	debounceState_t state;
	delay_t         crono;     /* cronómetro no bloqueante del antirrebote   */
	bool_t          keyFlag;   /* pulsación confirmada sin leer              */
} debounce_t;

/* Private variables ---------------------------------------------------------*/
static debounce_t buttons[BTN_COUNT];

/* Private function prototypes -----------------------------------------------*/
static void   debounceUpdateOne(button_t btn);
static bool_t readButtonPin(button_t btn);

/* Public functions ----------------------------------------------------------*/

void debounceFSM_init(void)
{
	for (uint8_t i = 0; i < (uint8_t)BTN_COUNT; i++) {
		buttons[i].state   = BUTTON_UP;
		buttons[i].keyFlag = false;
		delayInit(&buttons[i].crono, DEBOUNCE_TIME_MS);
	}
}

void debounceFSM_update(void)
{
	for (uint8_t i = 0; i < (uint8_t)BTN_COUNT; i++) {
		debounceUpdateOne((button_t)i);
	}
}

bool_t readKey(button_t btn)
{
	if (btn >= BTN_COUNT) {
		return false;
	}
	bool_t wasPressed = buttons[btn].keyFlag;
	buttons[btn].keyFlag = false;
	return wasPressed;
}

/* Private functions ---------------------------------------------------------*/

static void debounceUpdateOne(button_t btn)
{
	debounce_t *b = &buttons[btn];

	switch (b->state) {
	case BUTTON_UP:
		if (readButtonPin(btn)) {
			b->state = BUTTON_FALLING;
		}
		break;

	case BUTTON_FALLING:
		if (delayRead(&b->crono)) {          /* true recién cuando pasaron 40 ms */
			if (readButtonPin(btn)) {
				b->state   = BUTTON_DOWN;
				b->keyFlag = true;           /* buttonPressed() */
			} else {
				b->state = BUTTON_UP;        /* era rebote */
			}
		}
		break;

	case BUTTON_DOWN:
		if (!readButtonPin(btn)) {
			b->state = BUTTON_RISING;
		}
		break;

	case BUTTON_RISING:
		if (delayRead(&b->crono)) {
			if (!readButtonPin(btn)) {
				b->state = BUTTON_UP;        /* buttonReleased(): sin acción */
			} else {
				b->state = BUTTON_DOWN;      /* era rebote */
			}
		}
		break;

	default:
		b->state = BUTTON_UP;
		break;
	}
}

static bool_t readButtonPin(button_t btn)
{
	return portButtonRead((port_button_t)btn);
}
