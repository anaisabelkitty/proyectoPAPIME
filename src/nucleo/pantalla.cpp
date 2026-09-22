#include "pantalla.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <stdlib.h>
#include <string.h>

static const uint8_t LCD_COLS = 20;
static const uint8_t LCD_ROWS = 4;
static const uint8_t CELDA    = 10;   // ancho de cada celda de conector

static LiquidCrystal_I2C* lcd = nullptr;
static bool disponible = false;
static char prev[LCD_ROWS][LCD_COLS + 1];   // última fila escrita (para no repintar)

// ─── Páginas en secuencia (avanzan solo si el kit realmente llegó
// a ese paso; ver pantalla.h) ───────────────────────────────────────
enum Pagina { PAG_BLUETOOTH, PAG_WIFI, PAG_SENSORES };
static Pagina paginaActual = PAG_BLUETOOTH;

// ─── Detección de la dirección I2C del adaptador ───────────────────
// docs/conexiones.md: la dirección no está confirmada; 0x27 y 0x3F son
// las comunes, pero se prueban también el resto del rango del PCF8574.
static uint8_t detectarDireccion() {
    const uint8_t candidatas[] = {
        0x27, 0x3F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
        0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E};
    for (uint8_t i = 0; i < sizeof(candidatas); i++) {
        Wire.beginTransmission(candidatas[i]);
        if (Wire.endTransmission() == 0) return candidatas[i];
    }
    return 0;
}

static void forzarRedibujo() {
    for (uint8_t r = 0; r < LCD_ROWS; r++) strcpy(prev[r], "\x01");
}

// ─── Escritura de una fila (solo si cambió) ───────────────────────
static void escribirFila(uint8_t fila, const char* texto) {
    char buf[LCD_COLS + 1];
    uint8_t n = 0;
    for (; n < LCD_COLS && texto[n]; n++) buf[n] = texto[n];
    for (; n < LCD_COLS; n++) buf[n] = ' ';
    buf[LCD_COLS] = '\0';

    if (strcmp(buf, prev[fila]) == 0) return;
    strcpy(prev[fila], buf);
    lcd->setCursor(0, fila);
    lcd->print(buf);
}

// ─── Composición de texto ─────────────────────────────────────────
static void centrar(char* out, const char* texto) {
    uint8_t len = strlen(texto);
    if (len > LCD_COLS) len = LCD_COLS;
    uint8_t pad = (LCD_COLS - len) / 2;
    uint8_t k = 0;
    while (k < pad) out[k++] = ' ';
    for (uint8_t i = 0; i < len; i++) out[k++] = texto[i];
    out[k] = '\0';
}

static void componerNombre(char* out, const EstadoEnlace& e) {
    centrar(out, e.nombreKit[0] ? e.nombreKit : "Sensei");
}

// "N:ABR" (sin valores) recortado/rellenado a CELDA (10) columnas.
static void componerCeldaEstado(char* out, const LecturaConector& L) {
    char tmp[16];
    if (L.conectado) snprintf(tmp, sizeof(tmp), "%d:%s", (int)L.puerto, L.abrev);
    else             snprintf(tmp, sizeof(tmp), "%d:Disp", (int)L.puerto);

    uint8_t n = 0;
    for (; n < CELDA && tmp[n]; n++) out[n] = tmp[n];
    for (; n < CELDA; n++) out[n] = ' ';
    out[CELDA] = '\0';
}

static void componerFilaEstado(char* out, const LecturaConector& a, const LecturaConector& b) {
    char ca[CELDA + 1], cb[CELDA + 1];
    componerCeldaEstado(ca, a);
    componerCeldaEstado(cb, b);
    memcpy(out, ca, CELDA);
    memcpy(out + CELDA, cb, CELDA);
    out[2 * CELDA] = '\0';
}

// ─── Contenido de cada página (filas 1..3; la 0 es el nombre) ─────
static void paginaBluetooth(const EstadoEnlace& e, char filas[][LCD_COLS + 1]) {
    centrar(filas[1], "Bluetooth");
    if (!e.ble[0]) {
        centrar(filas[2], "esperando ESP32");
    } else {
        int n = atoi(e.ble);
        char buf[LCD_COLS + 1];
        if (n <= 0) strcpy(buf, "sin conexiones");
        else        snprintf(buf, sizeof(buf), "%d dispositivo%s", n, n == 1 ? "" : "s");
        centrar(filas[2], buf);
    }
    filas[3][0] = '\0';
}

static void paginaWifi(const EstadoEnlace& e, char filas[][LCD_COLS + 1]) {
    centrar(filas[1], "WiFi");
    if (strcmp(e.wifi, "CONNECTED") == 0) {
        centrar(filas[1], "WiFi conectado");
        centrar(filas[2], e.ip[0] ? e.ip : "");
    } else if (strcmp(e.wifi, "CONNECTING") == 0) {
        centrar(filas[2], "conectando...");
    } else if (strcmp(e.wifi, "FAILED") == 0) {
        centrar(filas[2], "error de conexion");
    } else if (strcmp(e.wifi, "UNCONFIGURED") == 0) {
        centrar(filas[2], "sin configurar");
    } else {
        centrar(filas[2], "esperando ESP32");
    }
    filas[3][0] = '\0';
}

static void paginaSensores(const LecturaConector L[NUCLEO_NUM_CONECTORES],
                           char filas[][LCD_COLS + 1]) {
    componerFilaEstado(filas[1], L[0], L[1]);
    componerFilaEstado(filas[2], L[2], L[3]);

    uint8_t conectados = 0;
    for (uint8_t i = 0; i < NUCLEO_NUM_CONECTORES; i++) {
        if (L[i].conectado) conectados++;
    }
    char buf[LCD_COLS + 1];
    snprintf(buf, sizeof(buf), "%d de %d conectados", conectados, (int)NUCLEO_NUM_CONECTORES);
    centrar(filas[3], buf);
}

// A qué pantalla debería estar el kit según lo que ya sabemos.
// Bluetooth -> WiFi -> Sensores; si algo retrocede (se cae el WiFi),
// esto también hace que la pantalla regrese sola.
static Pagina paginaSegunEstado(const EstadoEnlace& e) {
    if (!e.nombreKit[0])                  return PAG_BLUETOOTH; // el ESP32 nunca reportó
    if (strcmp(e.wifi, "CONNECTED") != 0) return PAG_WIFI;      // hay enlace, falta wifi
    return PAG_SENSORES;                                        // todo listo
}

// ─── API ─────────────────────────────────────────────────────────
bool pantalla_inicializar() {
    Wire.begin();
    uint8_t addr = detectarDireccion();
    if (addr == 0) { disponible = false; return false; }

    lcd = new LiquidCrystal_I2C(addr, LCD_COLS, LCD_ROWS);
    lcd->init();
    lcd->backlight();
    lcd->clear();
    forzarRedibujo();

    paginaActual = PAG_BLUETOOTH;
    disponible   = true;
    return true;
}

void pantalla_splash() {
    if (!disponible) return;
    lcd->clear();
    lcd->setCursor(7, 1); lcd->print(F("Sensei"));
    lcd->setCursor(4, 2); lcd->print(F("Iniciando..."));
    forzarRedibujo();
}

void pantalla_dibujar(const LecturaConector L[NUCLEO_NUM_CONECTORES],
                      const EstadoEnlace& e) {
    if (!disponible) return;

    Pagina deseada = paginaSegunEstado(e);
    if (deseada != paginaActual) {
        paginaActual = deseada;
        forzarRedibujo();
    }

    char filas[LCD_ROWS][LCD_COLS + 1];
    componerNombre(filas[0], e);
    filas[1][0] = filas[2][0] = filas[3][0] = '\0';

    switch (paginaActual) {
        case PAG_BLUETOOTH: paginaBluetooth(e, filas);    break;
        case PAG_WIFI:       paginaWifi(e, filas);        break;
        case PAG_SENSORES:   paginaSensores(L, filas);    break;
        default: break;
    }

    for (uint8_t r = 0; r < LCD_ROWS; r++) escribirFila(r, filas[r]);
}
