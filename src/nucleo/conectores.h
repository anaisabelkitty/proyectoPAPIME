#ifndef NUCLEO_CONECTORES_H
#define NUCLEO_CONECTORES_H

#include <Arduino.h>

// Lectura resuelta de los 4 conectores RJ45 del kit.
//
// El Mega detecta qué sensor hay en cada conector por su código de
// identificación (4 bits con pull-up interno) y, según el código, lee la
// señal con el método que corresponde. El resultado se entrega en un
// arreglo de LecturaConector que consumen el LCD y el enlace con el ESP32.

const uint8_t NUCLEO_NUM_CONECTORES = 4;

struct LecturaConector {
    uint8_t     puerto;       // 1..4
    int         codigo;       // código de identificación leído (0..15)
    bool        conectado;    // true si el código corresponde a un sensor conocido
    bool        valorValido;  // true si la lectura del sensor salió bien
    float       valor;        // magnitud medida
    uint8_t     decimales;    // decimales del valor para el JSON (protocolo.md)
    const char* id;           // identificador para el JSON ("ph", "temperatura", ...)
    const char* unidad;       // unidad para el JSON ("pH", "\xC2\xB0""C", "%", "cm")
    const char* abrev;        // etiqueta corta para el LCD ("pH", "Tmp", "Hum", "Dst")
};

// Configura los pines de identificación de los 4 conectores. Llamar en setup().
void conectores_inicializar();

// Relee los 4 conectores y llena el arreglo. Puede tardar ~750 ms si hay un
// DS18B20 conectado (tiempo de conversión del sensor).
void conectores_escanear(LecturaConector lecturas[NUCLEO_NUM_CONECTORES]);

// Vuelca el estado de los 4 conectores por Serial (USB) — lo usa el menú.
void conectores_imprimirSerial();

#endif
