# Práctica 4 - Antirrebote por software (MEF)

## Datos del Autor
* **Alumno:** Elvis de la Torre
* **Carrera:** (CESE) - FIUBA
* **Materia:** Programación de Microprocesadores

## Enunciado
[Práctica 4 - debounce.pdf](../../docs/clase-04/Práctica%204%20-%20debounce.pdf)

## Descripción del Proyecto

Se implementa una máquina de estados finitos (MEF) para el antirrebote por software del pulsador B1 de la NUCLEO-F4, encapsulada en el módulo `API_debounce` (`Drivers/API`), reutilizando los retardos no bloqueantes de `API_delay` (práctica 3).

- MEF de 4 estados (`BUTTON_UP`, `BUTTON_FALLING`, `BUTTON_DOWN`, `BUTTON_RAISING`), con tiempo de confirmación de 40 ms.
- `readKey()` informa si hubo una pulsación confirmada y se autoresetea al leerla.
- El LED2 parpadea en forma continua, alternando su período entre 500 ms y 100 ms cada vez que `readKey()` confirma una pulsación.

## Contenido

- `Drivers/API/Inc` y `Drivers/API/Src`: `API_delay.h/.c` (retardos no bloqueantes) y `API_debounce.h/.c` (MEF antirrebote). El `enum debounceState_t` y todas las variables de estado son privadas (`static`) al `.c`; el `.h` solo expone `debounceFSM_init`, `debounceFSM_update` y `readKey`.
- `Core/Src/main.c`: inicializa ambos módulos y en el loop principal llama a `debounceFSM_update()`, consulta `readKey()` para alternar el período de parpadeo, y actualiza el LED con un `delay_t` propio.

## Para pensar

**1) ¿Es adecuado el control de los parámetros pasados por el usuario? ¿Se controla que sean válidos y estén en rango?**

Las funciones públicas de `API_debounce` (`debounceFSM_init`, `debounceFSM_update`, `readKey`) no reciben parámetros, así que no hay nada que validar en esa interfaz. Internamente sí se apoyan en `API_delay`, cuyas funciones (`delayInit`, `delayRead`, `delayWrite`) validan puntero `NULL` y duración mayor a 0, llamando a `Error_APIdelay_Handler` si algo es inválido.

**2) ¿Se nota una mejora en la detección de pulsaciones respecto a la práctica 0? ¿Se pierden pulsaciones? ¿Hay falsos positivos?**

A diferencia de la práctica 0, la lectura del botón era inmediata, entonces hasta un rebote podía generar una doble detección o un cambio de estado instantáneo falso. Con esta implementación, probada en la placa, la lectura es más óptima y certera para la detección del cambio de estado, y se evitan esos falsos positivos.

**3) ¿Es adecuada la temporización con la que se llama a `debounceFSM_update()` y a `readKey()`? ¿Qué pasaría con un tiempo mucho más grande o más chico?**

Sí: llamarla en cada vuelta del loop da la mejor resolución posible. Con un período mucho más grande se podrían perder pulsaciones cortas o el filtrado de 40ms dejaría de ser preciso; mucho más chico no rompe nada, solo gasta CPU de más. `readKey()` no depende de temporización, se puede llamar cuando haga falta sin perder eventos.