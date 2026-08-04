#ifndef ULTRASONICO_H
#define ULTRASONICO_H

#include <Arduino.h>

// --- Constantes del sensor ultrasónico HC-SR04 ---
// Conexiones:
//   Vcc  → 5V del Arduino Mega
//   Trig → pin digital 8 del Arduino Mega
//   Echo → pin digital 9 del Arduino Mega
//   GND  → GND del Arduino Mega
//
// Principio de funcionamiento:
//   Se manda un pulso HIGH de al menos 10 us por Trig. El sensor responde
//   con 8 pulsos de ultrasonido a 40 kHz y levanta Echo en HIGH durante
//   el tiempo que tarda el eco en volver. La distancia se calcula con
//   la velocidad del sonido (340 m/s):
//
//     distancia_cm = (tiempo_echo_us × 0.034) / 2 = tiempo_echo_us / 58
//
// Rango del sensor (según datasheet):
//   2 cm a 400 cm, ángulo de medición de 15°.

const int ULTRA_PIN_TRIG = 8;   // Pin digital de salida (dispara el pulso)
const int ULTRA_PIN_ECHO = 9;   // Pin digital de entrada (mide el eco)

const unsigned long ULTRA_PULSO_TRIG_US = 10;      // Duración del pulso de disparo
const unsigned long ULTRA_TIMEOUT_US    = 30000;   // Máximo tiempo de espera del eco (~30 ms, cubre 400 cm con margen)

const float ULTRA_DIST_MIN_CM = 2.0;    // Rango mínimo confiable del sensor
const float ULTRA_DIST_MAX_CM = 400.0;  // Rango máximo confiable del sensor

// Valor centinela cuando no hay eco dentro del timeout (nada en rango, o sensor desconectado)
const float ULTRA_ERROR = -1.0;

// Funciones
void  ultra_inicializar();
float ultra_leerDistanciaCM();

#endif
