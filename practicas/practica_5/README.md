# Práctica 5 - UART y parser de comandos (MEF)

## Datos del Autor
* **Alumno:** Elvis de la Torre
* **Carrera:** (CESE) - FIUBA
* **Materia:** Programación de Microprocesadores

## Enunciado
[Práctica 5.pdf](../../docs/clase-05/Práctica%205.pdf)

## Descripción

- `API_uart`: capa de acceso a la USART2 en modo polling. `uartInit()` configura 115200 8N1 e imprime la configuración. Todas las funciones validan sus parámetros y el retorno de la HAL.
- `API_cmdparser`: parser de comandos por UART con una MEF de 5 estados (`IDLE`, `RECEIVING`, `PROCESS`, `EXEC`, `ERROR`).

Comandos: `HELP`, `LED ON|OFF|TOGGLE`, `STATUS`, `BAUD?`, `BAUD=<9600..921600>`.

## Prueba

```bash
picocom -b 115200 /dev/ttyACM0
```
