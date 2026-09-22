#ifndef NUCLEO_ENLACE_ESP32_H
#define NUCLEO_ENLACE_ESP32_H

#include <Arduino.h>
#include "conectores.h"

// Enlace UART entre el Mega y el ESP32-C6.
//
//   Mega TX1 (pin 18) --[divisor 5V->3.3V]--> ESP32-C6 GPIO18 (RX)
//   Mega RX1 (pin 19) ------------------------> ESP32-C6 GPIO19 (TX)
//   115200 baudios, 8N1  (docs/conexiones.md)
//
// Ida  (Mega -> ESP32): el JSON de sensores de docs/protocolo.md §5, una
//                       línea por muestreo. El ESP32 lo reenvía tal cual.
// Vuelta (ESP32 -> Mega): líneas de estado con prefijo '#':
//                       "#NAME SENSEI-A1B2", "#WIFI CONNECTED", "#IP 192.168.1.5",
//                       "#BLE 1" (cuántos celulares tienen el BLE conectado).

struct EstadoEnlace {
    char nombreKit[16];   // "SENSEI-A1B2"   ("" si el ESP32 aún no lo mandó)
    char wifi[14];        // "UNCONFIGURED" | "CONNECTING" | "CONNECTED" | "FAILED" | ""
    char ip[16];          // "192.168.1.5"   ("" si no hay)
    char ble[4];          // "0", "1", ...   ("" si el ESP32 aún no lo mandó)
};

void enlace_inicializar();
void enlace_reset(EstadoEnlace* e);

// Manda el JSON de sensores al ESP32 (una línea terminada en '\n').
void enlace_enviarJson(const LecturaConector lecturas[NUCLEO_NUM_CONECTORES]);

// Procesa lo que haya llegado del ESP32 y actualiza *e. No bloquea.
// Devuelve true si algún campo cambió.
bool enlace_procesarEntrada(EstadoEnlace* e);

#endif
