#ifndef PH_H
#define PH_H

#include <Arduino.h>

// --- Constantes del sensor PH-4502C ---
// Conexiones:
//   VCC del módulo → 5V del Arduino Mega
//   GND del módulo → GND del Arduino Mega
//   Po  del módulo → A2 del Arduino Mega
//   Electrodo E201 → conector BNC del módulo

const int   PH_PIN_SENSOR   = A2;
const float PH_VREF         = 5.0;
const float PH_ADC_MAX      = 1024.0;
const float PH_OFFSET_PH7   = 2.5187; // Voltaje medido en buffer pH 7 (calibrado)
const float PH_SENSIBILIDAD = 0.1752; // V por unidad de pH (calculado con Lagrange pH7 y pH4)

// Funciones
float ph_leerVoltaje();
float ph_calcularPH(float voltaje);

#endif
