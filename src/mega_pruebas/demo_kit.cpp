// Demo del kit completo en el Mega: conectores + JSON a la ESP32 + LCD.
// Diseño: docs/plan_implementacion.md. Pareja: src/esp32/main.cpp (entorno esp32-c6-kit).
//
// Cada 500 ms: escanea los 4 conectores, manda el JSON a la ESP32 por UART y
// redibuja el LCD. Por el monitor USB (9600) muestra las lecturas cada segundo,
// porque el LCD no muestra valores numéricos.

#include <Arduino.h>
#include "../nucleo/conectores.h"
#include "../nucleo/enlace_esp32.h"
#include "../nucleo/pantalla.h"

static const unsigned long CICLO_MS = 500;

static LecturaConector lecturas[CONECTORES_CANTIDAD];
static EstadoEnlace    enlace;
static unsigned long   ultimoCiclo = 0;
static uint8_t         ciclos = 0;

// Resumen de una línea para el monitor USB.
static void imprimirLecturas() {
    for (uint8_t i = 0; i < CONECTORES_CANTIDAD; i++) {
        const LecturaConector& L = lecturas[i];
        Serial.print(F("P")); Serial.print(L.puerto); Serial.print(F(" "));
        if (!L.conectado)        Serial.print(F("--"));
        else if (!L.valorValido) { Serial.print(L.abrev); Serial.print(F(" ...")); }
        else { Serial.print(L.abrev); Serial.print(' '); Serial.print(L.valor, L.decimales); }
        Serial.print(F(" | "));
    }
    Serial.print(F("ESP32: "));
    if (!enlace_activo(enlace)) Serial.println(F("sin enlace"));
    else {
        Serial.print(enlace.nombreKit); Serial.print(F(" WiFi "));
        Serial.print(enlace.wifi); Serial.print(F(" BLE "));
        Serial.println(enlace.ble);
    }
}

void setup() {
    Serial.begin(9600);
    conectores_inicializar();
    enlace_inicializar();
    enlace_reset(&enlace);

    pantalla_inicializar();   // si no hay LCD conectado, el resto sigue igual
    pantalla_splash();
    delay(1500);

    Serial.println(F("=== Demo del kit Sensei ==="));
}

void loop() {
    enlace_procesarEntrada(&enlace);

    if (millis() - ultimoCiclo >= CICLO_MS) {
        ultimoCiclo = millis();
        conectores_escanear(lecturas);
        enlace_enviarJson(lecturas);
        pantalla_dibujar(lecturas, enlace);

        if (++ciclos >= 2) {   // cada 2 ciclos = 1 s
            ciclos = 0;
            imprimirLecturas();
        }
    }
}
