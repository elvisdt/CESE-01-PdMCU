/*
 * API_cmdparser.c
 *
 *  Created on: 24 sept 2026
 *      Author: elvisdt
 *
 *  MEF de 5 estados (práctica 5):
 *
 *    IDLE ──(char normal)──> RECEIVING ──(\r o \n)──> PROCESS ──(ok)──> EXEC ──> IDLE
 *                                │                       │
 *                                └──(overflow)──> ERROR <┘(cmd/args inválidos)
 *                                                   │
 *                                                   └──> IDLE
 */

#include "API_cmdparser.h"
#include "API_uart.h"

#include <string.h>
#include <ctype.h>
#include <stddef.h>

/* Private defines -----------------------------------------------------------*/
#define CMD_MAX_LINE        64U         /* incluye '\0'  */
#define CMD_BYTES_PER_POLL  16U
#define CMD_DELIMITERS      " \t"
#define CMD_FLOAT_MAX_DIGITS 7U

/* Private types -------------------------------------------------------------*/
typedef enum {
	CMD_IDLE = 0,
	CMD_RECEIVING,
	CMD_PROCESS,
	CMD_EXEC,
	CMD_ERROR
} cmdState_t;

/* Private variables ---------------------------------------------------------*/
static const cmdEntry_t *cmdTable = NULL;
static uint8_t           cmdCount = 0;
static const cmdEntry_t *cmdFound = NULL;   /* comando validado en PROCESS */

static cmdState_t  state     = CMD_IDLE;
static cmdStatus_t lastError = CMD_OK;

static char    line[CMD_MAX_LINE];
static uint8_t lineLen = 0;
static char   *argv[CMD_MAX_ARGS];
static uint8_t argc = 0;

static bool_t  prevWasCR   = false;
static bool_t  discardLine = false;

/* Private function prototypes -----------------------------------------------*/
static void   cmdOnChar(uint8_t c);
static void   cmdProcessLine(void);
static void   cmdExec(void);
static void   cmdPrintError(void);
static bool_t cmdIsTerminator(uint8_t c);
static bool_t cmdIsComment(const char *s);
static void   cmdToUpper(char *s);

/* Public functions ----------------------------------------------------------*/

bool_t cmdParserInit(const cmdEntry_t *table, uint8_t count)
{
	if (table == NULL || count == 0U) {
		return false;
	}
	cmdTable    = table;
	cmdCount    = count;
	state       = CMD_IDLE;
	lastError   = CMD_OK;
	lineLen     = 0;
	argc        = 0;
	prevWasCR   = false;
	discardLine = false;

	cmdPrintHelp();
	return true;
}

void cmdPoll(void)
{
	if (cmdTable == NULL) {
		return;
	}

	for (uint8_t i = 0; i < CMD_BYTES_PER_POLL; i++) {
		uint8_t c;
		if (!uartReceiveByte(&c)) {
			return;                     /* no llegó nada */
		}

		cmdOnChar(c);

		if (state == CMD_PROCESS) {
			cmdProcessLine();           /* -> EXEC, ERROR o IDLE */
		}
		if (state == CMD_EXEC) {
			cmdExec();                  /* -> IDLE o ERROR */
		}
		if (state == CMD_ERROR) {
			cmdPrintError();
			state = CMD_IDLE;
		}
	}
}

void cmdPrintHelp(void)
{
	uartSendString((uint8_t *)"Comandos:\r\n  HELP\r\n");
	for (uint8_t i = 0; i < cmdCount; i++) {
		uartSendString((uint8_t *)"  ");
		uartSendString((uint8_t *)cmdTable[i].help);
		uartSendString((uint8_t *)"\r\n");
	}
}

bool_t cmdParseFloat(const char *s, float *value)
{
	float   result = 0.0f;
	float   scale  = 1.0f;
	bool_t  neg    = false;
	bool_t  dot    = false;
	uint8_t digits = 0;

	if (s == NULL || value == NULL || *s == '\0') {
		return false;
	}
	if (*s == '-' || *s == '+') {
		neg = (*s == '-');
		s++;
	}
	for (; *s != '\0'; s++) {
		if (*s == '.' && !dot) {
			dot = true;
		} else if (isdigit((unsigned char)*s) && ++digits <= CMD_FLOAT_MAX_DIGITS) {
			if (dot) {
				scale /= 10.0f;
				result += (float)(*s - '0') * scale;
			} else {
				result = result * 10.0f + (float)(*s - '0');
			}
		} else {
			return false;
		}
	}
	if (digits == 0U) {
		return false;
	}
	*value = neg ? -result : result;
	return true;
}

/* Private functions ---------------------------------------------------------*/

/* IDLE y RECEIVING: arman la línea caracter a caracter (con eco) */
static void cmdOnChar(uint8_t c)
{
	if (c == '\n' && prevWasCR) {       /* \r\n es un solo fin de línea */
		prevWasCR = false;
		return;
	}
	prevWasCR = (c == '\r');

	if (!discardLine) {
		if (cmdIsTerminator(c)) {
			uartSendString((uint8_t *)"\r\n");
		} else {
			uartSendStringSize(&c, 1);
		}
	}

	switch (state) {
	case CMD_IDLE:
		if (discardLine) {              /* resto de una línea demasiado larga */
			if (cmdIsTerminator(c)) {
				discardLine = false;
			}
			break;
		}
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
		} else if (lineLen < (CMD_MAX_LINE - 1U)) {
			line[lineLen++] = (char)c;
		} else {
			lastError   = CMD_ERR_OVERFLOW;
			discardLine = true;
			state       = CMD_ERROR;
		}
		break;

	default:
		break;                          /* PROCESS, EXEC, ERROR: en cmdPoll() */
	}
}

/* PROCESS: comentarios, separa palabras, busca el comando y valida argumentos */
static void cmdProcessLine(void)
{
	if (cmdIsComment(line)) {
		state = CMD_IDLE;
		return;
	}

	argc = 0;
	char *tok = strtok(line, CMD_DELIMITERS);
	while (tok != NULL) {
		if (argc >= CMD_MAX_ARGS) {
			lastError = CMD_ERR_ARG;
			state = CMD_ERROR;
			return;
		}
		cmdToUpper(tok);
		argv[argc++] = tok;
		tok = strtok(NULL, CMD_DELIMITERS);
	}

	if (argc == 0U) {                   /* línea con solo espacios */
		state = CMD_IDLE;
		return;
	}

	if (strcmp(argv[0], "HELP") == 0) {
		cmdFound = NULL;                /* incorporado */
		state = (argc == 1U) ? CMD_EXEC : CMD_ERROR;
		lastError = (argc == 1U) ? CMD_OK : CMD_ERR_ARG;
		return;
	}

	for (uint8_t i = 0; i < cmdCount; i++) {
		if (strcmp(argv[0], cmdTable[i].name) == 0) {
			uint8_t nArgs = (uint8_t)(argc - 1U);
			if (nArgs < cmdTable[i].minArgs || nArgs > cmdTable[i].maxArgs) {
				lastError = CMD_ERR_ARG;
				state = CMD_ERROR;
			} else {
				cmdFound = &cmdTable[i];
				state = CMD_EXEC;
			}
			return;
		}
	}

	lastError = CMD_ERR_UNKNOWN;
	state = CMD_ERROR;
}

/* EXEC: llama al handler; si devuelve error se informa */
static void cmdExec(void)
{
	if (cmdFound == NULL) {
		cmdPrintHelp();
		state = CMD_IDLE;
		return;
	}
	lastError = cmdFound->handler(argc, argv);
	state = (lastError == CMD_OK) ? CMD_IDLE : CMD_ERROR;
}

static void cmdPrintError(void)
{
	switch (lastError) {
	case CMD_ERR_OVERFLOW: uartSendString((uint8_t *)"\r\nERROR: line too long\r\n"); break;
	case CMD_ERR_UNKNOWN:  uartSendString((uint8_t *)"ERROR: unknown command\r\n");   break;
	case CMD_ERR_ARG:      uartSendString((uint8_t *)"ERROR: bad arguments\r\n");     break;
	case CMD_ERR_SYNTAX:   uartSendString((uint8_t *)"ERROR: syntax\r\n");            break;
	default:                                                                          break;
	}
}

static bool_t cmdIsTerminator(uint8_t c)
{
	return (c == '\r') || (c == '\n');
}

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
