/**
 * @file    API_cmdparser.h
 * @brief   Intérprete de comandos por UART (práctica 5).
 * @author  Elvis de la Torre
 * @date    24 sept 2026
 *
 * Es la MEF de cinco estados de la práctica 5, con una diferencia: los
 * comandos no están fijos adentro del módulo. La aplicación le pasa una
 * tabla con el nombre de cada comando, cuántos argumentos acepta y qué
 * función lo atiende. Así el mismo parser sirve para la estación y para
 * las pruebas sin tocarlo. HELP ya viene incorporado.
 *
 * Las líneas terminan en \\r, \\n o \\r\\n, las palabras se separan con
 * espacios o tabs, y todo se pasa a mayúsculas antes de buscar el comando.
 * Las líneas que empiezan con # o // se ignoran.
 */

#ifndef API_INC_API_CMDPARSER_H_
#define API_INC_API_CMDPARSER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

#define CMD_MAX_ARGS   4U   /**< Palabras por línea, contando el comando ("SET T 10 30"). */

/** @brief Resultado de procesar una línea. */
typedef enum {
	CMD_OK = 0,         /**< Se ejecutó. */
	CMD_ERR_OVERFLOW,   /**< La línea no entra en el buffer. */
	CMD_ERR_SYNTAX,     /**< Formato inválido. */
	CMD_ERR_UNKNOWN,    /**< El comando no está en la tabla. */
	CMD_ERR_ARG         /**< Cantidad o valor de argumentos inválido. */
} cmdStatus_t;

/**
 * @brief Función que atiende un comando.
 *
 * argv[0] es el nombre del comando y todas las palabras llegan en
 * mayúsculas. Va un puntero a función por comando para que el parser no
 * conozca a la aplicación; es la forma de encapsulamiento que propone el
 * apunte de device drivers de Protocolos.
 */
typedef cmdStatus_t (*cmdHandler_t)(uint8_t argc, char *argv[]);

/** @brief Una fila de la tabla de comandos. */
typedef struct {
	const char  *name;      /**< Nombre en mayúsculas, por ejemplo "GET". */
	uint8_t      minArgs;   /**< Mínimo de argumentos, sin contar el nombre. */
	uint8_t      maxArgs;   /**< Máximo de argumentos, sin contar el nombre. */
	cmdHandler_t handler;   /**< Función que lo atiende. */
	const char  *help;      /**< Línea que muestra HELP. */
} cmdEntry_t;

/**
 * @brief  Toma la tabla de comandos y muestra la ayuda. Requiere uartInit().
 * @param  table Tabla de comandos (no NULL, debe durar todo el programa).
 * @param  count Cantidad de filas (mayor que 0).
 * @return false si la tabla es inválida.
 */
bool_t cmdParserInit(const cmdEntry_t *table, uint8_t count);

/** @brief Avanza la MEF con lo que haya llegado. Se llama en cada vuelta del lazo. */
void cmdPoll(void);

/** @brief Lista los comandos disponibles. */
void cmdPrintHelp(void);

/**
 * @brief  Convierte un argumento como "12", "-3.5" o "1013.2" en float.
 *
 * Se hace a mano porque strtof() trae bastante código y acá alcanza con
 * enteros y un punto decimal.
 *
 * @param  s     Texto a convertir (no NULL).
 * @param  value Destino (no NULL).
 * @return false si el texto no es un número.
 */
bool_t cmdParseFloat(const char *s, float *value);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_CMDPARSER_H_ */
