/**
 * @file    app_ui.h
 * @brief   Aplicación de la estación: interfaz de usuario y consola.
 * @author  Elvis de la Torre
 * @date    4 oct 2026
 *
 * Reúne todos los módulos. main.c solo llama a uiInit() una vez y a
 * uiUpdate() en cada vuelta del lazo.
 */

#ifndef APP_INC_APP_UI_H_
#define APP_INC_APP_UI_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Inicializa los módulos que no dependen del bus y deja la MEF en UI_INIT. */
void uiInit(void);

/**
 * @brief Una vuelta de la aplicación. No bloquea.
 *
 * Lee los pulsadores y el encoder, atiende la consola, mide cada segundo,
 * evalúa la alarma, avanza la MEF de interfaz y redibuja el LCD si cambió algo.
 */
void uiUpdate(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_INC_APP_UI_H_ */
