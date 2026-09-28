/*
 * API_cmdparser.c
 *
 *  Created on: 24 sept 2026
 *      Author: elvisdt
 *
 *  Parser de comandos por UART (polling) con MEF de 5 estados:
 *
 *    IDLE ──(char normal)──> RECEIVING ──(\r o \n)──> PROCESS ──(ok)──> EXEC ──> IDLE
 *                                │                       │
 *                                └──(overflow)──> ERROR <┘(cmd/args inválidos)
 *                                                   │
 *                                                   └──> IDLE
 */

#include "API_cmdparser.h"
#include "API_uart.h"
#include "main.h"          /* LD2_Pin, LD2_GPIO_Port */
#include <string.h>
#include <ctype.h>

#define CMD_BYTES_PER_POLL  16U
#define CMD_DELIMITERS      " \t"

/* Estados de la MEF */
typedef enum {
	CMD_IDLE = 0,
	CMD_RECEIVING,
	CMD_PROCESS,
	CMD_EXEC,
	CMD_ERROR
} cmd_state_t;

/* Comandos reconocidos */
typedef enum {
	CMD_ID_HELP = 0,
	CMD_ID_LED_ON,
	CMD_ID_LED_OFF,
	CMD_ID_LED_TOGGLE,
	CMD_ID_STATUS
} cmd_id_t;

static cmd_state_t  state = CMD_IDLE;
static cmd_status_t lastError = CMD_OK;
static cmd_id_t     cmdId = CMD_ID_HELP;

static char    line[CMD_MAX_LINE];     /* línea recibida                    */
static uint8_t lineLen = 0;
static char   *tokens[CMD_MAX_TOKENS];
static uint8_t tokenCount = 0;

static bool_t  prevWasCR = false;      /* para tratar \r\n como un solo fin  */
static bool_t  discardLine = false;    /* descartar resto de línea larga     */

/* Funciones privadas */
static void   cmdOnChar(uint8_t c);
static void   cmdProcessLine(void);
static void   cmdExec(void);
static void   cmdPrintError(void);
static bool_t cmdIsTerminator(uint8_t c);
static bool_t cmdIsComment(const char *s);
static void   cmdToUpper(char *s);

/* ------------------------------------------------------------------------- */

void cmdParserInit(void)
{
	state       = CMD_IDLE;
	lastError   = CMD_OK;
	lineLen     = 0;
	tokenCount  = 0;
	prevWasCR   = false;
	discardLine = false;

	cmdPrintHelp();
}

void cmdPoll(void)
{
	for (uint8_t i = 0; i < CMD_BYTES_PER_POLL; i++) {

		/* '\0' como centinela: si sigue en '\0' no llegó nada (timeout) */
		uint8_t c = '\0';
		uartReceiveStringSize(&c, 1);
		if (c == '\0') {
			return;
		}

		cmdOnChar(c);

		/* Estados que no consumen caracteres: se resuelven en el mismo poll */
		if (state == CMD_PROCESS) {
			cmdProcessLine();           /* -> EXEC, ERROR o IDLE */
		}

		if (state == CMD_EXEC) {
			cmdExec();
			state = CMD_IDLE;
		}
		else if (state == CMD_ERROR) {
			cmdPrintError();
			state = CMD_IDLE;
		}
	}
}

void cmdPrintHelp(void)
{
	uartSendString((uint8_t *)
			"Comandos:\r\n"
			"  HELP\r\n"
			"  LED ON | LED OFF | LED TOGGLE\r\n"
			"  STATUS\r\n");
}

/* ------------------------------------------------------------------------- */

/* Estados IDLE y RECEIVING: arman la línea caracter a caracter */
static void cmdOnChar(uint8_t c)
{
	/* '\n' justo después de '\r' es el mismo fin de línea: se ignora */
	if (c == '\n' && prevWasCR) {
		prevWasCR = false;
		return;
	}
	prevWasCR = (c == '\r');

	/* eco: el terminador se devuelve como \r\n (nada si se descarta la línea) */
	if (discardLine) {
		/* sin eco */
	}
	else if (cmdIsTerminator(c)) {
		uartSendString((uint8_t *)"\r\n");
	}
	else {
		uartSendStringSize(&c, 1);
	}

	switch (state) {

	case CMD_IDLE:
		/* después de un overflow se descarta hasta el fin de esa línea */
		if (discardLine) {
			if (cmdIsTerminator(c)) {
				discardLine = false;
			}
			break;
		}
		/* espera el primer caracter que no sea terminador */
		if (!cmdIsTerminator(c)) {
			lineLen = 0;
			line[lineLen++] = (char)c;
			state = CMD_RECEIVING;
		}
		break;

	case CMD_RECEIVING:
		if (cmdIsTerminator(c)) {
			line[lineLen] = '\0';
			state = CMD_PROCESS;
		}
		else if (lineLen < (CMD_MAX_LINE - 1U)) {   /* deja lugar al '\0' */
			line[lineLen++] = (char)c;
		}
		else {
			lastError   = CMD_ERR_OVERFLOW;
			discardLine = true;
			state       = CMD_ERROR;
		}
		break;

	default:
		/* PROCESS, EXEC y ERROR se resuelven en cmdPoll() */
		break;
	}
}

/* Estado PROCESS: comentarios, tokeniza y valida comando + argumentos */
static void cmdProcessLine(void)
{
	if (cmdIsComment(line)) {
		state = CMD_IDLE;
		return;
	}

	/* separa por espacios/tabs (múltiples se ignoran) */
	tokenCount = 0;
	char *tok = strtok(line, CMD_DELIMITERS);

	while (tok != NULL) {
		if (tokenCount >= CMD_MAX_TOKENS) {
			lastError = CMD_ERR_ARG;
			state = CMD_ERROR;
			return;
		}
		cmdToUpper(tok);                 /* case-insensitive */
		tokens[tokenCount++] = tok;
		tok = strtok(NULL, CMD_DELIMITERS);
	}

	if (tokenCount == 0) {               /* línea con solo espacios */
		state = CMD_IDLE;
		return;
	}

	lastError = CMD_OK;

	if (strcmp(tokens[0], "HELP") == 0) {
		cmdId = CMD_ID_HELP;
		if (tokenCount != 1) lastError = CMD_ERR_ARG;
	}
	else if (strcmp(tokens[0], "STATUS") == 0) {
		cmdId = CMD_ID_STATUS;
		if (tokenCount != 1) lastError = CMD_ERR_ARG;
	}
	else if (strcmp(tokens[0], "LED") == 0) {
		if (tokenCount != 2)                        lastError = CMD_ERR_ARG;
		else if (strcmp(tokens[1], "ON") == 0)      cmdId = CMD_ID_LED_ON;
		else if (strcmp(tokens[1], "OFF") == 0)     cmdId = CMD_ID_LED_OFF;
		else if (strcmp(tokens[1], "TOGGLE") == 0)  cmdId = CMD_ID_LED_TOGGLE;
		else                                        lastError = CMD_ERR_ARG;
	}
	else {
		lastError = CMD_ERR_UNKNOWN;
	}

	state = (lastError == CMD_OK) ? CMD_EXEC : CMD_ERROR;
}

/* Estado EXEC: ejecuta la acción del comando validado */
static void cmdExec(void)
{
	switch (cmdId) {

	case CMD_ID_HELP:
		cmdPrintHelp();
		break;

	case CMD_ID_LED_ON:
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
		uartSendString((uint8_t *)"OK\r\n");
		break;

	case CMD_ID_LED_OFF:
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
		uartSendString((uint8_t *)"OK\r\n");
		break;

	case CMD_ID_LED_TOGGLE:
		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		uartSendString((uint8_t *)"OK\r\n");
		break;

	case CMD_ID_STATUS:
		if (HAL_GPIO_ReadPin(LD2_GPIO_Port, LD2_Pin) == GPIO_PIN_SET) {
			uartSendString((uint8_t *)"LED is ON\r\n");
		}
		else {
			uartSendString((uint8_t *)"LED is OFF\r\n");
		}
		break;

	default:
		break;
	}
}

/* Estado ERROR: mensaje según el error */
static void cmdPrintError(void)
{
	switch (lastError) {
	case CMD_ERR_OVERFLOW: uartSendString((uint8_t *)"\r\nERROR: line too long\r\n"); break;
	case CMD_ERR_UNKNOWN:  uartSendString((uint8_t *)"ERROR: unknown command\r\n"); break;
	case CMD_ERR_ARG:      uartSendString((uint8_t *)"ERROR: bad arguments\r\n");   break;
	case CMD_ERR_SYNTAX:   uartSendString((uint8_t *)"ERROR: syntax\r\n");          break;
	default:                                                                        break;
	}
}

static bool_t cmdIsTerminator(uint8_t c)
{
	return (c == '\r') || (c == '\n');
}

/* true si la línea empieza con '#' o "//" (ignorando espacios iniciales) */
static bool_t cmdIsComment(const char *s)
{
	while (*s == ' ' || *s == '\t') {
		s++;
	}
	return (s[0] == '#') || (s[0] == '/' && s[1] == '/');
}

static void cmdToUpper(char *s)
{
	for (; *s != '\0'; s++) {
		*s = (char)toupper((unsigned char)*s);
	}
}
