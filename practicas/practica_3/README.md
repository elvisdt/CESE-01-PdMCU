# Práctica 3 - Modularización

Enunciado: [Práctica 3 - Modularización.pdf](../../docs/clase-03/Práctica%203%20-%20Modularización.pdf)

## Contenido

- Módulo de retardos no bloqueantes encapsulado en `Drivers/API` (`API_delay.h` / `API_delay.c`), separado de `main.c`.
- Parpadeo secuencial del LED2 (`blink_steps[]`) con `delayWrite` y una única variable `delay_t`, duty 50%.
- `delayIsRunning` implementada y usada antes de cada `delayWrite`.
- Se agregó `Error_APIdelay_Handler` (no existía en la Práctica 2) para validar los parámetros (`NULL`, `duration == 0`) en las funciones de la API.

## Para pensar

**1) ¿Es suficientemente clara la consigna 2, o da lugar a implementaciones distintas?**

En general sí, pero no dice cómo manejar el paso de un tiempo a otro de la secuencia (conteo de toggles, reinicio, u otro)

**2) ¿Se puede cambiar el tiempo de encendido del led en un solo lugar? ¿Hay números "mágicos"?**

Sí, todo está centralizado en el arreglo `blink_steps[]`.

**3) ¿Qué bibliotecas estándar se debieron agregar a `API_delay.h`?**

`stdint.h` (para `uint32_t`) y `stdbool.h` (para `bool`)

**4) ¿Es adecuado el control de los parámetros pasados por el usuario?**

Sí: `delayInit`, `delayWrite` y `delayIsRunning` validan puntero `NULL`, y `delayInit`/`delayWrite` validan `duration == 0`, llamando a `Error_APIdelay_Handler` si algo es inválido.
