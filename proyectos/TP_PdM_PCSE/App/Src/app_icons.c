/**
 * @file    app_icons.c
 * @brief   Dibujos de los íconos del LCD.
 * @author  Elvis de la Torre
 * @date    14 oct 2026
 *
 * Cada fila del dibujo es un byte; los 5 bits bajos son los puntos de
 * izquierda a derecha. El comentario de cada fila muestra cómo se ve.
 */

#include "app_icons.h"
#include "API_lcd.h"

/* Dibujos -------------------------------------------------------------------*/

static const uint8_t iconBell[LCD_CHAR_ROWS] = {
	0x04,   /* ..#.. */
	0x0E,   /* .###. */
	0x0E,   /* .###. */
	0x0E,   /* .###. */
	0x1F,   /* ##### */
	0x00,   /* ..... */
	0x04,   /* ..#.. */
	0x00    /* ..... */
};

static const uint8_t iconBellOff[LCD_CHAR_ROWS] = {
	0x04,   /* ..#.. */
	0x0A,   /* .#.#. */
	0x0A,   /* .#.#. */
	0x0A,   /* .#.#. */
	0x1F,   /* ##### */
	0x00,   /* ..... */
	0x04,   /* ..#.. */
	0x00    /* ..... */
};

static const uint8_t iconSave[LCD_CHAR_ROWS] = {
	0x00,   /* ..... */
	0x01,   /* ....# */
	0x03,   /* ...## */
	0x16,   /* #.##. */
	0x1C,   /* ###.. */
	0x08,   /* .#... */
	0x00,   /* ..... */
	0x00    /* ..... */
};

static const uint8_t iconBack[LCD_CHAR_ROWS] = {
	0x00,   /* ..... */
	0x04,   /* ..#.. */
	0x08,   /* .#... */
	0x1F,   /* ##### */
	0x08,   /* .#... */
	0x04,   /* ..#.. */
	0x00,   /* ..... */
	0x00    /* ..... */
};

static const uint8_t iconNext[LCD_CHAR_ROWS] = {
	0x00,   /* ..... */
	0x04,   /* ..#.. */
	0x02,   /* ...#. */
	0x1F,   /* ##### */
	0x02,   /* ...#. */
	0x04,   /* ..#.. */
	0x00,   /* ..... */
	0x00    /* ..... */
};

/* Tabla de carga: código y dibujo de cada ícono. */
static const struct {
	char           code;
	const uint8_t *pattern;
} icons[] = {
	{ ICON_BELL,     iconBell    },
	{ ICON_BELL_OFF, iconBellOff },
	{ ICON_SAVE,     iconSave    },
	{ ICON_BACK,     iconBack    },
	{ ICON_NEXT,     iconNext    },
};

#define ICON_COUNT   (sizeof(icons) / sizeof(icons[0]))

/* Funciones públicas --------------------------------------------------------*/

bool_t iconsLoad(void)
{
	for (uint8_t i = 0U; i < (uint8_t)ICON_COUNT; i++) {
		if (lcdCreateChar((uint8_t)icons[i].code, icons[i].pattern) != LCD_OK) {
			return false;
		}
	}
	return true;
}
