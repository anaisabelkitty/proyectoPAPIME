#ifndef PH_H
#define PH_H

#include <Arduino.h>
#include <EEPROM.h>

// --- Constantes fijas del sensor PH-4502C ---
// Conexiones:
//   VCC del módulo → 5V del Arduino Mega
//   GND del módulo → GND del Arduino Mega
//   Po  del módulo → A2 del Arduino Mega
//   Electrodo E201C-BNC → conector BNC del módulo
//
// Potenciómetros del módulo:
//   POT2 (MÁS CERCANO al BNC) → ajuste de offset analógico (Po)
//   POT1 (MÁS LEJANO al BNC)  → límite de la salida digital (Do)
//
// Calibración:
//   Los valores de offset y sensibilidad se pueden actualizar en tiempo
//   de ejecución usando ph_guardarCalibracion(). Se guardan en EEPROM
//   y se recuperan automáticamente al encender el Arduino.
//   Valores por defecto (usados si nunca se ha calibrado):

const int   PH_PIN_SENSOR        = A2;
const float PH_VREF              = 5.0;
const float PH_ADC_MAX           = 1024.0;
const float PH_OFFSET_DEFAULT    = 2.5018f; // Voltaje buffer pH 7 (fábrica)
const float PH_SENSIBILIDAD_DEFAULT = 0.1771f; // V/pH (fábrica)

// Direcciones en EEPROM donde se guardan los valores de calibración
// El ATmega2560 tiene 4096 bytes de EEPROM.
// Se usan 8 bytes: 4 para el offset (float) y 4 para la sensibilidad (float).
const int PH_EEPROM_ADDR_OFFSET      = 0;  // bytes 0–3
const int PH_EEPROM_ADDR_SENSIBILIDAD = 4;  // bytes 4–7
const int PH_EEPROM_ADDR_FLAG        = 8;  // byte 8: 0xAB indica calibración válida
const byte PH_EEPROM_FLAG_VALIDO     = 0xAB;

// Variables de calibración activas (modificables en tiempo de ejecución)
extern float ph_offset;
extern float ph_sensibilidad;

// Funciones de medición
float ph_leerVoltaje();
float ph_calcularPH(float voltaje);

// Funciones de calibración persistente
void  ph_inicializar();              // Carga calibración de EEPROM (llamar en setup)
void  ph_guardarCalibracion(float offset, float sensibilidad); // Guarda en EEPROM
bool  ph_tieneCalibacionGuardada();  // Retorna true si hay calibración en EEPROM
void  ph_imprimirCalibracionActual(); // Imprime offset y sensibilidad actuales

#endif
