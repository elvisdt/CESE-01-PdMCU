/*
 * app_test.h
 *
 *  Created on: 4 oct 2026
 *      Author: elvisdt
 *
 *  Pruebas de a un módulo por vez. Se elige con APP_TEST (acá o con -DAPP_TEST=n
 *  en Properties > C/C++ Build > Settings > Preprocessor). Con TEST_NONE
 *  corre la aplicación normal (app_ui).
 *
 *  Todas las pruebas reportan por la consola UART (115200 8N1).
 */

#ifndef APP_INC_APP_TEST_H_
#define APP_INC_APP_TEST_H_

#ifdef __cplusplus
extern "C" {
#endif

#define TEST_NONE       0   /* aplicación completa (app_ui)                   */
#define TEST_DELAY      1   /* LD2 parpadea a 1 Hz + "tick" por UART          */
#define TEST_UART       2   /* consola de la práctica 5 (HELP, LED, STATUS...) */
#define TEST_DEBOUNCE   3   /* cuenta pulsaciones de OK / BACK / SW           */
#define TEST_ENCODER    4   /* imprime los pasos del encoder y el acumulado   */
#define TEST_I2C_SCAN   5   /* lista las direcciones que responden (0x27, 0x76) */
#define TEST_LCD        6   /* "Hola PdM" en el LCD                           */
#define TEST_BME280     7   /* chip ID y mediciones cada 1 s                  */
#define TEST_ALARM      8   /* OK habilita/deshabilita, estado por UART       */

#ifndef APP_TEST
// #define APP_TEST        TEST_DELAY
//#define APP_TEST TEST_I2C_SCAN
#define APP_TEST TEST_LCD

#endif




void appTestInit(void);
void appTestUpdate(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_INC_APP_TEST_H_ */
