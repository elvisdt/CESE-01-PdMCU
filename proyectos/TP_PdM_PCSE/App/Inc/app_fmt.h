/*
 * app_fmt.h
 *
 *  Created on: 5 oct 2026
 *      Author: elvisdt
 *
 *  Formato de números con un decimal sin printf de float
 *  (newlib-nano no lo trae habilitado por defecto).
 */

#ifndef APP_INC_APP_FMT_H_
#define APP_INC_APP_FMT_H_

#include <stddef.h>

/** @brief Escribe v con un decimal, p. ej. "-3.5" o "1013.2". */
void fmtFloat1(char *buf, size_t len, float v);

#endif /* APP_INC_APP_FMT_H_ */
