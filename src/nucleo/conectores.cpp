#include "conectores.h"

#include "../sensores_analogicos/ph/ph.h"
#include "../sensores_analogicos/humedad_suelo/humedad.h"
#include "../sensores_digitales/temperatura_ds18b20/temperatura.h"
#include "../sensores_digitales/ultrasonico_hcsr04/ultrasonico.h"

namespace {

// Pines del Mega por conector (docs/conexiones.md, "Pines del Mega por conector").
// Solo pines pares para los 16 bits de ID; la fila impar queda libre.
struct Conector {
    int id0, id1, id2, id3;   // identificación (INPUT_PULLUP)
    int senal1, senal2;       // señales (senal2 solo la usa el HC-SR04, como Echo)
};

const Conector CONECTORES[NUCLEO_NUM_CONECTORES] = {
    {28, 26, 24, 22, A0, A1},   // Conector 1
    {36, 34, 32, 30, A2, A3},   // Conector 2
    {44, 42, 40, 38, A4, A5},   // Conector 3
    {52, 50, 48, 46, A6, A7},   // Conector 4
};

// Códigos de identificación (docs/conexiones.md, "Tabla de códigos").
const int COD_VACIO       = 0;
const int COD_PH          = 2;
const int COD_HUMEDAD     = 4;
const int COD_TEMPERATURA = 9;
const int COD_ULTRASONICO = 10;

void configurarID(const Conector& c) {
    pinMode(c.id0, INPUT_PULLUP);
    pinMode(c.id1, INPUT_PULLUP);
    pinMode(c.id2, INPUT_PULLUP);
    pinMode(c.id3, INPUT_PULLUP);
}

int leerCodigo(const Conector& c) {
    int b0 = !digitalRead(c.id0);
    int b1 = !digitalRead(c.id1);
    int b2 = !digitalRead(c.id2);
    int b3 = !digitalRead(c.id3);
    return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

// Llena L según el código ya leído y la señal del conector.
void resolver(LecturaConector& L, const Conector& c) {
    L.conectado   = true;
    L.valorValido = false;
    L.valor       = 0.0f;

    switch (L.codigo) {
        case COD_PH: {
            L.id = "ph"; L.unidad = "pH"; L.abrev = "PH"; L.decimales = 2;
            float v = (analogRead(c.senal1) * PH_VREF) / PH_ADC_MAX;
            L.valor = ph_calcularPH(v);
            L.valorValido = true;
            break;
        }
        case COD_HUMEDAD: {
            L.id = "humedad"; L.unidad = "%"; L.abrev = "Hum"; L.decimales = 1;
            L.valor = hum_calcularHumedad(analogRead(c.senal1));
            L.valorValido = true;
            break;
        }
        case COD_TEMPERATURA: {
            // "\xC2\xB0" "C" = "°C" en UTF-8 (partido para que \xB0 no se coma la 'C').
            L.id = "temperatura"; L.unidad = "\xC2\xB0" "C"; L.abrev = "Tem"; L.decimales = 2;
            temp_configurarPin(c.senal1);
            float t = temp_leerCelsius();
            if (t != TEMP_ERROR) { L.valor = t; L.valorValido = true; }
            break;
        }
        case COD_ULTRASONICO: {
            // OJO: "distancia" todavía no está en la tabla de docs/protocolo.md §5.
            // Confirmar el id definitivo con el desarrollador de Android.
            L.id = "distancia"; L.unidad = "cm"; L.abrev = "Dst"; L.decimales = 1;
            ultra_configurarPines(c.senal1, c.senal2);
            float d = ultra_leerDistanciaCM();
            if (d != ULTRA_ERROR) { L.valor = d; L.valorValido = true; }
            break;
        }
        default:
            // COD_VACIO o código no reconocido: no hay sensor utilizable.
            L.conectado = false;
            L.id = ""; L.unidad = ""; L.abrev = "--"; L.decimales = 0;
            break;
    }
}

}  // namespace

void conectores_inicializar() {
    for (uint8_t i = 0; i < NUCLEO_NUM_CONECTORES; i++) {
        configurarID(CONECTORES[i]);
    }
}

void conectores_escanear(LecturaConector lecturas[NUCLEO_NUM_CONECTORES]) {
    for (uint8_t i = 0; i < NUCLEO_NUM_CONECTORES; i++) {
        LecturaConector& L = lecturas[i];
        L.puerto = i + 1;
        L.codigo = leerCodigo(CONECTORES[i]);
        resolver(L, CONECTORES[i]);
    }
}

void conectores_imprimirSerial() {
    LecturaConector lect[NUCLEO_NUM_CONECTORES];
    conectores_escanear(lect);

    Serial.println(F("-----------------------------------------"));
    for (uint8_t i = 0; i < NUCLEO_NUM_CONECTORES; i++) {
        const LecturaConector& L = lect[i];
        Serial.print(F("  Conector ")); Serial.print(L.puerto);
        Serial.print(F(" (codigo ")); Serial.print(L.codigo); Serial.print(F("): "));
        if (!L.conectado) {
            Serial.println(F("vacio / no reconocido"));
        } else if (!L.valorValido) {
            Serial.print(L.id); Serial.println(F("  (error de lectura)"));
        } else {
            Serial.print(L.id); Serial.print(F(" = "));
            Serial.print(L.valor, L.decimales); Serial.print(' '); Serial.println(L.unidad);
        }
    }
}
