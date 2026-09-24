#include "conectores.h"
#include "../sensores_analogicos/ph/ph.h"
#include "../sensores_analogicos/humedad_suelo/humedad.h"
#include "../sensores_digitales/temperatura_ds18b20/temperatura.h"
#include "../sensores_digitales/ultrasonico_hcsr04/ultrasonico.h"

// Pines de cada conector. Convención del RJ45: pin 3 = ID3, pin 4 = ID2,
// pin 5 = ID1, pin 6 = ID0. Solo se usan pines pares del Mega para el ID.
struct PinesConector {
    uint8_t id0, id1, id2, id3;   // identificación (INPUT_PULLUP)
    uint8_t senal1;               // señal 1 (Ax)
    uint8_t senal2;               // señal 2 (Ax); hoy solo el HC-SR04 la usa como Echo
};

static const PinesConector PINES[CONECTORES_CANTIDAD] = {
    {28, 26, 24, 22, A0, A1},   // Conector 1
    {36, 34, 32, 30, A2, A3},   // Conector 2
    {44, 42, 40, 38, A4, A5},   // Conector 3
    {52, 50, 48, 46, A6, A7},   // Conector 4
};

// Códigos de identificación (docs/diseno_electronico.md)
static const uint8_t COD_VACIO       = 0;
static const uint8_t COD_PH          = 2;
static const uint8_t COD_HUMEDAD     = 4;
static const uint8_t COD_TEMPERATURA = 9;
static const uint8_t COD_ULTRASONICO = 10;

static const uint8_t MUESTRAS_ANALOGICAS = 8;

// Última medición del DS18B20 de cada conector (lectura sin bloqueo).
struct TempEstado {
    bool          pendiente;   // hay una conversión pedida y sin recoger
    unsigned long inicio;      // millis() en que se pidió
    bool          valido;      // la última lectura recogida fue correcta
    float         valor;       // última lectura recogida
};
static TempEstado tempEstado[CONECTORES_CANTIDAD];

// Lee el código de 4 bits del conector. Con pull-up, un bit en 1 es un pin a GND.
static uint8_t leerCodigo(const PinesConector& c) {
    uint8_t b0 = !digitalRead(c.id0);
    uint8_t b1 = !digitalRead(c.id1);
    uint8_t b2 = !digitalRead(c.id2);
    uint8_t b3 = !digitalRead(c.id3);
    return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

// Promedio de varias lecturas analógicas. Devuelve el ADC como float (0–1023).
// Se fuerza INPUT porque el ultrasónico deja ese pin como salida digital.
static float leerAnalogicoPromedio(uint8_t pin) {
    pinMode(pin, INPUT);
    long suma = 0;
    for (uint8_t i = 0; i < MUESTRAS_ANALOGICAS; i++) suma += analogRead(pin);
    return (float)suma / MUESTRAS_ANALOGICAS;
}

// Deja la lectura como "sin sensor".
static void marcarVacio(LecturaConector& L) {
    L.conectado   = false;
    L.valorValido = false;
    L.valor       = 0.0f;
    L.decimales   = 0;
    L.id          = "";
    L.unidad      = "";
    L.abrev       = "--";
}

// Lee el sensor que corresponde al código y llena L.
static void resolver(LecturaConector& L, const PinesConector& c) {
    marcarVacio(L);

    switch (L.codigo) {
        case COD_PH: {
            float voltaje = (leerAnalogicoPromedio(c.senal1) * PH_VREF) / PH_ADC_MAX;
            L.conectado = true; L.valorValido = true;
            L.valor     = ph_calcularPH(voltaje);
            L.decimales = 2; L.id = "ph"; L.unidad = "pH"; L.abrev = "PH";
            break;
        }
        case COD_HUMEDAD: {
            float adc = leerAnalogicoPromedio(c.senal1);
            L.conectado = true; L.valorValido = true;
            L.valor     = hum_calcularHumedad((int)(adc + 0.5f));
            L.decimales = 1; L.id = "humedad"; L.unidad = "%"; L.abrev = "HUM";
            break;
        }
        case COD_TEMPERATURA: {
            // Lectura sin bloqueo: cada escaneo recoge la conversión pedida en un
            // escaneo anterior (si ya pasaron TEMP_CONVERSION_MS) y pide otra.
            // Hasta que llega la primera, valorValido queda en false.
            TempEstado& t = tempEstado[L.puerto - 1];
            temp_configurarPin(c.senal1);
            if (t.pendiente && millis() - t.inicio >= TEMP_CONVERSION_MS) {
                float lectura = temp_leerUltimaConversion();
                t.valido    = (lectura != TEMP_ERROR);
                t.valor     = lectura;
                t.pendiente = false;
            }
            if (!t.pendiente) {
                temp_pedirConversion();
                t.pendiente = true;
                t.inicio    = millis();
            }
            L.conectado   = true;
            L.valorValido = t.valido;
            L.valor       = t.valor;
            L.decimales   = 2; L.id = "temperatura"; L.unidad = "\xC2\xB0" "C"; L.abrev = "TEMP";
            break;
        }
        case COD_ULTRASONICO: {
            ultra_configurarPines(c.senal1, c.senal2);
            float d = ultra_leerDistanciaCM();
            L.conectado   = true;
            L.valorValido = (d != ULTRA_ERROR);
            L.valor       = d;
            L.decimales   = 1; L.id = "distancia"; L.unidad = "cm"; L.abrev = "DIST";
            break;
        }
        case COD_VACIO:
        default:
            break;   // vacío o código sin programar: queda como marcarVacio()
    }
}

void conectores_inicializar() {
    for (uint8_t i = 0; i < CONECTORES_CANTIDAD; i++) {
        pinMode(PINES[i].id0, INPUT_PULLUP);
        pinMode(PINES[i].id1, INPUT_PULLUP);
        pinMode(PINES[i].id2, INPUT_PULLUP);
        pinMode(PINES[i].id3, INPUT_PULLUP);
    }
    ph_inicializar();
    hum_inicializar();
}

void conectores_escanear(LecturaConector lecturas[CONECTORES_CANTIDAD]) {
    for (uint8_t i = 0; i < CONECTORES_CANTIDAD; i++) {
        lecturas[i].puerto = i + 1;
        lecturas[i].codigo = leerCodigo(PINES[i]);
        resolver(lecturas[i], PINES[i]);
    }
}
