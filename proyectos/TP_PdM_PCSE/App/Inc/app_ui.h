/*
 * app_ui.h
 *
 *  Created on: 4 oct 2026
 *      Author: elvisdt
 *
 *  MEF de interfaz de usuario: integra sensor, LCD, encoder, pulsadores,
 *  alarma y consola. main.c solo llama a uiInit() y uiUpdate().
 */

#ifndef APP_INC_APP_UI_H_
#define APP_INC_APP_UI_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { UI_INIT = 0, UI_SHOW, UI_SET_MIN, UI_SET_MAX, UI_ERROR } uiState_t;

void uiInit(void);     /* inicializa los módulos y la MEF de interfaz      */
void uiUpdate(void);   /* sensor cada 1 s, alarma, consola y MEF (no bloquea) */

#ifdef __cplusplus
}
#endif

#endif /* APP_INC_APP_UI_H_ */
