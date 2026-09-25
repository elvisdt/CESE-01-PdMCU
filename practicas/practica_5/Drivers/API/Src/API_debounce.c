/*
 * API_debounce.c
 *
 *  Created on: 17 sept 2026
 *      Author: elvisdt
 */

#include "API_debounce.h"
#include "API_delay.h"
#include "stm32f4xx_hal.h"

/* Private defines - configuración de hardware del módulo --------------------*/
#define BUTTON_GPIO_PORT   GPIOC
#define BUTTON_GPIO_PIN    GPIO_PIN_13
#define DEBOUNCE_TIME_MS   ((tick_t)40)

/* Private types ---------------------------------------------------------*/
typedef enum{
	BUTTON_UP,
	BUTTON_FALLING,
	BUTTON_DOWN,
	BUTTON_RAISING,
} debounceState_t;

/* Private variables -------------------------------------------------------*/
static debounceState_t btn_state;
static delay_t delay_crono;   	// cronometro no bloqueante para el antirrebote
static bool_t key_flag = false; // true cuando hay un flanco descendente confirmado sin leer

/* Private function prototypes ----------------------------------------------*/
static bool_t readButtonPin(void);
static void buttonPressed(void);
static void buttonReleased(void);

/* Public functions ---------------------------------------------------------*/

// debe cargar el estado inicial
void debounceFSM_init(void){
	btn_state = BUTTON_UP;
	delayInit(&delay_crono, DEBOUNCE_TIME_MS);
}

// debe leer las entradas, resolver la lógica de transición y actualizar las salidas
void debounceFSM_update(void){

	switch (btn_state) {
		case BUTTON_UP:
			if(readButtonPin()){
				btn_state = BUTTON_FALLING;
			}
			break;

		case BUTTON_FALLING:
			if(delayRead(&delay_crono)){ // true recién cuando pasaron los 40ms
				if(readButtonPin()){
					btn_state = BUTTON_DOWN;
					buttonPressed();
				}else {
					btn_state = BUTTON_UP; // era rebote
				}
			}
			break;

		case BUTTON_DOWN:
			if(!readButtonPin()){
				btn_state = BUTTON_RAISING;
			}
			break;

		case BUTTON_RAISING:
			if(delayRead(&delay_crono)){
				if(!readButtonPin()){
					btn_state = BUTTON_UP;
					buttonReleased();
				}else {
					btn_state = BUTTON_DOWN; // era rebote
				}
			}
			break;

		default:
			btn_state = BUTTON_UP;
			break;
	}
}

// capturo-reseteo - retorno flag
bool_t readKey(void){
	bool_t wasPressed = key_flag;
	key_flag = false;
	return wasPressed;
}

/* Private functions ----------------------------------------------------------*/

static bool_t readButtonPin(void){
	// B1 es activo en bajo: presionado = pin en RESET
	return (HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, BUTTON_GPIO_PIN) == GPIO_PIN_RESET);
}

static void buttonPressed(void){
	key_flag = true;
}

static void buttonReleased(void){
	/* sin acción requerida en el Punto 2 */
}
