// Programa de prueba — Kit Sensei
// Selecciona el sensor desde el Serial Monitor escribiendo su número.
//
// Sensores disponibles:
//   1 → pH PH-4502C          (pin A2)
//   2 → Temperatura DS18B20  (pin digital 7)

#include <Arduino.h>
#include "sensores_analogicos/ph/ph.h"
#include "sensores_digitales/temperatura_ds18b20/temperatura.h"

// Sensor activo: 0 = ninguno, 1 = pH, 2 = temperatura
int sensorActivo = 0;

void imprimirMenu() {
    Serial.println();
    Serial.println("=========================================");
    Serial.println("  Kit Sensei — Selecciona un sensor");
    Serial.println("=========================================");
    Serial.println("  1  →  pH PH-4502C");
    Serial.println("  2  →  Temperatura DS18B20");
    Serial.println("  0  →  Detener lectura");
    Serial.println("-----------------------------------------");
    Serial.println("Escribe el numero y presiona ENTER:");
}

void setup() {
    Serial.begin(9600);
    pinMode(PH_PIN_SENSOR, INPUT);
    temp_inicializar();
    imprimirMenu();
}

void loop() {
    // Leer selección del usuario
    if (Serial.available() > 0) {
        int opcion = Serial.parseInt();
        while (Serial.available() > 0) Serial.read(); // vaciar buffer

        if (opcion == 1) {
            sensorActivo = 1;
            Serial.println();
            Serial.println(">> Leyendo pH PH-4502C (escribe 0 para detener)");
        } else if (opcion == 2) {
            sensorActivo = 2;
            Serial.println();
            Serial.println(">> Leyendo temperatura DS18B20 (escribe 0 para detener)");
        } else if (opcion == 0) {
            sensorActivo = 0;
            imprimirMenu();
        } else {
            Serial.println("Opcion no valida.");
            imprimirMenu();
        }
    }

    // Leer el sensor activo
    if (sensorActivo == 1) {
        float voltaje = ph_leerVoltaje();
        float pH      = ph_calcularPH(voltaje);
        Serial.print("Voltaje: ");
        Serial.print(voltaje, 3);
        Serial.print(" V  |  pH: ");
        Serial.println(pH, 2);
        delay(1000);

    } else if (sensorActivo == 2) {
        float temperatura = temp_leerCelsius();
        if (temperatura == TEMP_ERROR) {
            Serial.println("ERROR: sensor DS18B20 desconectado.");
        } else {
            Serial.print("Temperatura: ");
            Serial.print(temperatura, 2);
            Serial.println(" °C");
        }
        delay(1000);
    }
}
