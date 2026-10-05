/*
 * port_stm32f4xx.c
 *
 *  Created on: 1 oct 2026
 *      Author: elvisdt
 *
 *  Implementación de API_port.h para STM32F4xx (NUCLEO-F446RE) sobre la HAL.
 *  Es el ÚNICO archivo fuera de Core/ que conoce la HAL y los pines de la placa.
 *
 *  Depende de CubeMX en lo que genera en Core/:
 *    - HAL_Init() + SystemClock_Config()     -> base de tiempo (HAL_GetTick)
 *    - MX_GPIO_Init()                        -> pines con sus labels (ver README)
 *    - HAL_xxx_MspInit() en *_hal_msp.c      -> relojes y pines AF de USART2,
 *                                               I2C1 y TIM3
 *  USART2, I2C1 y TIM3 se configuran en CubeMX con "Do Not Generate Function
 *  Call": el init lo hace este archivo con sus propios handles.
 *
 *  Mientras un periférico no esté habilitado en CubeMX, sus funciones
 *  compilan igual y devuelven error (ver #ifdef HAL_xxx_MODULE_ENABLED).
 */

#include "API_port.h"
#include "main.h"          /* HAL + labels de pines generados por CubeMX */

#include <stddef.h>

/* Private defines -----------------------------------------------------------*/
#define PORT_UART_INSTANCE   USART2
#define PORT_I2C_INSTANCE    I2C1
#define PORT_I2C_SPEED_HZ    100000U
#define PORT_I2C_TRIALS      2U
#define PORT_I2C_READY_MS    5U
#define PORT_ENC_INSTANCE    TIM3
#define PORT_ENC_FILTER      0x0FU

/* Pines de la aplicación: si todavía no existen los labels en CubeMX,
 * el pulsador OK cae en B1 (pulsador azul) para poder probar desde ya. */
#if !defined(BTN_OK_Pin)
#define BTN_OK_Pin        B1_Pin
#define BTN_OK_GPIO_Port  B1_GPIO_Port
#endif

/* Private variables ---------------------------------------------------------*/
static UART_HandleTypeDef portUart;
#ifdef HAL_I2C_MODULE_ENABLED
static I2C_HandleTypeDef  portI2c;
#endif
#ifdef HAL_TIM_MODULE_ENABLED
static TIM_HandleTypeDef  portEnc;
#endif

/* Private function prototypes -----------------------------------------------*/
static port_status_t portFromHal(HAL_StatusTypeDef st);

/* Sistema -------------------------------------------------------------------*/

uint32_t portGetTickMs(void)
{
	return HAL_GetTick();
}

void portDelayMs(uint32_t ms)
{
	HAL_Delay(ms);
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

void portBuzzerWrite(bool on)
{
#if defined(BUZZER_Pin)
	HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
#else
	(void)on;          /* pin aún no configurado en CubeMX */
#endif
}

bool portButtonRead(port_button_t btn)
{
	/* todos los pulsadores son activos en bajo (pull-up) */
	switch (btn) {
	case PORT_BTN_OK:
		return (HAL_GPIO_ReadPin(BTN_OK_GPIO_Port, BTN_OK_Pin) == GPIO_PIN_RESET);
#if defined(BTN_BACK_Pin)
	case PORT_BTN_BACK:
		return (HAL_GPIO_ReadPin(BTN_BACK_GPIO_Port, BTN_BACK_Pin) == GPIO_PIN_RESET);
#endif
#if defined(ENC_SW_Pin)
	case PORT_BTN_ENC_SW:
		return (HAL_GPIO_ReadPin(ENC_SW_GPIO_Port, ENC_SW_Pin) == GPIO_PIN_RESET);
#endif
	default:
		return false;
	}
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

/* I2C -----------------------------------------------------------------------*/

#ifdef HAL_I2C_MODULE_ENABLED

bool portI2cInit(void)
{
	portI2c.Instance             = PORT_I2C_INSTANCE;
	portI2c.Init.ClockSpeed      = PORT_I2C_SPEED_HZ;
	portI2c.Init.DutyCycle       = I2C_DUTYCYCLE_2;
	portI2c.Init.OwnAddress1     = 0;
	portI2c.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
	portI2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	portI2c.Init.OwnAddress2     = 0;
	portI2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	portI2c.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

	return (HAL_I2C_Init(&portI2c) == HAL_OK);
}

bool portI2cIsReady(uint8_t addr7)
{
	return (HAL_I2C_IsDeviceReady(&portI2c, (uint16_t)(addr7 << 1),
			PORT_I2C_TRIALS, PORT_I2C_READY_MS) == HAL_OK);
}

port_status_t portI2cWrite(uint8_t addr7, const uint8_t *data, uint16_t size, uint32_t timeoutMs)
{
	if (data == NULL || size == 0U) {
		return PORT_ERROR;
	}
	return portFromHal(HAL_I2C_Master_Transmit(&portI2c, (uint16_t)(addr7 << 1),
			(uint8_t *)data, size, timeoutMs));
}

port_status_t portI2cMemRead(uint8_t addr7, uint8_t reg, uint8_t *data, uint16_t size, uint32_t timeoutMs)
{
	if (data == NULL || size == 0U) {
		return PORT_ERROR;
	}
	return portFromHal(HAL_I2C_Mem_Read(&portI2c, (uint16_t)(addr7 << 1), reg,
			I2C_MEMADD_SIZE_8BIT, data, size, timeoutMs));
}

port_status_t portI2cMemWrite(uint8_t addr7, uint8_t reg, const uint8_t *data, uint16_t size, uint32_t timeoutMs)
{
	if (data == NULL || size == 0U) {
		return PORT_ERROR;
	}
	return portFromHal(HAL_I2C_Mem_Write(&portI2c, (uint16_t)(addr7 << 1), reg,
			I2C_MEMADD_SIZE_8BIT, (uint8_t *)data, size, timeoutMs));
}

#else /* I2C1 todavía no habilitado en CubeMX */

bool portI2cInit(void) { return false; }
bool portI2cIsReady(uint8_t addr7) { (void)addr7; return false; }
port_status_t portI2cWrite(uint8_t addr7, const uint8_t *data, uint16_t size, uint32_t timeoutMs)
{ (void)addr7; (void)data; (void)size; (void)timeoutMs; return PORT_ERROR; }
port_status_t portI2cMemRead(uint8_t addr7, uint8_t reg, uint8_t *data, uint16_t size, uint32_t timeoutMs)
{ (void)addr7; (void)reg; (void)data; (void)size; (void)timeoutMs; return PORT_ERROR; }
port_status_t portI2cMemWrite(uint8_t addr7, uint8_t reg, const uint8_t *data, uint16_t size, uint32_t timeoutMs)
{ (void)addr7; (void)reg; (void)data; (void)size; (void)timeoutMs; return PORT_ERROR; }

#endif /* HAL_I2C_MODULE_ENABLED */

/* Encoder -------------------------------------------------------------------*/

#ifdef HAL_TIM_MODULE_ENABLED

bool portEncoderInit(void)
{
	TIM_Encoder_InitTypeDef cfg = {0};

	portEnc.Instance               = PORT_ENC_INSTANCE;
	portEnc.Init.Prescaler         = 0;
	portEnc.Init.CounterMode       = TIM_COUNTERMODE_UP;
	portEnc.Init.Period            = 0xFFFFU;
	portEnc.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
	portEnc.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	cfg.EncoderMode = TIM_ENCODERMODE_TI12;
	cfg.IC1Polarity = TIM_ICPOLARITY_RISING;
	cfg.IC1Selection = TIM_ICSELECTION_DIRECTTI;
	cfg.IC1Prescaler = TIM_ICPSC_DIV1;
	cfg.IC1Filter    = PORT_ENC_FILTER;
	cfg.IC2Polarity = TIM_ICPOLARITY_RISING;
	cfg.IC2Selection = TIM_ICSELECTION_DIRECTTI;
	cfg.IC2Prescaler = TIM_ICPSC_DIV1;
	cfg.IC2Filter    = PORT_ENC_FILTER;

	if (HAL_TIM_Encoder_Init(&portEnc, &cfg) != HAL_OK) {
		return false;
	}
	return (HAL_TIM_Encoder_Start(&portEnc, TIM_CHANNEL_ALL) == HAL_OK);
}

uint16_t portEncoderGetCount(void)
{
	return (uint16_t)__HAL_TIM_GET_COUNTER(&portEnc);
}

#else /* TIM3 todavía no habilitado en CubeMX */

bool portEncoderInit(void) { return false; }
uint16_t portEncoderGetCount(void) { return 0U; }

#endif /* HAL_TIM_MODULE_ENABLED */

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
