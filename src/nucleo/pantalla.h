#ifndef NUCLEO_PANTALLA_H
#define NUCLEO_PANTALLA_H

#include <Arduino.h>
#include "conectores.h"
#include "enlace_esp32.h"

// LCD 20x4 por I2C (adaptador PCF8574). SDA pin 20, SCL pin 21 del Mega.
// Textos y reglas: docs/plan_implementacion.md §1.
//
// La fila 0 lleva siempre el nombre del kit. Las demás filas dependen de la
// pantalla, que avanza en orden y no salta pasos:
//
//   Sin enlace : el Mega no recibe nada de la ESP32.
//   Paso 1     : Bluetooth. Se queda hasta que un celular se conecte por BLE.
//   Paso 2     : WiFi. Sin configurar / conectando / falló.
//   Paso 3     : sensores. Solo el nombre de cada uno, sin valores numéricos.
//
// Si el WiFi se cae, la pantalla regresa sola al paso 2.

// Arranca el LCD y autodetecta la dirección I2C (0x27, 0x3F, ...).
// Devuelve false si no hay ningún dispositivo I2C; el resto del programa
// sigue funcionando igual, solo sin display.
bool pantalla_inicializar();

// Pantalla de arranque: SENSEI / Iniciando...
void pantalla_splash();

// Elige la pantalla según el estado y redibuja solo las filas que cambiaron.
// Llamar seguido (cada ciclo).
void pantalla_dibujar(const LecturaConector lecturas[CONECTORES_CANTIDAD],
                      const EstadoEnlace& enlace);

#endif
