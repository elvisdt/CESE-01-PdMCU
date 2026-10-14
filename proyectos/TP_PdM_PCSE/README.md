# Estación de medición ambiental

Trabajo final de Programación de Microcontroladores (CESE - FIUBA).

La estación mide temperatura, humedad y presión con un BME280 cada un segundo y
muestra los valores en un LCD de 16x2. La temperatura y la humedad tienen
límites de alarma que se ajustan con el encoder; si alguna sale de su rango se
encienden el LED y el buzzer. La presión solo se muestra. Todo se puede
consultar y configurar también desde una consola UART.

Corre sobre una **NUCLEO-F446RE** y está armado a partir de las prácticas del curso:
el antirrebote de la práctica 4, el retardo no bloqueante de la 3 y la UART con
el intérprete de comandos de la 5.

## Hardware

| Componente | Interfaz | Pines |
|---|---|---|
| BME280 (0x76) | I2C1, 100 kHz | PB8 SCL, PB9 SDA |
| LCD 16x2 con PCF8574 (0x27) | I2C1 (mismo bus) | PB8, PB9 |
| Encoder KY-040 | TIM3 en modo encoder | PA6, PA7 |
| Pulsador del encoder (SW) | GPIO con pull-up | PB5 |
| Pulsador OK | GPIO con pull-up | PB10 (D6) |
| Pulsador BACK | GPIO con pull-up | PB4 (D5) |
| Buzzer activo | GPIO salida | PA10 (D2) |
| LED de alarma | GPIO salida (LD2) | PA5 |
| Consola | USART2 a 115200 8N1 (ST-LINK) | PA2, PA3 |

El LCD y el BME280 van alimentados con 5 V. El LCD se ve muy tenue con 3,3 V y
el módulo del BME280 que uso tiene regulador propio.

![Diagrama de bloques](docs/diagramas/diagrama_bloques.png)

## Cómo se usa

Al encender aparece una pantalla de bienvenida mientras arranca el sensor.
Después queda la pantalla principal:

```
T:24.8°C H: 55%       <- temperatura y humedad
P: 1007.1 hPa  🔔     <- presión y, en la última columna, el estado de la alarma
```

El ícono de la alarma es un caracter propio cargado en la CGRAM del LCD
(`App/Inc/app_icons.h`): campana llena si está habilitada, vacía si está
apagada. Cuando se dispara, la segunda fila muestra qué pasó, por ejemplo
`ALARMA T ALTA` o `ALARMA H BAJA`, y la campana parpadea.

La humedad va sin decimal: el sensor tiene ±3 %HR de precisión y así la fila
entra en las 16 columnas.

- **SW del encoder:** activa o desactiva la alarma.
- **OK:** entra a editar límites. Con el encoder se elige temperatura o humedad,
  OK, se ajusta el mínimo, OK, el máximo, y un último OK guarda.
- **BACK:** vuelve un paso atrás sin guardar.

Los límites arrancan en 10 a 35 °C y 20 a 80 %HR.

Si el mínimo queda mayor o igual que el máximo, o fuera del rango que mide el
sensor, no se guarda y en el LCD aparece "Rango invalido".

Por la consola (`picocom -b 115200 /dev/ttyACM0`):

| Comando | Qué hace |
|---|---|
| `GET` | muestra las tres mediciones |
| `LIM?` | muestra los límites de temperatura y humedad |
| `SET <T\|H> <min> <max>` | cambia los límites, por ejemplo `SET T 18 30` |
| `ALARM <ON\|OFF>` | activa o desactiva la alarma |
| `STATUS` | estado de la interfaz y de la alarma |
| `HELP` | lista de comandos |

La consola también avisa sola cuando cambia de estado la interfaz
(`UI: SHOW -> SEL_VAR`) o la alarma (`ALARMA: ARMED -> ACTIVE`), lo que ayuda
mucho al probar.

## Máquinas de estados

### Interfaz de usuario (`app_ui`)

Es la MEF principal del trabajo. Decide qué se muestra y qué hace cada botón.

![MEF de la interfaz](docs/diagramas/mef_ui.png)

- `UI_INIT`: arranca en dos pasos. Primero el bus I2C, el encoder y el LCD, que
  muestra la bienvenida. A los 1,5 s inicializa el sensor y hace la primera
  lectura. Si sale bien pasa a `UI_SHOW`; si no, a `UI_ERROR`.
- `UI_SHOW`: la pantalla principal. Es el estado en el que pasa la mayor parte del tiempo.
- `UI_SEL_VAR`: elige si se editan los límites de temperatura o de humedad.
- `UI_SET_MIN` / `UI_SET_MAX`: edita los límites sobre una copia. Recién al
  confirmar se pasan al módulo de alarma, que los valida.
- `UI_ERROR`: el sensor dejó de responder. Reintenta cada 2 s y, cuando vuelve,
  regresa a `UI_SHOW`.

Mientras tanto la medición, la alarma y la consola siguen corriendo en todos los
estados, así que se puede estar editando un límite y la alarma igual se dispara.

### Alarma (`API_alarm`)

![MEF de la alarma](docs/diagramas/mef_alarma.png)

- `DISABLED`: no evalúa nada.
- `ARMED`: evalúa cada medición nueva. Si la temperatura o la humedad salen de
  su rango pasa a `ACTIVE`.
- `ACTIVE`: el LED queda encendido y el buzzer suena intermitente (500 ms).
  Para volver a `ARMED` las dos tienen que entrar al rango con un margen de
  histéresis (0,5 °C y 2 %HR). Sin ese margen, una medición que
  oscila justo en el borde haría sonar y callar el buzzer a cada rato.

El antirrebote de los pulsadores (`API_debounce`) también es una MEF, la de la
práctica 4. Ahora tiene una instancia por cada uno de los tres pulsadores.

## Organización del código

```
App/                    lo propio de esta aplicación
  app_ui                MEF de interfaz y comandos de consola
  app_test              pruebas de un módulo por vez
Drivers/API/            módulos portables, ninguno incluye la HAL
Drivers/Port/STM32F4xx/ lo único que depende del micro
```

La idea fue separar en capas como en el ejemplo del LCD que vimos en Protocolos:

- `API_lcd`, `API_bme280` y `API_encoder` solo saben hablar con el dispositivo.
- Cada uno tiene un `*_port.c` que les da acceso al bus.
- `port_stm32f4xx.c` es el único archivo que llama a la HAL: comparte el I2C
  entre el LCD y el sensor, maneja el timer del encoder, la UART y los GPIO.

Si cambiara de micro, solo habría que reescribir la carpeta `Port`.

Algunas decisiones:

- La UART, el I2C y el TIM3 los inicializa la capa de port, no `main.c`. En CubeMX
  están marcados con *Do Not Generate Function Call*.
- Nada bloquea el lazo principal: todo se temporiza con `API_delay`.
- Las variables de cada módulo son `static`, y desde afuera se accede a ellas
  solo con funciones (por ejemplo `alarmGetLimits()` y `alarmSetLimits()`).
- El driver del BME280 detecta si el sensor se reinició (lee 0x80000 en vez de
  una medición) y obliga a reinicializarlo.

| Módulo | Qué hace | Origen |
|---|---|---|
| `API_delay` | retardos no bloqueantes | práctica 3 |
| `API_debounce` | antirrebote de 3 pulsadores | práctica 4 |
| `API_uart` | envío y recepción por USART2 | práctica 5 |
| `API_cmdparser` | intérprete de comandos con tabla | práctica 5 |
| `API_gpio` | LED y buzzer | práctica 5 |
| `API_encoder` | pasos del encoder a partir del TIM3 | nuevo |
| `API_lcd` | LCD 16x2 por I2C | nuevo |
| `API_bme280` | lectura y compensación del sensor | nuevo |
| `API_alarm` | límites, histéresis y salidas | nuevo |
| `app_ui` | interfaz de usuario | nuevo |

## Compilar y probar

Se abre con STM32CubeIDE (*File › Import › Existing Projects into Workspace*).
Si al compilar falta algún archivo de la HAL, se regenera con *Generate Code*
desde `TP_PdM_PCSE.ioc`.

Para probar de a un módulo se elige la prueba en `App/Inc/app_test.h`:

```c
#define APP_TEST  TEST_LCD
```

Las pruebas disponibles son `TEST_DELAY`, `TEST_UART`, `TEST_DEBOUNCE`,
`TEST_ENCODER`, `TEST_I2C_SCAN`, `TEST_LCD`, `TEST_BME280` y `TEST_ALARM`.
Con `TEST_NONE` corre la estación completa.

La documentación del código se genera con `doxygen` y queda en
`docs/doxygen/html/index.html`.
