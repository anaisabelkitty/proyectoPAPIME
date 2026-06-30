#include "humedad.h"

// Lee el valor crudo del ADC en el pin A0 del módulo OKY3442.
// Toma 10 muestras, las ordena y promedia las 6 centrales
// para eliminar ruido y picos espurios (mismo método que el sensor de pH).
int hum_leerADC() {
    int buf[10];

    // Paso 1: tomar 10 lecturas con 10 ms de separación
    for (int i = 0; i < 10; i++) {
        buf[i] = analogRead(HUM_PIN_SENSOR);
        delay(10);
    }

    // Paso 2: ordenar de menor a mayor (burbuja simple)
    for (int i = 0; i < 9; i++) {
        for (int j = i + 1; j < 10; j++) {
            if (buf[i] > buf[j]) {
                int tmp = buf[i];
                buf[i]  = buf[j];
                buf[j]  = tmp;
            }
        }
    }

    // Paso 3: promediar las 6 del centro (índices 2..7)
    long suma = 0;
    for (int i = 2; i < 8; i++) suma += buf[i];
    return (int)(suma / 6);
}

// Convierte la lectura cruda del ADC a porcentaje de humedad del suelo
// usando interpolación de Lagrange grado 1.
//
// Dos puntos calibrados:
//   (HUM_ADC_SECO=1023,   HUM_PCT_SECO=0)
//   (HUM_ADC_HUMEDO=0,  HUM_PCT_HUMEDO=100)
//
// Fórmula de Lagrange grado 1 (equivale a una línea recta):
//   P(x) = y0 * (x - x1)/(x0 - x1) + y1 * (x - x0)/(x1 - x0)
//
// Sustituyendo (x0=1023, y0=0) y (x1=0, y1=100):
//   P(x) = 0 * (x - 0)/(1023 - 0) + 100 * (x - 1023)/(0 - 1023)
//         = 100 * (x - 1023) / (-1023)
//         = (1023 - x) * 100 / 1023
//
// El resultado se limita al rango 0–100 para evitar valores negativos
// o mayores a 100 por lecturas fuera del rango de calibración.
float hum_calcularHumedad(int lectura_adc) {
    float humedad = (HUM_ADC_SECO - (float)lectura_adc) * HUM_PCT_HUMEDO / HUM_ADC_SECO;

    // Limitar al rango 0–100%
    if (humedad < 0.0f)   humedad = 0.0f;
    if (humedad > 100.0f) humedad = 100.0f;

    return humedad;
}
