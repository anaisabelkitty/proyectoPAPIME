#include "ph.h"

// Lee el voltaje en el pin Po del módulo PH-4502C
// Toma 10 muestras, las ordena (burbuja) y promedia las 6 centrales
// para eliminar ruido y picos espurios (igual al sketch del datasheet).
float ph_leerVoltaje() {
    int buf[10];

    // Paso 1: tomar 10 lecturas con 30 ms de separación
    for (int i = 0; i < 10; i++) {
        buf[i] = analogRead(PH_PIN_SENSOR);
        delay(30);
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

    // Paso 3: promediar solo las 6 del centro (índices 2..7),
    // descartando los 2 más bajos y los 2 más altos
    long suma = 0;
    for (int i = 2; i < 8; i++) {
        suma += buf[i];
    }
    float promedio = (float)suma / 6.0;

    // Paso 4: ADC a voltaje
    // Fórmula: Vsensor = (promedio × Vref) / 1024
    return promedio * PH_VREF / PH_ADC_MAX;
}

// Convierte voltaje a pH usando interpolación de Lagrange grado 1.
// Fórmula: pH = 7 + (PH_OFFSET_PH7 − Vout) / PH_SENSIBILIDAD
//
// Los valores de PH_OFFSET_PH7 y PH_SENSIBILIDAD se obtienen
// ejecutando extras/ph/calibracion_buffer.cpp y copiando el resultado
// a ph.h. No modificar esta función; solo actualizar las constantes.
float ph_calcularPH(float voltaje) {
    // Diferencia respecto al voltaje de pH 7 (offset calibrado)
    float diferencia = PH_OFFSET_PH7 - voltaje;

    // Dividir entre la sensibilidad y centrar en pH 7
    return 7.0 + (diferencia / PH_SENSIBILIDAD);
}
