/**
 * @file    app_fmt.h
 * @brief   Números con un decimal sin printf de float.
 * @author  Elvis de la Torre
 * @date    5 oct 2026
 *
 * newlib-nano no trae %f habilitado por defecto y activarlo agrega varios KB.
 * Para mostrar 24.8 °C alcanza con separar la parte entera del decimal.
 */

#ifndef APP_INC_APP_FMT_H_
#define APP_INC_APP_FMT_H_

#include <stddef.h>

#define FMT_FLOAT1_LEN   8U   /**< Alcanza para "-1100.0" y el '\0'. */

/**
 * @brief Escribe v redondeado a un decimal, por ejemplo "-3.5" o "1013.2".
 * @param buf Destino (no NULL).
 * @param len Tamaño de buf.
 * @param v   Valor a escribir.
 */
void fmtFloat1(char *buf, size_t len, float v);

#endif /* APP_INC_APP_FMT_H_ */
