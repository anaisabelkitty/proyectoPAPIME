#ifndef TEMPERATURA_H
#define TEMPERATURA_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// --- Constantes del sensor DS18B20 (sumergible, acero inoxidable) ---
// Conexiones:
//   Cable rojo  (VCC)  → 5V del Arduino Mega
//   Cable negro (GND)  → GND del Arduino Mega
//   Cable amarillo (DQ) → pin digital 7 del Arduino Mega
//
// Resistencia de pull-up:
//   4.7 kΩ entre el pin DQ (cable amarillo) y 5V.
//   Obligatoria para que el protocolo 1-Wire funcione correctamente.
//
// Protocolo:
//   1-Wire. Un solo pin de datos para comunicación bidireccional.
//   Cada sensor tiene un identificador único de 64 bits grabado en fábrica.
//   Se pueden conectar varios sensores en paralelo en el mismo pin (bus 1-Wire).
//
// Resolución configurada:
//   12 bits → resolución de 0.0625 °C (máxima precisión del sensor).
//   Tiempo de conversión: ~750 ms por lectura.
//
// Librerías necesarias (agregar en platformio.ini):
//   lib_deps =
//       paulstoffregen/OneWire @ ^2.3.8
//       milesburton/DallasTemperature @ ^3.11.0

const int TEMP_PIN_SENSOR = 7;   // Pin digital donde está conectado el cable DQ
const int TEMP_RESOLUCION = 12;  // Resolución en bits (9, 10, 11 o 12)

// Valor centinela que devuelve DallasTemperature cuando la lectura falla
// (sensor desconectado, error en el bus, etc.)
const float TEMP_ERROR = DEVICE_DISCONNECTED_C;  // -127.00 °C

// Funciones
void   temp_inicializar();
float  temp_leerCelsius();

#endif
