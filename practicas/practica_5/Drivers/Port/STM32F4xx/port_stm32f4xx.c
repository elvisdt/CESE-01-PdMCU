/*
 * port_stm32f4xx.c
 *
 *  Created on: 1 oct 2026
 *      Author: elvisdt
 *
 *  Implementación de API_port.h para STM32F4xx (NUCLEO-F446RE) sobre la HAL.
 *  Es el ÚNICO archivo fuera de Core/ que conoce la HAL y los pines de la placa.
 *
 *  Depende de CubeMX solo en lo que sigue generando en Core/:
 *    - SystemClock_Config() y HAL_Init()      -> base de tiempo (HAL_GetTick)
 *    - MX_GPIO_Init()                         -> LD2 salida, B1 entrada
 *    - HAL_UART_MspInit() en *_hal_msp.c      -> reloj USART2 y PA2/PA3 en AF7
 */

#include "API_port.h"
#include "main.h"          /* HAL + LD2_Pin/LD2_GPIO_Port, B1_Pin/B1_GPIO_Port */

#include <stddef.h>

/* Private defines -----------------------------------------------------------*/
#define PORT_UART_INSTANCE   USART2

/* Private variables ---------------------------------------------------------*/
static UART_HandleTypeDef portUart;

/* Private function prototypes -----------------------------------------------*/
static port_status_t portFromHal(HAL_StatusTypeDef st);

/* Sistema -------------------------------------------------------------------*/

uint32_t portGetTickMs(void)
{
	return HAL_GetTick();
}

void portFatalError(void)
{
	__disable_irq();
	while (1) {
	}
}

/* GPIO ----------------------------------------------------------------------*/

void portLedWrite(bool on)
{
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void portLedToggle(void)
{
	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}

bool portLedRead(void)
{
	return (HAL_GPIO_ReadPin(LD2_GPIO_Port, LD2_Pin) == GPIO_PIN_SET);
}

bool portButtonRead(void)
{
	/* B1 es activo en bajo: presionado = pin en RESET */
	return (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET);
}

/* UART ----------------------------------------------------------------------*/

bool portUartInit(uint32_t baudrate)
{
	portUart.Instance          = PORT_UART_INSTANCE;
	portUart.Init.BaudRate     = baudrate;
	portUart.Init.WordLength   = UART_WORDLENGTH_8B;
	portUart.Init.StopBits     = UART_STOPBITS_1;
	portUart.Init.Parity       = UART_PARITY_NONE;
	portUart.Init.Mode         = UART_MODE_TX_RX;
	portUart.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
	portUart.Init.OverSampling = UART_OVERSAMPLING_16;

	return (HAL_UART_Init(&portUart) == HAL_OK);
}

bool portUartDeInit(void)
{
	return (HAL_UART_DeInit(&portUart) == HAL_OK);
}

port_status_t portUartWrite(const uint8_t *data, uint16_t size, uint32_t timeoutMs)
{
	if (data == NULL) {
		return PORT_ERROR;
	}
	/* la HAL de F4 no usa const en el buffer de TX */
	return portFromHal(HAL_UART_Transmit(&portUart, (uint8_t *)data, size, timeoutMs));
}

port_status_t portUartRead(uint8_t *data, uint16_t size, uint32_t timeoutMs)
{
	if (data == NULL) {
		return PORT_ERROR;
	}
	return portFromHal(HAL_UART_Receive(&portUart, data, size, timeoutMs));
}

/* Private functions ---------------------------------------------------------*/

static port_status_t portFromHal(HAL_StatusTypeDef st)
{
	switch (st) {
	case HAL_OK:      return PORT_OK;
	case HAL_TIMEOUT: return PORT_TIMEOUT;
	case HAL_BUSY:    return PORT_BUSY;
	default:          return PORT_ERROR;
	}
}
