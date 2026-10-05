/*
 * API_cmdparser.h
 *
 *  Created on: 24 sept 2026
 *      Author: elvisdt
 *
 *  Parser de comandos por UART (polling, no bloqueante) de la práctica 5,
 *  generalizado: la aplicación le pasa su tabla de comandos. El parser arma
 *  la línea, la separa en palabras, valida cantidad de argumentos y llama
 *  al handler. HELP está incorporado y lista la tabla.
 */

#ifndef API_INC_API_CMDPARSER_H_
#define API_INC_API_CMDPARSER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

#define CMD_MAX_ARGS   4U      /* incluye el nombre: "SET T 10 30" = 4 */

typedef enum {
	CMD_OK = 0,
	CMD_ERR_OVERFLOW,
	CMD_ERR_SYNTAX,
	CMD_ERR_UNKNOWN,
	CMD_ERR_ARG
} cmdStatus_t;

/* argv[0] es el comando; todas las palabras llegan en mayúsculas */
typedef cmdStatus_t (*cmdHandler_t)(uint8_t argc, char *argv[]);

typedef struct {
	const char  *name;       /* en mayúsculas, p. ej. "GET"           */
	uint8_t      minArgs;    /* argumentos sin contar el nombre       */
	uint8_t      maxArgs;
	cmdHandler_t handler;
	const char  *help;       /* una línea para HELP                   */
} cmdEntry_t;

/**
 * @brief  Inicializa el parser con la tabla de comandos y muestra la ayuda.
 *         Requiere uartInit() previo.
 * @retval false si la tabla es NULL o vacía.
 */
bool_t cmdParserInit(const cmdEntry_t *table, uint8_t count);

/** @brief MEF del parser. Llamar en cada vuelta del lazo (no bloquea). */
void   cmdPoll(void);

/** @brief Imprime la lista de comandos. */
void   cmdPrintHelp(void);

/**
 * @brief  Convierte "12", "-3.5" o "1013.2" a float (sin usar strtof).
 * @retval false si el texto no es un número válido.
 */
bool_t cmdParseFloat(const char *s, float *value);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_CMDPARSER_H_ */
