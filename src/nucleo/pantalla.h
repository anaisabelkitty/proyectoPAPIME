#ifndef NUCLEO_PANTALLA_H
#define NUCLEO_PANTALLA_H

#include <Arduino.h>
#include "conectores.h"
#include "enlace_esp32.h"

// LCD 20x4 por I2C (adaptador PCF8574). SDA pin 20, SCL pin 21 del Mega.
//
// Sin botones: 3 pantallas EN SECUENCIA, no un carrusel por tiempo. Cada
// una se queda fija hasta que el kit realmente avanza a ese paso; si algo
// retrocede (se cae el WiFi) la pantalla regresa sola a la que corresponde:
//
//   1. Bluetooth: se queda aquí hasta que el ESP32 reporte su nombre real
//      (SENSEI-XXXX, el identificador por MAC). Sin eso no hay enlace con
//      el ESP32 y no tiene caso mostrar WiFi ni sensores.
//   2. WiFi: se queda aquí hasta que el estado sea CONNECTED.
//   3. Sensores: SOLO el estado de los 4 conectores (qué sensor hay o si
//      están disponibles), sin las lecturas numéricas — esas van por
//      WebSocket/web, no en el LCD.
//
// La fila 0 (nombre del kit) queda fija en las tres pantallas.

// Arranca el LCD. Autodetecta la dirección I2C (0x27 / 0x3F / ...).
// Devuelve false si no encuentra ningún dispositivo I2C: el resto del
// programa sigue funcionando igual, solo sin display.
bool pantalla_inicializar();

// Pantalla de arranque.
void pantalla_splash();

// Redibuja el LCD: avanza de página sola cada ~3 s y solo reescribe las
// filas que cambiaron (sin parpadeo). Llamar seguido (cada ciclo).
void pantalla_dibujar(const LecturaConector lecturas[NUCLEO_NUM_CONECTORES],
                      const EstadoEnlace& enlace);

#endif
