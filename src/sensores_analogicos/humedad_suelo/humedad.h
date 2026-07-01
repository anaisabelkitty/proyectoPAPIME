#ifndef HUMEDAD_H
#define HUMEDAD_H

#include <Arduino.h>
#include <EEPROM.h>

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
//   Suelo SECO   → alta resistencia → voltaje alto → ADC alto
//   Suelo HÚMEDO → baja resistencia → voltaje bajo → ADC bajo
//   La relación exacta depende del tipo de suelo (arena, tierra negra,
//   minerales, fertilizantes, etc.), por eso NO se usan valores fijos
//   de fábrica. Se calibra con 2 puntos reales, igual que el sensor de pH.
//
// Calibración:
//   1. Con la sonda al aire o en suelo completamente seco → anotar ADC (seco)
//   2. Con la sonda en agua o suelo saturado → anotar ADC (húmedo)
//   Con esos dos puntos se aplica interpolación de Lagrange grado 1
//   para convertir cualquier lectura de ADC a porcentaje de humedad.
//
// Los valores de calibración se guardan en EEPROM y se recuperan
// automáticamente al encender el Arduino. Si no hay calibración
// guardada, se usan valores por defecto (deben ajustarse en campo).

const int   HUM_PIN_SENSOR = A4;
const float HUM_VREF       = 5.0;
const float HUM_ADC_MAX    = 1023.0;

// Valores por defecto (usar solo como referencia inicial —
// se recomienda calibrar en el suelo real con la opción del menú)
const int HUM_ADC_SECO_DEFAULT   = 1023; // ADC típico con la sonda al aire
const int HUM_ADC_HUMEDO_DEFAULT = 300;  // ADC típico con la sonda en agua

// Direcciones en EEPROM (después de las usadas por el sensor de pH: 0-8)
const int HUM_EEPROM_ADDR_SECO   = 16; // bytes 16–19 (int)
const int HUM_EEPROM_ADDR_HUMEDO = 20; // bytes 20–23 (int)
const int HUM_EEPROM_ADDR_FLAG   = 24; // byte 24: 0xCD indica calibración válida
const byte HUM_EEPROM_FLAG_VALIDO = 0xCD;

// Variables de calibración activas (modificables en tiempo de ejecución)
extern int hum_adc_seco;
extern int hum_adc_humedo;

// Funciones de medición
int   hum_leerADC();
float hum_calcularHumedad(int lectura_adc);

// Funciones de calibración persistente
void hum_inicializar();                          // Carga calibración de EEPROM
void hum_guardarCalibracion(int seco, int humedo); // Guarda en EEPROM
bool hum_tieneCalibacionGuardada();

#endif
