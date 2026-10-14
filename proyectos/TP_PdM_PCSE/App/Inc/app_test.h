/**
 * @file    app_test.h
 * @brief   Pruebas de un módulo por vez.
 * @author  Elvis de la Torre
 * @date    4 oct 2026
 *
 * Cada módulo se fue probando solo en la placa antes de integrarlo. La
 * prueba se elige con APP_TEST; con TEST_NONE corre la estación completa.
 * Todas informan por la consola (115200 8N1).
 */

#ifndef APP_INC_APP_TEST_H_
#define APP_INC_APP_TEST_H_

#ifdef __cplusplus
extern "C" {
#endif

#define TEST_NONE       0   /**< Estación completa (app_ui). */
#define TEST_DELAY      1   /**< LD2 parpadea a 1 Hz y avisa por consola. */
#define TEST_UART       2   /**< Consola de la práctica 5: LED y STATUS. */
#define TEST_DEBOUNCE   3   /**< Cuenta las pulsaciones de cada botón. */
#define TEST_ENCODER    4   /**< Pasos del encoder y su pulsador. */
#define TEST_I2C_SCAN   5   /**< Lista las direcciones que responden en el bus. */
#define TEST_LCD        6   /**< Texto y un contador de segundos en el LCD. */
#define TEST_BME280     7   /**< ID del sensor y una medición por segundo. */
#define TEST_ALARM      8   /**< Alarma con un dato simulado; OK la habilita. */

#ifndef APP_TEST
#define APP_TEST        TEST_NONE   /**< Prueba elegida. */
#endif

/** @brief Inicializa la prueba elegida. */
void appTestInit(void);

/** @brief Una vuelta de la prueba elegida. No bloquea. */
void appTestUpdate(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_INC_APP_TEST_H_ */
