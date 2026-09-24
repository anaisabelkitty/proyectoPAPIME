// Prueba del módulo nucleo/conectores: escanea los 4 RJ45 y muestra el resultado
// por el monitor USB (9600). Enchufa un sensor en cualquier conector para verlo.

#include <Arduino.h>
#include "../nucleo/conectores.h"

LecturaConector lecturas[CONECTORES_CANTIDAD];

void setup() {
    Serial.begin(9600);
    conectores_inicializar();
    Serial.println(F("=== Prueba de conectores ==="));
}

void loop() {
    conectores_escanear(lecturas);

    uint8_t conectados = 0;
    for (uint8_t i = 0; i < CONECTORES_CANTIDAD; i++) {
        const LecturaConector& L = lecturas[i];
        Serial.print(F("P")); Serial.print(L.puerto);
        Serial.print(F(" (codigo ")); Serial.print(L.codigo); Serial.print(F(")  "));

        if (!L.conectado) {
            Serial.println(F("--"));
            continue;
        }
        conectados++;
        Serial.print(L.abrev); Serial.print(F("  "));
        if (L.valorValido) {
            Serial.print(L.valor, L.decimales);
            Serial.print(' '); Serial.println(L.unidad);
        } else {
            Serial.println(F("ERROR de lectura"));
        }
    }
    Serial.print(conectados); Serial.println(F(" de 4 conectados"));
    Serial.println();
    delay(1000);
}
