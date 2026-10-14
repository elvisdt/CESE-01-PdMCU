/**
 * @file    API_common.c
 * @brief   Manejo de errores irrecuperables de los módulos API.
 * @author  Elvis de la Torre
 * @date    27 sept 2026
 */

#include "API_common.h"
#include "API_port.h"

void Error_API_Handler(void)
{
	portFatalError();
}
