#include <Arduino.h>
#include "sensores_analogicos/ph/ph.h"

void setup() {
    Serial.begin(9600);
    pinMode(PH_PIN_SENSOR, INPUT);
    Serial.println("=========================================");
    Serial.println("  Sensor de pH PH-4502C");
    Serial.println("=========================================\n");
}

void loop() {
    float voltaje = ph_leerVoltaje();
    float pH      = ph_calcularPH(voltaje);

    Serial.print("Voltaje: ");
    Serial.print(voltaje, 3);
    Serial.print(" V  |  pH: ");
    Serial.println(pH, 2);

    delay(1000);
}
