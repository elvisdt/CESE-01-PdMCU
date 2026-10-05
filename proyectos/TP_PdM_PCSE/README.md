# TP final PdM — Estación de medición ambiental

Estación con BME280 (T, HR, P), LCD 16x2 por I2C, encoder, pulsadores, alarma
(LED + buzzer) y consola UART, sobre **NUCLEO-F446RE**.
Propuesta y diagramas en [`docs/`](docs/).

## Estado de los módulos

| Módulo | Origen | Estado | Prueba |
|---|---|---|---|
| `API_delay` | Práctica 3 | listo | `TEST_DELAY` |
| `API_uart`, `API_cmdparser` | Práctica 5 | listo (comandos de P5) | `TEST_UART` |
| `API_debounce` | Práctica 4 | adaptado a 3 pulsadores | `TEST_DEBOUNCE` |
| `API_gpio` | Práctica 5 | LED + buzzer | `TEST_ALARM` |
| `API_port` (STM32F4xx) | Práctica 5 | + I2C, encoder, buzzer, pulsadores | — |
| `API_encoder` | nuevo | listo | `TEST_ENCODER` |
| `API_lcd` | nuevo | **esqueleto** (`lcdSendNibble`, `lcdInit`) | `TEST_I2C_SCAN`, `TEST_LCD` |
| `API_bme280` | nuevo | **esqueleto** (chip ID listo; calibración y lectura) | `TEST_BME280` |
| `API_alarm` | nuevo | **esqueleto** (`alarmUpdate`) | `TEST_ALARM` |
| `app_ui` | nuevo | **esqueleto** (MEF de interfaz) | `TEST_NONE` |

## Estructura

```
TP_PdM_PCSE/
├── Core/                     CubeMX: main.c, msp, it, startup
├── App/                      aplicación
│   ├── app_ui                MEF de interfaz (UI_INIT, SHOW, SET_MIN, SET_MAX, ERROR)
│   └── app_test              pruebas de a un módulo (APP_TEST)
├── Drivers/
│   ├── API/                  módulos portables (no incluyen la HAL)
│   └── Port/STM32F4xx/       única capa que usa la HAL
└── docs/                     propuesta y diagramas
```

## Primera vez: importar y completar CubeMX

1. CubeIDE → *File › Import › Existing Projects into Workspace* → esta carpeta.
2. Abrir `TP_PdM_PCSE.ioc` y configurar:

| Señal | Pin | Configuración | Label |
|---|---|---|---|
| I2C1 SCL / SDA | PB8 / PB9 | I2C, 100 kHz | — |
| TIM3 CH1 / CH2 | PA6 / PA7 | Combined Channels: Encoder Mode | — |
| Encoder SW | PB5 | GPIO_Input, pull-up | `ENC_SW` |
| Pulsador OK | PA10 | GPIO_Input, pull-up | `BTN_OK` |
| Pulsador BACK | PB3 | GPIO_Input, pull-up | `BTN_BACK` |
| Buzzer | PB10 | GPIO_Output | `BUZZER` |
| LED alarma | PA5 | ya es LD2 | — |

   - **PB3 es SWO**: en *System Core › SYS › Debug* dejar **Serial Wire** (sin trace),
     o usar otro pin para BACK (p. ej. PB4).
   - En *Project Manager › Advanced Settings*, marcar **Do Not Generate Function Call**
     en `MX_USART2_UART_Init`, `MX_I2C1_Init` y `MX_TIM3_Init`: los inicializa `API_port`.
3. *Generate Code*. Copia los drivers HAL/CMSIS que faltan (I2C, TIM).

Hasta que se haga esto el proyecto compila igual: los labels que faltan
caen en B1 (OK) y las funciones de I2C/TIM devuelven error.

## Probar módulo por módulo

Elegir la prueba en `App/Inc/app_test.h`:

```c
#define APP_TEST  TEST_ENCODER
```

o sin tocar código: *Properties › C/C++ Build › Settings › MCU GCC Compiler ›
Preprocessor* → `APP_TEST=4`. Con `TEST_NONE` corre la aplicación completa.

Consola: `picocom -b 115200 /dev/ttyACM0`

| # | Prueba | Qué verificar |
|---|---|---|
| 1 | `TEST_DELAY` | LD2 parpadea a 1 Hz y llega `tick ON/OFF` |
| 2 | `TEST_UART` | `HELP`, `LED ON`, `STATUS` |
| 3 | `TEST_DEBOUNCE` | cada pulsación suma exactamente 1 |
| 4 | `TEST_ENCODER` | 1 clic = ±1; si no, ajustar `ENC_COUNTS_PER_STEP` |
| 5 | `TEST_I2C_SCAN` | aparecen 0x27 y 0x76 |
| 6 | `TEST_LCD` | "Hola PdM" en el LCD |
| 7 | `TEST_BME280` | chip ID 0x60 y valores razonables |
| 8 | `TEST_ALARM` | OK alterna DISABLED/ARMED; con dato simulado pasa a ACTIVE |
