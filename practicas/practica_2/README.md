# Práctica 2 - Retardos no bloqueantes

Enunciado: [Práctica 2.pdf](../../docs/clase-02/Práctica%202.pdf)

## Contenido

- **Punto 1**: módulo de retardos no bloqueantes (`delayInit`, `delayRead`, `delayWrite`) en `main.h` / `main.c`.
- **Punto 2**: parpadeo del LED2, 100 ms ON / 100 ms OFF, usando el módulo del Punto 1.
- **Punto 3 (opcional)**: parpadeo con patrón de tiempos (`blink_steps[]`), 5 veces a 1 s, 5 veces a 200 ms, 5 veces a 100 ms.

El `#define ENABLE_POINT_3` en `main.c` alterna entre el Punto 2 (`0`) y el Punto 3 (`1`).

## Para pensar

**1) ¿Se pueden cambiar los tiempos en un solo lugar, o están hardcodeados?**

- Para el Punto 2, NO: el valor 100 (ms) está hardcodeado directo al inicializar el `delayInit`.
- Para el Punto 3, sí: los tiempos y repeticiones se pueden cambiar en un solo lugar, en la estructura y arreglo `blink_steps[]`, que centraliza el patrón.

**2) ¿Qué bibliotecas estándar se debieron agregar para que compile?**

`stdint.h` (para `uint32_t`, `uint8_t`) y `stdbool.h` (para `bool`), bibliotecas estándar de C.

**3) ¿Es adecuado el control de los parámetros pasados por el usuario?**

No, ninguna función valida sus parámetros (punteros `NULL`, `duration = 0`, `total_steps = 0`).

**4) ¿Cuán reutilizable es el código implementado?**

- `delayInit`/`delayRead`/`delayWrite`: reutilizable, reciben `delay_t` por puntero, no usan variables globales.
- Lógica del patrón (Punto 3): poco reutilizable, está escrita directo en el `while(1)` con variables globales fijas, ahí se podría pasar a funciones y mejorar la integración.

**5) ¿Cuán sencillo resulta cambiar el patrón de tiempos de parpadeo?**

Al estar en un arreglo y como variable global, se podría editar el arreglo `blink_steps[]`, sin manipular la lógica.
