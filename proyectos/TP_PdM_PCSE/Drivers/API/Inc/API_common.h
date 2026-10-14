/**
 * @file    API_common.h
 * @brief   Tipos comunes a todos los módulos API.
 * @author  Elvis de la Torre
 * @date    24 sept 2026
 *
 * Los tipos tick_t y bool_t vienen de la práctica 3. Están acá y no en
 * API_delay.h para que cualquier módulo los use sin depender del retardo.
 */

#ifndef API_INC_API_COMMON_H_
#define API_INC_API_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t tick_t;   /**< Tiempo en milisegundos. */
typedef bool     bool_t;   /**< Booleano de las prácticas. */

/**
 * @brief Error del que un módulo no puede recuperarse.
 *
 * Detiene el sistema. Se usa ante parámetros que solo pueden ser un error
 * de programación (un puntero NULL a un delay, por ejemplo), no ante fallas
 * esperables como un sensor desconectado.
 */
void Error_API_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_COMMON_H_ */
