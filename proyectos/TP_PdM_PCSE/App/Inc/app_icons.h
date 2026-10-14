/*
 * app_icons.h
 *
 *  Created on: 14 oct 2026
 *      Author: elvisdt
 *
 *  Íconos propios del LCD (CGRAM del HD44780, 5x8 puntos).
 *  Se cargan una vez con lcdCreateChar() al iniciar y después se imprimen
 *  como un caracter más con su código (ICON_xxx).
 *
 *  Para dibujar uno nuevo: cada fila es un byte, los 5 bits bajos son los
 *  puntos de izquierda a derecha (1 = encendido).
 */

#ifndef APP_INC_APP_ICONS_H_
#define APP_INC_APP_ICONS_H_

#include <stdint.h>

/* Códigos con que se imprimen (el 0 no se usa: es el fin de un string) */
#define ICON_BELL       '\x01'   /* alarma habilitada         */
#define ICON_BELL_OFF   '\x02'   /* alarma deshabilitada      */

/* Campana llena: alarma habilitada */
static const uint8_t iconBell[8] = {
	0x04,   /* ..#.. */
	0x0E,   /* .###. */
	0x0E,   /* .###. */
	0x0E,   /* .###. */
	0x1F,   /* ##### */
	0x00,   /* ..... */
	0x04,   /* ..#.. */
	0x00    /* ..... */
};

/* Campana vacía: alarma deshabilitada */
static const uint8_t iconBellOff[8] = {
	0x04,   /* ..#.. */
	0x0A,   /* .#.#. */
	0x0A,   /* .#.#. */
	0x0A,   /* .#.#. */
	0x1F,   /* ##### */
	0x00,   /* ..... */
	0x04,   /* ..#.. */
	0x00    /* ..... */
};

#endif /* APP_INC_APP_ICONS_H_ */
