#include "ph.h"

// Lee el voltaje en el pin Po del módulo PH-4502C
float ph_leerVoltaje() {
    int lectura = analogRead(PH_PIN_SENSOR);

    // Paso 1: ADC a voltaje
    // Fórmula: Vsensor = (lectura × Vref) / 1024
    return (lectura * PH_VREF) / PH_ADC_MAX;
}

// Convierte voltaje a pH usando interpolación de Lagrange grado 1
// Puntos de calibración:
//   (x0, y0) = (2.5,  7) → buffer pH 7, voltaje 2.5V
//   (x1, y1) = (3.04, 4) → buffer pH 4, voltaje 3.04V
// Fórmula resultante: pH = 7 + (2.5 - Vout) / 0.18
float ph_calcularPH(float voltaje) {
    // Paso 2: diferencia respecto al offset de pH 7
    float diferencia = PH_OFFSET_PH7 - voltaje;

    // Paso 3: dividir entre sensibilidad y sumar a 7
    return 7.0 + (diferencia / PH_SENSIBILIDAD);
}
