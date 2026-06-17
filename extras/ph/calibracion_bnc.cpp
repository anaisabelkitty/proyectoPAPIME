// REFERENCIA — No se compila, solo es para consulta
// Código de calibración BNC completada: POT2 ajustado a ~2.50V ✔
//
// Procedimiento realizado:
//   1. Puente en el conector BNC
//   2. VCC→5V | GND→GND | Po→A0
//   3. Se ajustó POT2 hasta obtener ~2.50V estable

#include <Arduino.h>

const int   PH_PIN   = A0;
const float VREF     = 5.0;
const int   ADC_R    = 1024;
const int   MUESTRAS = 20;

float leerVoltaje() {
    long suma = 0;
    for (int i = 0; i < MUESTRAS; i++) {
        suma += analogRead(PH_PIN);
        delay(10);
    }
    float promedio = (float)suma / MUESTRAS;
    return promedio * VREF / ADC_R;
}

void setup() {
    Serial.begin(9600);
    Serial.println("  CALIBRACION BNC - Modulo pH");
    Serial.println("  Ajusta POT2 hasta obtener ~2.5V");
}

void loop() {
    float voltaje = leerVoltaje();
    Serial.print("Voltaje Po: ");
    Serial.print(voltaje, 3);
    Serial.print(" V  →  ");

    if (voltaje < 2.3)                     Serial.println("⬆ Sube POT2");
    else if (voltaje >= 2.3 && voltaje < 2.5) Serial.println("⬆ Sube un poco");
    else if (voltaje >= 2.5 && voltaje <= 3.0) Serial.println("✔ CORRECTO");
    else if (voltaje > 3.0 && voltaje <= 3.2) Serial.println("⬇ Baja un poco");
    else                                   Serial.println("⬇ Baja POT2");

    delay(1000);
}
