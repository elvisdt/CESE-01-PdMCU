/*
 * API_common.c
 *
 *  Created on: Sep 27, 2026
 *      Author: elvisdt
 */

#include "API_common.h"
#include "API_port.h"

void Error_API_Handler(void)
{
	portFatalError();
}
