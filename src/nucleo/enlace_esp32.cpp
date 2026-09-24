#include "enlace_esp32.h"
#include <string.h>
#include <stdlib.h>

static const long ENLACE_BAUDIOS = 115200;

void enlace_inicializar() {
    Serial1.begin(ENLACE_BAUDIOS);
}

void enlace_reset(EstadoEnlace* e) {
    e->nombreKit[0] = '\0';
    e->wifi[0]      = '\0';
    e->ip[0]        = '\0';
    e->ble          = 0;
    e->ultimoRx     = 0;
    e->visto        = false;
}

bool enlace_activo(const EstadoEnlace& e) {
    return e.visto && (millis() - e.ultimoRx < ENLACE_TIMEOUT_MS);
}

// ─── Ida: JSON de sensores (docs/protocolo.md §5) ──────────────────
// Se escribe directo al puerto, sin armar el texto en RAM.
void enlace_enviarJson(const LecturaConector lecturas[CONECTORES_CANTIDAD]) {
    Serial1.print(F("{\"s\":["));
    for (uint8_t i = 0; i < CONECTORES_CANTIDAD; i++) {
        const LecturaConector& L = lecturas[i];
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
            // Vacío, código sin programar o lectura fallida: solo "p" y "c".
            Serial1.print(F(",\"c\":false}"));
        }
    }
    Serial1.println(F("]}"));
}

// ─── Vuelta: estado que reporta la ESP32 ──────────────────────────
static void copiarCampo(char* dst, size_t n, const char* src) {
    strncpy(dst, src, n - 1);
    dst[n - 1] = '\0';
}

static bool aplicarLinea(EstadoEnlace* e, const char* linea) {
    if (strncmp(linea, "#NAME ", 6) == 0) { copiarCampo(e->nombreKit, sizeof(e->nombreKit), linea + 6); return true; }
    if (strncmp(linea, "#WIFI ", 6) == 0) { copiarCampo(e->wifi, sizeof(e->wifi), linea + 6);           return true; }
    if (strncmp(linea, "#IP ", 4) == 0)   { copiarCampo(e->ip, sizeof(e->ip), linea + 4);               return true; }
    if (strncmp(linea, "#BLE ", 5) == 0)  { e->ble = (uint8_t)atoi(linea + 5);                          return true; }
    return false;
}

void enlace_procesarEntrada(EstadoEnlace* e) {
    static char    linea[48];
    static uint8_t idx = 0;

    while (Serial1.available()) {
        char c = (char)Serial1.read();
        if (c == '\n' || c == '\r') {
            if (idx == 0) continue;
            linea[idx] = '\0';
            idx = 0;
            if (linea[0] == '#' && aplicarLinea(e, linea)) {
                e->visto    = true;
                e->ultimoRx = millis();
            }
        } else if (idx < sizeof(linea) - 1) {
            linea[idx++] = c;
        } else {
            idx = 0;   // línea demasiado larga: descartar
        }
    }
}
