// Prueba completa BLE + WiFi + UART (lado del Mega).
// Guía: docs/prueba_wifi_ble_esp32_mega.md. Pareja: src/esp32_pruebas/wifi_ble_uart_prueba.cpp
//
// Ida    (Mega -> ESP32): JSON de sensores simulado cada 500 ms, mismo formato que el
//                         firmware real (docs/protocolo.md §5). La ESP32 lo reenvía por
//                         WebSocket y lo sirve en GET /data.
// Vuelta (ESP32 -> Mega): líneas "#NAME", "#WIFI", "#BLE", "#IP" cada 3 s. Se imprimen
//                         en el monitor USB (9600) y se resume el estado.

#include <Arduino.h>

const unsigned long BAUDIOS_ESP32 = 115200;
const unsigned long INTERVALO_JSON_MS = 500;

unsigned long ultimoEnvio = 0;
unsigned long jsonEnviados = 0;
unsigned long ultimaLineaEsp32 = 0;
bool esp32Vista = false;

char nombreKit[16] = "";
char wifi[14] = "";
char ble[4] = "";
char ip[16] = "";

void setup() {
    Serial.begin(9600);
    Serial1.begin(BAUDIOS_ESP32);
    randomSeed(analogRead(A15));
    Serial.println(F("=== Prueba BLE + WiFi + UART (Mega) ==="));
    Serial.println(F("Mando JSON simulado cada 500 ms y muestro lo que reporta la ESP32."));
    Serial.println();
}

void enviarJsonSimulado() {
    float temperatura = 22.0 + random(0, 500) / 100.0;
    float ph = 6.5 + random(0, 200) / 100.0;

    Serial1.print(F("{\"s\":["));
    Serial1.print(F("{\"p\":1,\"c\":true,\"id\":\"temperatura\",\"v\":"));
    Serial1.print(temperatura, 1);
    Serial1.print(F(",\"u\":\"C\"},"));
    Serial1.print(F("{\"p\":2,\"c\":true,\"id\":\"ph\",\"v\":"));
    Serial1.print(ph, 2);
    Serial1.print(F(",\"u\":\"pH\"},"));
    Serial1.print(F("{\"p\":3,\"c\":false},{\"p\":4,\"c\":false}"));
    Serial1.println(F("]}"));
    jsonEnviados++;
}

void copiar(char *dst, size_t n, const char *src) {
    strncpy(dst, src, n - 1);
    dst[n - 1] = '\0';
}

void procesarLinea(const char *linea) {
    if (linea[0] != '#') {
        Serial.print(F("[ESP32 ?] "));
        Serial.println(linea);
        return;
    }
    if (strncmp(linea, "#NAME ", 6) == 0) copiar(nombreKit, sizeof(nombreKit), linea + 6);
    else if (strncmp(linea, "#WIFI ", 6) == 0) copiar(wifi, sizeof(wifi), linea + 6);
    else if (strncmp(linea, "#BLE ", 5) == 0) copiar(ble, sizeof(ble), linea + 5);
    else if (strncmp(linea, "#IP ", 4) == 0) copiar(ip, sizeof(ip), linea + 4);
    Serial.print(F("[ESP32] "));
    Serial.println(linea);
}

void leerEsp32() {
    static char linea[48];
    static uint8_t idx = 0;
    while (Serial1.available()) {
        char c = Serial1.read();
        if (c == '\n' || c == '\r') {
            if (idx == 0) continue;
            linea[idx] = '\0';
            idx = 0;
            esp32Vista = true;
            ultimaLineaEsp32 = millis();
            procesarLinea(linea);
        } else if (idx < sizeof(linea) - 1) {
            linea[idx++] = c;
        } else {
            idx = 0;
        }
    }
}

void imprimirResumen() {
    static unsigned long ultimoResumen = 0;
    if (millis() - ultimoResumen < 5000) return;
    ultimoResumen = millis();

    Serial.print(F("--- JSON enviados: "));
    Serial.print(jsonEnviados);
    if (!esp32Vista) {
        Serial.println(F(" | ESP32: SIN RESPUESTA (revisar GPIO16 -> RX1 pin 19 y GND comun) ---"));
        return;
    }
    Serial.print(F(" | ESP32: "));
    Serial.print(nombreKit);
    Serial.print(F(" | WiFi: "));
    Serial.print(wifi);
    Serial.print(F(" | BLE: "));
    Serial.print(ble);
    if (ip[0]) {
        Serial.print(F(" | IP: "));
        Serial.print(ip);
    }
    Serial.println(F(" ---"));
}

void loop() {
    if (millis() - ultimoEnvio >= INTERVALO_JSON_MS) {
        ultimoEnvio = millis();
        enviarJsonSimulado();
    }
    leerEsp32();
    imprimirResumen();
}
