#include "humedad.h"

// Variables de calibración activas
int hum_adc_seco   = HUM_ADC_SECO_DEFAULT;
int hum_adc_humedo = HUM_ADC_HUMEDO_DEFAULT;

// ─────────────────────────────────────────────────────────────
//  hum_inicializar()
//  Carga los valores de calibración desde la EEPROM.
//  Si no hay calibración guardada, usa los valores por defecto.
// ─────────────────────────────────────────────────────────────
void hum_inicializar() {
    byte flag;
    EEPROM.get(HUM_EEPROM_ADDR_FLAG, flag);

    if (flag == HUM_EEPROM_FLAG_VALIDO) {
        EEPROM.get(HUM_EEPROM_ADDR_SECO,   hum_adc_seco);
        EEPROM.get(HUM_EEPROM_ADDR_HUMEDO, hum_adc_humedo);
    } else {
        hum_adc_seco   = HUM_ADC_SECO_DEFAULT;
        hum_adc_humedo = HUM_ADC_HUMEDO_DEFAULT;
    }
}

// ─────────────────────────────────────────────────────────────
//  hum_guardarCalibracion()
//  Guarda los dos puntos de calibración en EEPROM y los activa.
// ─────────────────────────────────────────────────────────────
void hum_guardarCalibracion(int seco, int humedo) {
    hum_adc_seco   = seco;
    hum_adc_humedo = humedo;

    EEPROM.put(HUM_EEPROM_ADDR_SECO,   seco);
    EEPROM.put(HUM_EEPROM_ADDR_HUMEDO, humedo);
    EEPROM.put(HUM_EEPROM_ADDR_FLAG,   HUM_EEPROM_FLAG_VALIDO);
}

bool hum_tieneCalibacionGuardada() {
    byte flag;
    EEPROM.get(HUM_EEPROM_ADDR_FLAG, flag);
    return (flag == HUM_EEPROM_FLAG_VALIDO);
}

// Lee el valor crudo del ADC en el pin A0 del módulo OKY3442.
// Toma 10 muestras, las ordena y promedia las 6 centrales
// para eliminar ruido y picos espurios (mismo método que el sensor de pH).
int hum_leerADC() {
    int buf[10];

    for (int i = 0; i < 10; i++) {
        buf[i] = analogRead(HUM_PIN_SENSOR);
        delay(10);
    }

    for (int i = 0; i < 9; i++) {
        for (int j = i + 1; j < 10; j++) {
            if (buf[i] > buf[j]) {
                int tmp = buf[i];
                buf[i]  = buf[j];
                buf[j]  = tmp;
            }
        }
    }

    long suma = 0;
    for (int i = 2; i < 8; i++) suma += buf[i];
    return (int)(suma / 6);
}

// ─────────────────────────────────────────────────────────────
//  hum_calcularHumedad()
//  Convierte la lectura cruda del ADC a porcentaje de humedad
//  usando interpolación de Lagrange grado 1 con los dos puntos
//  de calibración activos (hum_adc_seco = 0%, hum_adc_humedo = 100%).
//
//  Fórmula de Lagrange grado 1 para dos puntos (x0,y0) y (x1,y1):
//    P(x) = y0 * (x - x1)/(x0 - x1) + y1 * (x - x0)/(x1 - x0)
//
//  Sustituyendo (x0=hum_adc_seco, y0=0) y (x1=hum_adc_humedo, y1=100):
//    P(x) = 100 * (x - hum_adc_seco) / (hum_adc_humedo - hum_adc_seco)
// ─────────────────────────────────────────────────────────────
float hum_calcularHumedad(int lectura_adc) {
    float humedad = 100.0f * (float)(lectura_adc - hum_adc_seco)
                            / (float)(hum_adc_humedo - hum_adc_seco);

    if (humedad < 0.0f)   humedad = 0.0f;
    if (humedad > 100.0f) humedad = 100.0f;

    return humedad;
}
