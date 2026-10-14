/**
 * @file    API_debounce.c
 * @brief   Antirrebote de los pulsadores (práctica 4).
 * @author  Elvis de la Torre
 * @date    17 sept 2026
 *
 * MEF por pulsador:
 *
 * - BUTTON_UP: suelto. Si el pin se lee apretado pasa a BUTTON_FALLING.
 * - BUTTON_FALLING: espera 40 ms. Si sigue apretado, BUTTON_DOWN y queda
 *   marcada la pulsación; si no, era un rebote y vuelve a BUTTON_UP.
 * - BUTTON_DOWN: apretado. Si se lee suelto pasa a BUTTON_RAISING.
 * - BUTTON_RAISING: espera 40 ms. Si sigue suelto, BUTTON_UP; si no, vuelve
 *   a BUTTON_DOWN.
 */

#include "API_debounce.h"
#include "API_delay.h"
#include "API_port.h"

/* Configuración privada -----------------------------------------------------*/

#define DEBOUNCE_TIME_MS   40U

/* Tipos privados ------------------------------------------------------------*/

typedef enum {
	BUTTON_UP,
	BUTTON_FALLING,
	BUTTON_DOWN,
	BUTTON_RAISING
} debounceState_t;

typedef struct {
	debounceState_t state;
	delay_t         timer;     /**< espera de los 40 ms */
	bool_t          pressed;   /**< pulsación confirmada sin leer */
} debounce_t;

/* Variables privadas --------------------------------------------------------*/

static debounce_t buttons[BTN_COUNT];

/* Funciones privadas --------------------------------------------------------*/

static void   debounceUpdateOne(button_t btn);
static bool_t isPressed(button_t btn);

/* Funciones públicas --------------------------------------------------------*/

void debounceFSM_init(void)
{
	for (uint8_t i = 0U; i < (uint8_t)BTN_COUNT; i++) {
		buttons[i].state   = BUTTON_UP;
		buttons[i].pressed = false;
		delayInit(&buttons[i].timer, DEBOUNCE_TIME_MS);
	}
}

void debounceFSM_update(void)
{
	for (uint8_t i = 0U; i < (uint8_t)BTN_COUNT; i++) {
		debounceUpdateOne((button_t)i);
	}
}

bool_t readKey(button_t btn)
{
	if (btn >= BTN_COUNT) {
		return false;
	}
	bool_t wasPressed = buttons[btn].pressed;
	buttons[btn].pressed = false;
	return wasPressed;
}

/* Funciones privadas --------------------------------------------------------*/

/** @brief Avanza la MEF de un pulsador. */
static void debounceUpdateOne(button_t btn)
{
	debounce_t *b = &buttons[btn];

	switch (b->state) {
	case BUTTON_UP:
		if (isPressed(btn)) {
			b->state = BUTTON_FALLING;
		}
		break;

	case BUTTON_FALLING:
		if (delayRead(&b->timer)) {
			if (isPressed(btn)) {
				b->state   = BUTTON_DOWN;
				b->pressed = true;
			} else {
				b->state = BUTTON_UP;
			}
		}
		break;

	case BUTTON_DOWN:
		if (!isPressed(btn)) {
			b->state = BUTTON_RAISING;
		}
		break;

	case BUTTON_RAISING:
		if (delayRead(&b->timer)) {
			b->state = isPressed(btn) ? BUTTON_DOWN : BUTTON_UP;
		}
		break;

	default:
		/* Un estado inválido solo puede venir de memoria corrupta: se
		 * arranca de nuevo desde un lugar seguro. */
		b->state = BUTTON_UP;
		break;
	}
}

/** @brief Nivel actual del pin, sin filtrar. */
static bool_t isPressed(button_t btn)
{
	return portButtonRead((port_button_t)btn);
}
