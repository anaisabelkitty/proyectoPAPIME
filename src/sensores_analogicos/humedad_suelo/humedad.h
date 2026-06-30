#ifndef HUMEDAD_H
#define HUMEDAD_H

#include <Arduino.h>

// --- Constantes del sensor de humedad de suelo OKY3442 ---
// Chip:    LM393 (comparador con salida analógica y digital)
// Módulo:  3.2 cm × 1.4 cm | Sonda: 6 cm × 2 cm | Cable: 21 cm
//
// Conexiones:
//   VCC del módulo → 5V del Arduino Mega
//   GND del módulo → GND del Arduino Mega
//   A0  del módulo → A4 del Arduino Mega  (señal analógica)
//   D0  del módulo → no se usa en este proyecto
//
// Funcionamiento de la salida analógica (A0):
//   Suelo SECO   → alta resistencia → voltaje alto → ADC alto (~1023)
//   Suelo HÚMEDO → baja resistencia → voltaje bajo → ADC bajo (~0)
//   La relación es INVERSA: más humedad = ADC más bajo.
//
// Conversión a porcentaje de humedad:
//   Se usa interpolación de Lagrange grado 1 con dos puntos:
//     (ADC=1023, humedad=0%)  — suelo completamente seco
//     (ADC=0,    humedad=100%) — suelo completamente húmedo
//   Resultado: humedad(%) = (1 - ADC/1023) × 100 = (1023 - ADC) × 100 / 1023
//
// Potenciómetro del módulo:
//   Ajusta el umbral de la salida digital D0 (no afecta la salida analógica A0).
//
// Referencia:
//   AG Electrónica. (2024). OKY3442 — Sensor de humedad del suelo.
//   https://agelectronica.com/detalle?busca=OKY3442

const int   HUM_PIN_SENSOR = A4;    // Pin analógico del Arduino Mega
const float HUM_VREF       = 5.0;   // Voltaje de referencia del ADC
const float HUM_ADC_MAX    = 1023.0; // Máximo valor del ADC (10 bits → 0..1023)

// Puntos de Lagrange para la conversión:
//   x0=1023 (seco)  → y0=0%
//   x1=0    (húmedo) → y1=100%
const float HUM_ADC_SECO   = 1023.0; // ADC cuando el suelo está completamente seco
const float HUM_ADC_HUMEDO = 0.0;    // ADC cuando el suelo está completamente húmedo
const float HUM_PCT_SECO   = 0.0;    // Porcentaje correspondiente al suelo seco
const float HUM_PCT_HUMEDO = 100.0;  // Porcentaje correspondiente al suelo húmedo

// Funciones
int   hum_leerADC();
float hum_calcularHumedad(int lectura_adc);

#endif
