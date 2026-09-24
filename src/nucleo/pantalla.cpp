#include "pantalla.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>

static const uint8_t LCD_COLS = 20;
static const uint8_t LCD_ROWS = 4;
static const uint8_t CELDA    = 10;   // ancho de la celda de cada sensor

static LiquidCrystal_I2C* lcd = nullptr;
static bool disponible = false;
static char anterior[LCD_ROWS][LCD_COLS + 1];   // última fila escrita, para no repintar

enum Pagina { PAG_SIN_ENLACE, PAG_BLUETOOTH, PAG_WIFI, PAG_SENSORES };

// ─── Dirección I2C del adaptador ──────────────────────────────────
// 0x27 y 0x3F son las comunes; se prueba también el resto del rango del PCF8574.
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
    for (uint8_t r = 0; r < LCD_ROWS; r++) anterior[r][0] = '\x01';
}

// Escribe una fila rellena con espacios, solo si cambió.
static void escribirFila(uint8_t fila, const char* texto) {
    char buf[LCD_COLS + 1];
    uint8_t n = 0;
    for (; n < LCD_COLS && texto[n]; n++) buf[n] = texto[n];
    for (; n < LCD_COLS; n++) buf[n] = ' ';
    buf[LCD_COLS] = '\0';

    if (strcmp(buf, anterior[fila]) == 0) return;
    strcpy(anterior[fila], buf);
    lcd->setCursor(0, fila);
    lcd->print(buf);
}

// Centra el texto en 20 columnas.
static void centrar(char* out, const char* texto) {
    uint8_t len = strlen(texto);
    if (len > LCD_COLS) len = LCD_COLS;
    uint8_t pad = (LCD_COLS - len) / 2;
    uint8_t k = 0;
    while (k < pad) out[k++] = ' ';
    for (uint8_t i = 0; i < len; i++) out[k++] = texto[i];
    out[k] = '\0';
}

// ─── Contenido de cada pantalla (filas 1..3; la 0 es el nombre) ───
static void paginaSinEnlace(char filas[][LCD_COLS + 1]) {
    centrar(filas[1], "Sin enlace con ESP32");
    centrar(filas[2], "Revisa cable/energia");
}

static void paginaBluetooth(char filas[][LCD_COLS + 1]) {
    centrar(filas[1], "Paso 1 de 3");
    centrar(filas[2], "Conecta el Bluetooth");
    centrar(filas[3], "desde la app");
}

static void paginaWifi(const EstadoEnlace& e, char filas[][LCD_COLS + 1]) {
    centrar(filas[1], "Paso 2 de 3");
    if (strcmp(e.wifi, "CONNECTING") == 0) {
        centrar(filas[2], "Conectando a WiFi...");
    } else if (strcmp(e.wifi, "FAILED") == 0) {
        centrar(filas[2], "No se pudo conectar");
        centrar(filas[3], "Intenta de nuevo");
    } else {
        centrar(filas[2], "Elige la red WiFi");
        centrar(filas[3], "en la app");
    }
}

// "P1: PH" o "P3: --", rellenado a CELDA columnas.
static void componerCelda(char* out, const LecturaConector& L) {
    char tmp[16];
    snprintf(tmp, sizeof(tmp), "P%d: %s", (int)L.puerto, L.conectado ? L.abrev : "--");
    uint8_t n = 0;
    for (; n < CELDA && tmp[n]; n++) out[n] = tmp[n];
    for (; n < CELDA; n++) out[n] = ' ';
    out[CELDA] = '\0';
}

static void componerFilaSensores(char* out, const LecturaConector& a, const LecturaConector& b) {
    char ca[CELDA + 1], cb[CELDA + 1];
    componerCelda(ca, a);
    componerCelda(cb, b);
    memcpy(out, ca, CELDA);
    memcpy(out + CELDA, cb, CELDA);
    out[2 * CELDA] = '\0';
}

static void paginaSensores(const LecturaConector L[CONECTORES_CANTIDAD],
                           char filas[][LCD_COLS + 1]) {
    componerFilaSensores(filas[1], L[0], L[1]);
    componerFilaSensores(filas[2], L[2], L[3]);

    uint8_t conectados = 0;
    for (uint8_t i = 0; i < CONECTORES_CANTIDAD; i++) {
        if (L[i].conectado) conectados++;
    }
    char buf[LCD_COLS + 1];
    snprintf(buf, sizeof(buf), "%d de %d conectados", (int)conectados, (int)CONECTORES_CANTIDAD);
    centrar(filas[3], buf);
}

// Qué pantalla corresponde según lo que se sabe de la ESP32.
static Pagina paginaSegunEstado(const EstadoEnlace& e) {
    if (!enlace_activo(e))                return PAG_SIN_ENLACE;
    if (strcmp(e.wifi, "CONNECTED") == 0) return PAG_SENSORES;
    if (e.ble > 0 || strcmp(e.wifi, "CONNECTING") == 0 || strcmp(e.wifi, "FAILED") == 0)
        return PAG_WIFI;
    return PAG_BLUETOOTH;
}

// ─── API ──────────────────────────────────────────────────────────
bool pantalla_inicializar() {
    Wire.begin();
    uint8_t addr = detectarDireccion();
    if (addr == 0) { disponible = false; return false; }

    lcd = new LiquidCrystal_I2C(addr, LCD_COLS, LCD_ROWS);
    lcd->init();
    lcd->backlight();
    lcd->clear();
    forzarRedibujo();
    disponible = true;
    return true;
}

void pantalla_splash() {
    if (!disponible) return;
    lcd->clear();
    forzarRedibujo();
    char fila[LCD_COLS + 1];
    centrar(fila, "SENSEI");       escribirFila(1, fila);
    centrar(fila, "Iniciando..."); escribirFila(2, fila);
}

void pantalla_dibujar(const LecturaConector L[CONECTORES_CANTIDAD],
                      const EstadoEnlace& e) {
    if (!disponible) return;

    static Pagina paginaActual = PAG_SIN_ENLACE;
    Pagina deseada = paginaSegunEstado(e);
    if (deseada != paginaActual) {
        paginaActual = deseada;
        lcd->clear();
        forzarRedibujo();
    }

    char filas[LCD_ROWS][LCD_COLS + 1];
    for (uint8_t r = 0; r < LCD_ROWS; r++) filas[r][0] = '\0';

    // Fila 0: nombre del kit. Sin enlace, o mientras no se conozca, solo SENSEI.
    bool hayNombre = paginaActual != PAG_SIN_ENLACE && e.nombreKit[0];
    centrar(filas[0], hayNombre ? e.nombreKit : "SENSEI");

    switch (paginaActual) {
        case PAG_SIN_ENLACE: paginaSinEnlace(filas);      break;
        case PAG_BLUETOOTH:  paginaBluetooth(filas);      break;
        case PAG_WIFI:       paginaWifi(e, filas);        break;
        case PAG_SENSORES:   paginaSensores(L, filas);    break;
    }

    for (uint8_t r = 0; r < LCD_ROWS; r++) escribirFila(r, filas[r]);
}
