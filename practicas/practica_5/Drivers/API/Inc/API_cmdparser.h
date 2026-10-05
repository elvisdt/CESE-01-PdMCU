/*
 * API_cmdparser.h
 *
 *  Created on: 24 sept 2026
 *      Author: elvisdt
 *
 *  Parser de comandos por UART (polling, no bloqueante).
 */

#ifndef API_INC_API_CMDPARSER_H_
#define API_INC_API_CMDPARSER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "API_common.h"

/**
 * @brief  Inicializa el parser y muestra la ayuda por UART.
 *         Requiere uartInit() previo.
 */
void cmdParserInit(void);

/**
 * @brief  MEF del parser. Llamar periódicamente desde el bucle principal.
 *         Procesa hasta 16 bytes por invocación (no bloqueante).
 */
void cmdPoll(void);

/**
 * @brief  Imprime por UART la lista de comandos disponibles.
 */
void cmdPrintHelp(void);

#ifdef __cplusplus
}
#endif

#endif /* API_INC_API_CMDPARSER_H_ */
