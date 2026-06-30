#include "ph.h"

// Variables de calibración activas — se actualizan al cargar de EEPROM
// o al terminar una calibración con buffers.
float ph_offset       = PH_OFFSET_DEFAULT;
float ph_sensibilidad = PH_SENSIBILIDAD_DEFAULT;

// ─────────────────────────────────────────────────────────────
//  ph_inicializar()
//  Carga los valores de calibración desde la EEPROM.
//  Si no hay calibración guardada, usa los valores por defecto.
//  Debe llamarse una vez en setup().
// ─────────────────────────────────────────────────────────────
void ph_inicializar() {
    byte flag;
    EEPROM.get(PH_EEPROM_ADDR_FLAG, flag);

    if (flag == PH_EEPROM_FLAG_VALIDO) {
        // Hay calibración guardada — cargarla
        EEPROM.get(PH_EEPROM_ADDR_OFFSET,       ph_offset);
        EEPROM.get(PH_EEPROM_ADDR_SENSIBILIDAD, ph_sensibilidad);
    } else {
        // No hay calibración guardada — usar valores por defecto de ph.h
        ph_offset       = PH_OFFSET_DEFAULT;
        ph_sensibilidad = PH_SENSIBILIDAD_DEFAULT;
    }
}

// ─────────────────────────────────────────────────────────────
//  ph_guardarCalibracion()
//  Guarda offset y sensibilidad en EEPROM y los activa de inmediato.
//  Se llama al terminar la calibración con buffers.
// ─────────────────────────────────────────────────────────────
void ph_guardarCalibracion(float offset, float sensibilidad) {
    ph_offset       = offset;
    ph_sensibilidad = sensibilidad;

    EEPROM.put(PH_EEPROM_ADDR_OFFSET,       offset);
    EEPROM.put(PH_EEPROM_ADDR_SENSIBILIDAD, sensibilidad);
    EEPROM.put(PH_EEPROM_ADDR_FLAG,         PH_EEPROM_FLAG_VALIDO);
}

// ─────────────────────────────────────────────────────────────
//  ph_tieneCalibacionGuardada()
//  Retorna true si hay calibración válida en EEPROM.
// ─────────────────────────────────────────────────────────────
bool ph_tieneCalibacionGuardada() {
    byte flag;
    EEPROM.get(PH_EEPROM_ADDR_FLAG, flag);
    return (flag == PH_EEPROM_FLAG_VALIDO);
}

// ─────────────────────────────────────────────────────────────
//  ph_imprimirCalibracionActual()
//  Imprime en el Serial Monitor los valores de calibración activos.
// ─────────────────────────────────────────────────────────────
void ph_imprimirCalibracionActual() {
    Serial.println("  Calibracion activa:");
    Serial.print("    PH_OFFSET_PH7   = ");
    Serial.println(ph_offset, 4);
    Serial.print("    PH_SENSIBILIDAD = ");
    Serial.println(ph_sensibilidad, 4);
    if (ph_tieneCalibacionGuardada()) {
        Serial.println("  (guardada en EEPROM)");
    } else {
        Serial.println("  (valores por defecto — sin calibracion guardada)");
    }
}

// ─────────────────────────────────────────────────────────────
//  ph_leerVoltaje()
//  Toma 10 muestras, las ordena y promedia las 6 centrales.
// ─────────────────────────────────────────────────────────────
float ph_leerVoltaje() {
    int buf[10];

    for (int i = 0; i < 10; i++) {
        buf[i] = analogRead(PH_PIN_SENSOR);
        delay(30);
    }

    // Ordenar de menor a mayor (burbuja simple)
    for (int i = 0; i < 9; i++) {
        for (int j = i + 1; j < 10; j++) {
            if (buf[i] > buf[j]) {
                int tmp = buf[i];
                buf[i]  = buf[j];
                buf[j]  = tmp;
            }
        }
    }

    // Promediar las 6 centrales (índices 2..7)
    long suma = 0;
    for (int i = 2; i < 8; i++) suma += buf[i];
    float promedio = (float)suma / 6.0;

    return promedio * PH_VREF / PH_ADC_MAX;
}

// ─────────────────────────────────────────────────────────────
//  ph_calcularPH()
//  Convierte voltaje a pH usando los valores de calibración activos.
//  Fórmula: pH = 7 + (ph_offset − voltaje) / ph_sensibilidad
// ─────────────────────────────────────────────────────────────
float ph_calcularPH(float voltaje) {
    return 7.0 + (ph_offset - voltaje) / ph_sensibilidad;
}
