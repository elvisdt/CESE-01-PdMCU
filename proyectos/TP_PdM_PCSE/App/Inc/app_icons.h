/**
 * @file    app_icons.h
 * @brief   Íconos propios del LCD.
 * @author  Elvis de la Torre
 * @date    14 oct 2026
 *
 * El HD44780 deja definir hasta 8 caracteres de 5x8 puntos en su CGRAM. Se
 * cargan una vez al arrancar con iconsLoad() y después se imprimen como un
 * caracter más, con su código ICON_xxx. Los dibujos están en app_icons.c.
 *
 * El código 0 no se usa porque en un string de C marca el final.
 */

#ifndef APP_INC_APP_ICONS_H_
#define APP_INC_APP_ICONS_H_

#include "API_common.h"

#define ICON_BELL       '\x01'   /**< Campana llena: alarma habilitada. */
#define ICON_BELL_OFF   '\x02'   /**< Campana vacía: alarma deshabilitada. */
#define ICON_SAVE       '\x03'   /**< Tilde: OK guarda. */
#define ICON_BACK       '\x04'   /**< Flecha a la izquierda: BACK vuelve. */
#define ICON_NEXT       '\x05'   /**< Flecha a la derecha: OK sigue. */

#define ICON_SAVE_STR   "\x03"     /**< ICON_SAVE para armar textos fijos. */

/**
 * @brief  Carga todos los íconos en el LCD. Requiere lcdInit().
 * @return false si el LCD no respondió.
 */
bool_t iconsLoad(void);

#endif /* APP_INC_APP_ICONS_H_ */
