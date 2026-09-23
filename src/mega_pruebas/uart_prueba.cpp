// Prueba de comunicación UART Mega ↔ ESP32-C6 (lado del Mega).
// Guía de conexión: docs/prueba_uart_esp32_mega.md
//
// Cada segundo manda "PING <n>" por Serial1 (TX1 = pin 18, RX1 = pin 19) y espera
// que la ESP32 conteste "PONG <n>". El resultado se imprime en el monitor USB (9600).
// El programa de la ESP32 está en src/esp32_pruebas/uart_prueba.cpp.

#include <Arduino.h>

const unsigned long BAUDIOS_ESP32 = 115200;
const unsigned long INTERVALO_MS  = 1000;
const unsigned long TIMEOUT_MS    = 500;

unsigned long contador = 0;
unsigned long aciertos = 0;
unsigned long fallos   = 0;

void setup() {
    Serial.begin(9600);
    Serial1.begin(BAUDIOS_ESP32);
    Serial.println(F("=== Prueba UART Mega <-> ESP32-C6 ==="));
    Serial.println(F("Mega TX1 (18) -> divisor -> ESP32 GPIO17 (RX)"));
    Serial.println(F("Mega RX1 (19) <-------------- ESP32 GPIO16 (TX)"));
    Serial.println();
}

// Espera una línea terminada en '\n' hasta el timeout. Devuelve cuántos bytes llegaron
// (aunque no formen una línea completa) para poder diagnosticar ruido o baudios incorrectos.
int esperarLinea(char *buf, size_t max, bool &completa) {
    size_t n = 0;
    completa = false;
    unsigned long inicio = millis();
    while (millis() - inicio < TIMEOUT_MS) {
        while (Serial1.available()) {
            char c = Serial1.read();
            if (c == '\n') {
                buf[n] = '\0';
                completa = true;
                return n;
            }
            if (c != '\r' && n < max - 1) buf[n++] = c;
        }
    }
    buf[n] = '\0';
    return n;
}

void loop() {
    unsigned long inicioCiclo = millis();
    contador++;
    while (Serial1.available()) Serial1.read(); // descarta basura anterior

    Serial1.print(F("PING "));
    Serial1.println(contador);

    char esperado[24];
    snprintf(esperado, sizeof(esperado), "PONG %lu", contador);

    char respuesta[32];
    bool completa;
    int bytes = esperarLinea(respuesta, sizeof(respuesta), completa);

    Serial.print(F("#"));
    Serial.print(contador);
    Serial.print(F("  "));

    if (completa && strcmp(respuesta, esperado) == 0) {
        aciertos++;
        Serial.print(F("OK    recibido: "));
        Serial.println(respuesta);
    } else {
        fallos++;
        if (bytes == 0) {
            Serial.println(F("FALLO sin respuesta (revisar TX/RX cruzados, GND comun, divisor, ESP32 encendida)"));
        } else {
            Serial.print(F("FALLO llegaron "));
            Serial.print(bytes);
            Serial.print(F(" bytes no validos: \""));
            Serial.print(respuesta);
            Serial.println(F("\" (revisar baudios o ruido)"));
        }
    }

    if (contador % 10 == 0) {
        Serial.print(F("--- Resumen: "));
        Serial.print(aciertos);
        Serial.print(F(" OK / "));
        Serial.print(fallos);
        Serial.println(F(" fallos ---"));
    }

    unsigned long transcurrido = millis() - inicioCiclo;
    if (transcurrido < INTERVALO_MS) delay(INTERVALO_MS - transcurrido);
}
