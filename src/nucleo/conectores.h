#ifndef NUCLEO_CONECTORES_H
#define NUCLEO_CONECTORES_H

#include <Arduino.h>

// Lectura de los 4 conectores RJ45 del kit.
//
// El Mega lee el código de identificación de cada conector (4 bits con pull-up
// interno; ver docs/diseno_electronico.md) y, según el código, lee el sensor con
// el método que le corresponde. El resultado queda en un arreglo de
// LecturaConector que consumen el enlace con la ESP32 (JSON) y el LCD.
//
// Sensores reconocidos hoy: pH (2), humedad (4), temperatura (9), ultrasónico (10).
// Un conector vacío (0) o con un código sin programar se reporta como no conectado.

const uint8_t CONECTORES_CANTIDAD = 4;

struct LecturaConector {
    uint8_t     puerto;       // 1..4
    uint8_t     codigo;       // código de identificación leído (0..15)
    bool        conectado;    // true si el código corresponde a un sensor programado
    bool        valorValido;  // true si la lectura del sensor salió bien
    float       valor;        // magnitud medida (solo válida si valorValido)
    uint8_t     decimales;    // decimales del valor para el JSON (docs/proyecto.md)
    const char* id;           // identificador para el JSON: "ph", "humedad", ...
    const char* unidad;       // unidad para el JSON: "pH", "%", "°C", "cm"
    const char* abrev;        // etiqueta corta para el LCD: "PH", "HUM", "TEMP", "DIST"
};

// Configura los pines de identificación y carga la calibración guardada en EEPROM
// (pH y humedad). Llamar una vez en setup().
void conectores_inicializar();

// Relee los 4 conectores y llena el arreglo. No bloquea: el DS18B20 se lee con
// conversión asíncrona, así que su valor se actualiza cada ~1 s y en el primer
// escaneo aún no es válido.
void conectores_escanear(LecturaConector lecturas[CONECTORES_CANTIDAD]);

#endif
