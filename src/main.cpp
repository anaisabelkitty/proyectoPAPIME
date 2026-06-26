// Programa de prueba — Kit Sensei
// Selecciona el modo desde el Serial Monitor escribiendo su número.
//
// Opciones:
//   1 → pH PH-4502C solo          (Práctica 01)
//   2 → Temperatura DS18B20 solo  (Práctica 02)
//   3 → pH + Temperatura juntos   (Práctica 03)
//   0 → Volver al menú

#include <Arduino.h>
#include "sensores_analogicos/ph/ph.h"
#include "sensores_digitales/temperatura_ds18b20/temperatura.h"

int modoActivo = 0;

void imprimirMenu() {
    Serial.println();
    Serial.println("=========================================");
    Serial.println("  Kit Sensei — Selecciona un modo");
    Serial.println("=========================================");
    Serial.println("  1  →  pH solo          (Practica 01)");
    Serial.println("  2  →  Temperatura sola (Practica 02)");
    Serial.println("  3  →  pH + Temperatura (Practica 03)");
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
        while (Serial.available() > 0) Serial.read();

        if (opcion == 1) {
            modoActivo = 1;
            Serial.println();
            Serial.println(">> pH solo — PH-4502C (0 para volver al menu)");
        } else if (opcion == 2) {
            modoActivo = 2;
            Serial.println();
            Serial.println(">> Temperatura sola — DS18B20 (0 para volver al menu)");
        } else if (opcion == 3) {
            modoActivo = 3;
            Serial.println();
            Serial.println(">> pH + Temperatura simultaneos (0 para volver al menu)");
            Serial.println("pH        | Temperatura");
            Serial.println("----------|-----------");
        } else if (opcion == 0) {
            modoActivo = 0;
            imprimirMenu();
        } else {
            Serial.println("Opcion no valida.");
            imprimirMenu();
        }
    }

    // --- Modo 1: solo pH ---
    if (modoActivo == 1) {
        float voltaje = ph_leerVoltaje();
        float pH      = ph_calcularPH(voltaje);
        Serial.print("Voltaje: ");
        Serial.print(voltaje, 3);
        Serial.print(" V  |  pH: ");
        Serial.println(pH, 2);
        delay(1000);

    // --- Modo 2: solo temperatura ---
    } else if (modoActivo == 2) {
        float temperatura = temp_leerCelsius();
        if (temperatura == TEMP_ERROR) {
            Serial.println("ERROR: sensor DS18B20 desconectado.");
        } else {
            Serial.print("Temperatura: ");
            Serial.print(temperatura, 2);
            Serial.println(" °C");
        }
        delay(1000);

    // --- Modo 3: pH y temperatura simultáneos ---
    } else if (modoActivo == 3) {
        // Temperatura primero (tarda ~750 ms internamente)
        float temperatura = temp_leerCelsius();
        // Luego pH (también toma su tiempo con el promedio de 10 muestras)
        float voltaje = ph_leerVoltaje();
        float pH      = ph_calcularPH(voltaje);

        if (temperatura == TEMP_ERROR) {
            Serial.print(pH, 2);
            Serial.println("       | ERROR DS18B20");
        } else {
            Serial.print(pH, 2);
            Serial.print("      | ");
            Serial.print(temperatura, 2);
            Serial.println(" °C");
        }
        delay(200); // pequeña pausa entre ciclos
    }
}
