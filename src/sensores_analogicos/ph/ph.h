#ifndef PH_H
#define PH_H

#include <Arduino.h>

// --- Constantes del sensor PH-4502C ---
// Conexiones:
//   VCC del módulo → 5V del Arduino Mega
//   GND del módulo → GND del Arduino Mega
//   Po  del módulo → A2 del Arduino Mega
//   Electrodo E201C-BNC → conector BNC del módulo
//
// Potenciómetros del módulo:
//   POT2 (MÁS CERCANO al BNC) → ajuste de offset analógico (Po)
//                               Verificado físicamente en nuestro módulo.
//                               Algunos tutoriales lo llaman POT1.
//   POT1 (MÁS LEJANO al BNC)  → límite de la salida digital (Do)
//                               No afecta la lectura de pH.
//
// Calibración:
//   Estos valores se obtienen ejecutando extras/ph/calibracion_buffer.cpp.
//   Después de calibrar, reemplaza PH_OFFSET_PH7 y PH_SENSIBILIDAD con
//   los valores que imprime ese sketch en el Serial Monitor.

const int   PH_PIN_SENSOR   = A2;
const float PH_VREF         = 5.0;
const float PH_ADC_MAX      = 1024.0;
const float PH_OFFSET_PH7   = 2.5018; // Voltaje medido en buffer pH 7 (calibrado)
const float PH_SENSIBILIDAD = 0.1771; // V/pH obtenido con Lagrange (buffer pH 7 y pH 4)

// Funciones
float ph_leerVoltaje();
float ph_calcularPH(float voltaje);

#endif
