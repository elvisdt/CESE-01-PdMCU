# Práctica 5 - UART y parser de comandos (MEF)

## Datos del Autor
* **Alumno:** Elvis de la Torre
* **Carrera:** (CESE) - FIUBA
* **Materia:** Programación de Microprocesadores

## Enunciado
[Práctica 5.pdf](../../docs/clase-05/Práctica%205.pdf)

## Descripción

### Arquitectura

```
Core/ (CubeMX)          clock, MX_GPIO_Init, HAL_UART_MspInit, main()
   │
Drivers/API/            lógica portable: NO incluye la HAL
   API_cmdparser ──> API_uart ──┐
   API_debounce  ──> API_gpio ──┼──> API_port.h  (interfaz de hardware)
                     API_delay ─┘          │
Drivers/Port/STM32F4xx/                    ▼
   port_stm32f4xx.c     implementación de API_port.h con la HAL de ST
```

Para portar a otro micro/placa: escribir `Drivers/Port/<familia>/port_<familia>.c`
implementando `API_port.h`. Los módulos `API_*` no cambian.

### Módulos

- `API_port` (`Inc/API_port.h`): interfaz de hardware (tick, error fatal, LED, pulsador, UART 8N1).
- `API_uart`: UART de consola en polling. `uartInit()` configura 115200 8N1 e imprime la configuración. Valida parámetros y el resultado del port.
- `API_gpio`: LED de usuario (`gpioLedOn/Off/Toggle`, `gpioLedIsOn`) y pulsador (`gpioButtonIsPressed`).
- `API_cmdparser`: parser de comandos por UART con una MEF de 5 estados (`IDLE`, `RECEIVING`, `PROCESS`, `EXEC`, `ERROR`).
- `API_common`, `API_delay`, `API_debounce`: tipos comunes, retardo no bloqueante y antirrebote.

Comandos: `HELP`, `LED ON|OFF|TOGGLE`, `STATUS`, `BAUD?`, `BAUD=<9600..921600>`.

## Prueba

```bash
picocom -b 115200 /dev/ttyACM0
```
