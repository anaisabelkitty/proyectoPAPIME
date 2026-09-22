#include "enlace_esp32.h"
#include <string.h>

static const long ENLACE_BAUDIOS = 115200;

void enlace_inicializar() {
    Serial1.begin(ENLACE_BAUDIOS);
}

void enlace_reset(EstadoEnlace* e) {
    e->nombreKit[0] = '\0';
    e->wifi[0]      = '\0';
    e->ip[0]        = '\0';
    e->ble[0]       = '\0';
}

// ─── Ida: JSON de sensores (docs/protocolo.md §5) ──────────────────
void enlace_enviarJson(const LecturaConector lect[NUCLEO_NUM_CONECTORES]) {
    Serial1.print(F("{\"s\":["));
    for (uint8_t i = 0; i < NUCLEO_NUM_CONECTORES; i++) {
        const LecturaConector& L = lect[i];
        if (i) Serial1.print(',');
        Serial1.print(F("{\"p\":"));
        Serial1.print(L.puerto);
        if (L.conectado && L.valorValido) {
            Serial1.print(F(",\"c\":true,\"id\":\""));
            Serial1.print(L.id);
            Serial1.print(F("\",\"v\":"));
            Serial1.print(L.valor, L.decimales);
            Serial1.print(F(",\"u\":\""));
            Serial1.print(L.unidad);
            Serial1.print(F("\"}"));
        } else {
            // Conector vacío, código desconocido o lectura fallida: solo "p" y "c".
            // (El protocolo actual no distingue "sin sensor" de "sensor con error".)
            Serial1.print(F(",\"c\":false}"));
        }
    }
    Serial1.println(F("]}"));
}

// ─── Vuelta: estado que reporta el ESP32 ──────────────────────────
static bool setCampo(char* dst, size_t n, const char* src) {
    if (strcmp(dst, src) == 0) return false;
    strncpy(dst, src, n - 1);
    dst[n - 1] = '\0';
    return true;
}

static bool aplicarLinea(EstadoEnlace* e, const char* linea) {
    if (strncmp(linea, "#NAME ", 6) == 0)
        return setCampo(e->nombreKit, sizeof(e->nombreKit), linea + 6);
    if (strncmp(linea, "#WIFI ", 6) == 0)
        return setCampo(e->wifi, sizeof(e->wifi), linea + 6);
    if (strncmp(linea, "#IP ", 4) == 0)
        return setCampo(e->ip, sizeof(e->ip), linea + 4);
    if (strncmp(linea, "#BLE ", 5) == 0)
        return setCampo(e->ble, sizeof(e->ble), linea + 5);
    return false;
}

bool enlace_procesarEntrada(EstadoEnlace* e) {
    static char   linea[48];
    static uint8_t idx = 0;
    bool cambio = false;

    while (Serial1.available()) {
        char c = (char)Serial1.read();
        if (c == '\n' || c == '\r') {
            if (idx == 0) continue;
            linea[idx] = '\0';
            idx = 0;
            // Espejo por USB: para comprobar que SÍ está llegando algo del
            // ESP32 por el cable (pin 19), útil mientras se arma el cableado.
            Serial.print(F("[ESP32] ")); Serial.println(linea);
            if (linea[0] == '#') cambio |= aplicarLinea(e, linea);
        } else if (idx < sizeof(linea) - 1) {
            linea[idx++] = c;
        } else {
            idx = 0;   // línea demasiado larga: descartar
        }
    }
    return cambio;
}
