#ifndef NUCLEO_ENLACE_ESP32_H
#define NUCLEO_ENLACE_ESP32_H

#include <Arduino.h>
#include "conectores.h"

// Enlace UART entre el Mega y la ESP32-C6 (docs/plan_implementacion.md §4).
//
//   Mega TX1 (18) --[divisor 5V→3.3V]--> ESP32 GPIO17 (RX)
//   Mega RX1 (19) <---------------------- ESP32 GPIO16 (TX)
//   115200 baudios, 8N1
//
// Ida    (Mega → ESP32): una línea JSON por muestreo (docs/protocolo.md §5).
// Vuelta (ESP32 → Mega): líneas de estado con prefijo '#':
//     #NAME SENSEI-85A8   nombre del kit
//     #WIFI CONNECTED     UNCONFIGURED | CONNECTING | CONNECTED | FAILED
//     #BLE 1              cuántos celulares tienen el Bluetooth conectado
//     #IP 172.20.10.6     solo cuando hay WiFi

// Si pasa más de este tiempo sin recibir nada de la ESP32, se considera sin enlace.
// La ESP32 reporta cada 3 s, así que 10 s cubre varias pérdidas seguidas.
const unsigned long ENLACE_TIMEOUT_MS = 10000;

struct EstadoEnlace {
    char          nombreKit[16];   // "SENSEI-85A8" ("" si la ESP32 aún no lo mandó)
    char          wifi[14];        // "UNCONFIGURED" | "CONNECTING" | "CONNECTED" | "FAILED" | ""
    char          ip[16];          // "172.20.10.6" ("" si no hay)
    uint8_t       ble;             // celulares conectados por BLE
    unsigned long ultimoRx;        // millis() de la última línea válida recibida
    bool          visto;           // true si la ESP32 se ha reportado alguna vez
};

void enlace_inicializar();
void enlace_reset(EstadoEnlace* e);

// true si la ESP32 se reportó hace menos de ENLACE_TIMEOUT_MS.
bool enlace_activo(const EstadoEnlace& e);

// Manda el JSON de sensores a la ESP32 (una línea terminada en '\n').
void enlace_enviarJson(const LecturaConector lecturas[CONECTORES_CANTIDAD]);

// Procesa lo que haya llegado de la ESP32 y actualiza *e. No bloquea.
void enlace_procesarEntrada(EstadoEnlace* e);

#endif
