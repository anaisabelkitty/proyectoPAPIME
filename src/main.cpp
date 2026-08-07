// Kit Sensei — Menú principal
//
// Opciones:
//   1 → Leer pH en tiempo real           (sensor PH-4502C, pin A2)
//   2 → Leer temperatura en tiempo real  (sensor DS18B20, pin 7)
//   3 → Calibración de offset BNC        (ajustar POT2 del módulo pH)
//   4 → Calibración con buffers          (Lagrange — actualiza EEPROM)
//   5 → Leer humedad de suelo            (sensor OKY3442, pin A4)
//   6 → pH + Temperatura simultáneos     (para práctica 03)
//   7 → Calibrar humedad de suelo        (2 puntos, Lagrange — EEPROM)
//   8 → Leer distancia en tiempo real    (sensor HC-SR04, Trig 8 / Echo 9)
//   9 → Escanear los 4 conectores        (identificación automática por código)
//   0 → Volver al menú

#include <Arduino.h>
#include "sensores_analogicos/ph/ph.h"
#include "sensores_digitales/temperatura_ds18b20/temperatura.h"
#include "sensores_analogicos/humedad_suelo/humedad.h"
#include "sensores_digitales/ultrasonico_hcsr04/ultrasonico.h"

// ─── Pines de los 4 conectores RJ45 (identificación + señal) ───
// Ver docs/diseno_electronico.md, sección "Pines del Mega dedicados a esto".
// Convención fija: en el RJ45, de izquierda a derecha, va ID3, ID2, ID1, ID0
// (pin 3 = ID3 ... pin 6 = ID0), así el binario se lee directo en el conector
// en el mismo orden en que se escribe. Ver docs/diseno_electronico.md.
struct Conector {
    int id0, id1, id2, id3; // pines de identificación (INPUT_PULLUP)
    int senal1;             // pin de señal 1 (Ax)
    int senal2;             // pin de señal 2 (Ax); hoy solo la usa el HC-SR04 como Echo
};

const int NUM_CONECTORES = 4;
Conector conectores[NUM_CONECTORES] = {
    {25, 24, 23, 22, A0, A1},  // Conector 1 (RJ45 pin6=id0, pin5=id1, pin4=id2, pin3=id3)
    {29, 28, 27, 26, A2, A3},  // Conector 2
    {33, 32, 31, 30, A4, A5},  // Conector 3
    {37, 36, 35, 34, A6, A7},  // Conector 4
};

// Códigos de identificación (ver tabla en docs/diseno_electronico.md)
const int COD_VACIO       = 0; // también es el código de "Voltaje AR2657", sin implementar todavía
const int COD_PH          = 2;
const int COD_HUMEDAD     = 4;
const int COD_TEMPERATURA = 8;
const int COD_ULTRASONICO = 9;

// ─── Constantes de calibración buffer ───────────────────────
const float CAL_PH_4  =  4.01f;
const float CAL_PH_7  =  7.00f;
const float CAL_PH_10 = 10.01f;
const int   CAL_MUESTRAS = 30;

// ─── Estado global ───────────────────────────────────────────
int modoActivo   = 0;
int modoCal      = 0;
bool calEnCurso  = false;
float cal_v7     = 0.0f;
float cal_v2     = 0.0f;
float cal_ph2    = 0.0f;
int   cal_paso   = 0;

// ─── Estado calibración de humedad ───────────────────────────
int humCal_paso  = 0;
int humCal_seco  = 0;

// ─── Identificación por código binario (pull-up interno) ──────
void configurarPinesID(Conector &c) {
    pinMode(c.id0, INPUT_PULLUP);
    pinMode(c.id1, INPUT_PULLUP);
    pinMode(c.id2, INPUT_PULLUP);
    pinMode(c.id3, INPUT_PULLUP);
}

int leerCodigoID(Conector &c) {
    int b0 = !digitalRead(c.id0);
    int b1 = !digitalRead(c.id1);
    int b2 = !digitalRead(c.id2);
    int b3 = !digitalRead(c.id3);
    return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}

// Lee el sensor detectado en un conector y lo imprime.
void leerYMostrarConector(int numero, Conector &c) {
    int codigo = leerCodigoID(c);

    Serial.print("  Conector "); Serial.print(numero);
    Serial.print(" (codigo "); Serial.print(codigo); Serial.print("): ");

    if (codigo == COD_PH) {
        int   adc     = analogRead(c.senal1);
        float voltaje = (adc * PH_VREF) / PH_ADC_MAX;
        float pH      = ph_calcularPH(voltaje);
        Serial.print("pH = "); Serial.println(pH, 2);
    }
    else if (codigo == COD_HUMEDAD) {
        int   adc     = analogRead(c.senal1);
        float humedad = hum_calcularHumedad(adc);
        Serial.print("Humedad = "); Serial.print(humedad, 1); Serial.println(" %");
    }
    else if (codigo == COD_TEMPERATURA) {
        temp_configurarPin(c.senal1);
        float t = temp_leerCelsius();
        if (t == TEMP_ERROR) Serial.println("ERROR DS18B20");
        else { Serial.print("Temperatura = "); Serial.print(t, 2); Serial.println(" C"); }
    }
    else if (codigo == COD_ULTRASONICO) {
        ultra_configurarPines(c.senal1, c.senal2);
        float d = ultra_leerDistanciaCM();
        if (d == ULTRA_ERROR) Serial.println("ERROR (sin eco o fuera de rango)");
        else { Serial.print("Distancia = "); Serial.print(d, 1); Serial.println(" cm"); }
    }
    else if (codigo == COD_VACIO) {
        Serial.println("vacio");
    }
    else {
        Serial.println("codigo no reconocido");
    }
}

// ─── Vaciar buffer serial ─────────────────────────────────────
void vaciarSerial() {
    delay(10);
    while (Serial.available() > 0) Serial.read();
}

// ─── Utilidades de calibración ───────────────────────────────

float cal_leerVoltajePromedio() {
    long suma = 0;
    for (int i = 0; i < CAL_MUESTRAS; i++) {
        suma += analogRead(PH_PIN_SENSOR);
        delay(20);
    }
    return ((float)suma / CAL_MUESTRAS) * PH_VREF / PH_ADC_MAX;
}

void cal_imprimirResultados(float v_a, float ph_a, float v_b, float ph_b, float v_7) {
    float sensibilidad = (v_b - v_a) / (ph_a - ph_b);
    float offset       = v_7;
    float ph_calc_a = 7.0 + (offset - v_a) / sensibilidad;
    float ph_calc_b = 7.0 + (offset - v_b) / sensibilidad;

    Serial.println();
    Serial.println("=========================================");
    Serial.println("  RESULTADOS DE CALIBRACION");
    Serial.println("=========================================");
    Serial.print("    PH_OFFSET_PH7   = "); Serial.println(offset, 4);
    Serial.print("    PH_SENSIBILIDAD = "); Serial.println(sensibilidad, 4);
    Serial.println();
    Serial.println("  Verificacion:");
    Serial.print("    Buffer pH "); Serial.print(ph_a, 2);
    Serial.print("  → calcula pH: "); Serial.println(ph_calc_a, 2);
    Serial.print("    Buffer pH "); Serial.print(ph_b, 2);
    Serial.print("  → calcula pH: "); Serial.println(ph_calc_b, 2);
    ph_guardarCalibracion(offset, sensibilidad);
    Serial.println("  ✔ Guardado en EEPROM. Escribe 0 para volver.");
    Serial.println("=========================================");
}

// ─── Menú ────────────────────────────────────────────────────

void imprimirMenu() {
    Serial.println();
    Serial.println("=========================================");
    Serial.println("  Kit Sensei");
    Serial.println("=========================================");
    Serial.print("  pH: offset="); Serial.print(ph_offset, 4);
    Serial.print("  sens="); Serial.println(ph_sensibilidad, 4);
    if (ph_tieneCalibacionGuardada()) Serial.println("  (calibracion en EEPROM)");
    else                              Serial.println("  (valores por defecto)");
    Serial.println("-----------------------------------------");
    Serial.println("  1  pH en tiempo real");
    Serial.println("  2  Temperatura en tiempo real");
    Serial.println("  3  Calibrar offset BNC (POT2)");
    Serial.println("  4  Calibrar pH con buffers");
    Serial.println("  5  Humedad de suelo");
    Serial.println("  6  pH + Temperatura simultaneos");
    Serial.println("  7  Calibrar humedad (2 puntos)");
    Serial.println("  8  Distancia (HC-SR04)");
    Serial.println("  9  Escanear los 4 conectores (identificacion)");
    Serial.println("  0  Volver al menu");
    Serial.println("-----------------------------------------");
    Serial.print("> ");
}

// ─── Leer dígito del Serial (compatible con "No line ending") ─
// Con "No line ending" el carácter llega solo, sin terminador.
// Esta función devuelve el número en cuanto llega el primer dígito.
static String _buf = "";
int leerEntero() {
    while (Serial.available() > 0) {
        char c = (char)Serial.read();
        if (c == '\n' || c == '\r') {
            // Con terminador: procesar lo acumulado
            if (_buf.length() > 0) {
                int n = _buf.toInt();
                _buf = "";
                return n;
            }
        } else if (isDigit(c)) {
            // Sin terminador: procesar inmediatamente al llegar el dígito
            // (vaciamos el buffer por si quedó algo)
            _buf = "";
            while (Serial.available() > 0) Serial.read();
            return c - '0';
        }
    }
    return -1;
}

// ─── Setup ───────────────────────────────────────────────────

void setup() {
    Serial.begin(9600);
    pinMode(PH_PIN_SENSOR, INPUT);
    ph_inicializar();
    temp_inicializar();
    hum_inicializar();
    ultra_inicializar();
    for (int i = 0; i < NUM_CONECTORES; i++) {
        configurarPinesID(conectores[i]);
    }
    imprimirMenu();
}

// ─── Loop ────────────────────────────────────────────────────

void loop() {

    // ── Menú principal ────────────────────────────────────────
    if (modoActivo == 0 && !calEnCurso) {
        int op = leerEntero();
        if (op == -1) return;

        Serial.println(); // salto después del echo

        if      (op == 1) { modoActivo = 1; Serial.println(">> pH (escribe 0 para volver)"); }
        else if (op == 2) { modoActivo = 2; Serial.println(">> Temperatura (escribe 0 para volver)"); }
        else if (op == 3) {
            modoActivo = 3;
            Serial.println("=========================================");
            Serial.println("  AJUSTE DE OFFSET — POT2 (junto al BNC)");
            Serial.println("  1. Desconecta el electrodo del BNC.");
            Serial.println("  2. Cortocircuita el BNC con un cable.");
            Serial.println("  3. Gira POT2 hasta ver: CORRECTO");
            Serial.println("  Escribe 0 para salir.");
            Serial.println("=========================================");
        }
        else if (op == 4) {
            modoActivo = 4; modoCal = 0; cal_paso = 0;
            Serial.println("=========================================");
            Serial.println("  CALIBRACION CON SOLUCIONES BUFFER");
            Serial.println("  A → pH 4.01 + pH 7.00  (muestras acidas)");
            Serial.println("  B → pH 7.00 + pH 10.01 (muestras alcalinas)");
            Serial.println("  Escribe A o B:");
            Serial.print("> ");
        }
        else if (op == 5) { modoActivo = 5; Serial.println(">> Humedad (escribe 0 para volver)"); }
        else if (op == 6) {
            modoActivo = 6;
            Serial.println(">> pH + Temperatura (escribe 0 para volver)");
            Serial.println("pH       | Temperatura");
            Serial.println("---------|-----------");
        }
        else if (op == 7) {
            modoActivo = 7; humCal_paso = 0;
            Serial.println("=========================================");
            Serial.println("  CALIBRACION HUMEDAD DE SUELO");
            Serial.println("  Paso 1: sonda al aire o en suelo SECO.");
            Serial.println("  Presiona cualquier tecla cuando este lista.");
            Serial.print("> ");
        }
        else if (op == 8) { modoActivo = 8; Serial.println(">> Distancia HC-SR04 (escribe 0 para volver)"); }
        else if (op == 9) { modoActivo = 9; Serial.println(">> Escaneo de conectores (escribe 0 para volver)"); }
        else if (op == 0) { imprimirMenu(); }
        else { Serial.println("Opcion no valida."); imprimirMenu(); }
        return;
    }

    // ── Modo 1: pH ────────────────────────────────────────────
    if (modoActivo == 1) {
        if (Serial.available() > 0 && Serial.peek() == '0') {
            while (Serial.available() > 0) Serial.read();
            modoActivo = 0; _buf = ""; imprimirMenu(); return;
        }
        float voltaje = ph_leerVoltaje();
        float pH      = ph_calcularPH(voltaje);
        // Verificar de nuevo después de la lectura (puede haber llegado el 0 durante el muestreo)
        if (Serial.available() > 0 && Serial.peek() == '0') {
            while (Serial.available() > 0) Serial.read();
            modoActivo = 0; _buf = ""; imprimirMenu(); return;
        }
        Serial.print("Voltaje: "); Serial.print(voltaje, 3);
        Serial.print(" V  |  pH: "); Serial.println(pH, 2);
        // Esperar 1s pero revisando cada 100ms si llegó el 0
        for (int i = 0; i < 10; i++) {
            delay(100);
            if (Serial.available() > 0 && Serial.peek() == '0') {
                while (Serial.available() > 0) Serial.read();
                modoActivo = 0; _buf = ""; imprimirMenu(); return;
            }
        }
        return;
    }

    // ── Modo 2: temperatura ───────────────────────────────────
    if (modoActivo == 2) {
        if (Serial.available() > 0 && Serial.peek() == '0') {
            while (Serial.available() > 0) Serial.read();
            modoActivo = 0; _buf = ""; imprimirMenu(); return;
        }
        float t = temp_leerCelsius();
        if (Serial.available() > 0 && Serial.peek() == '0') {
            while (Serial.available() > 0) Serial.read();
            modoActivo = 0; _buf = ""; imprimirMenu(); return;
        }
        if (t == TEMP_ERROR) Serial.println("ERROR: DS18B20 desconectado.");
        else { Serial.print("Temperatura: "); Serial.print(t, 2); Serial.println(" °C"); }
        for (int i = 0; i < 10; i++) {
            delay(100);
            if (Serial.available() > 0 && Serial.peek() == '0') {
                while (Serial.available() > 0) Serial.read();
                modoActivo = 0; _buf = ""; imprimirMenu(); return;
            }
        }
        return;
    }

    // ── Modo 3: offset BNC ────────────────────────────────────
    if (modoActivo == 3) {
        if (Serial.available() > 0 && Serial.peek() == '0') {
            while (Serial.available() > 0) Serial.read();
            modoActivo = 0; _buf = ""; imprimirMenu(); return;
        }
        long suma = 0;
        for (int i = 0; i < 20; i++) { suma += analogRead(PH_PIN_SENSOR); delay(10); }
        float voltaje = ((float)suma / 20.0) * PH_VREF / PH_ADC_MAX;
        Serial.print("Voltaje Po: "); Serial.print(voltaje, 3); Serial.print(" V  →  ");
        if      (voltaje < 2.30)  Serial.println("subir POT2");
        else if (voltaje < 2.48)  Serial.println("subir un poco POT2");
        else if (voltaje <= 2.52) Serial.println("✔ CORRECTO");
        else if (voltaje <= 2.70) Serial.println("bajar un poco POT2");
        else                      Serial.println("bajar POT2");
        for (int i = 0; i < 10; i++) {
            delay(100);
            if (Serial.available() > 0 && Serial.peek() == '0') {
                while (Serial.available() > 0) Serial.read();
                modoActivo = 0; _buf = ""; imprimirMenu(); return;
            }
        }
        return;
    }

    // ── Modo 4: calibración pH con buffers ───────────────────
    if (modoActivo == 4) {

        if (cal_paso == 0 && Serial.available() > 0) {
            char c = (char)Serial.read();
            vaciarSerial();
            c = tolower(c);
            if (c == '\r' || c == '\n') return;
            if (c == '0') { modoActivo = 0; imprimirMenu(); return; }
            if (c == 'a') { modoCal = 1; cal_paso = 1; Serial.println("Modo A: pH 4.01 + 7.00"); }
            else if (c == 'b') { modoCal = 2; cal_paso = 1; Serial.println("Modo B: pH 7.00 + 10.01"); }
            else { Serial.println("Escribe A, B o 0:"); Serial.print("> "); return; }
            Serial.println("--- PASO 1/2 --- Sumerge en pH 7.00, espera 2 min, presiona ENTER:");
            Serial.print("> ");
        }
        else if (cal_paso == 1 && Serial.available() > 0) {
            vaciarSerial();
            Serial.print("  Leyendo...");
            cal_v7 = cal_leerVoltajePromedio();
            Serial.print(" V7="); Serial.println(cal_v7, 4);
            cal_paso = 2;
            float ph2 = (modoCal == 1) ? CAL_PH_4 : CAL_PH_10;
            Serial.print("--- PASO 2/2 --- Enjuaga, sumerge en pH ");
            Serial.print(ph2, 2); Serial.println(", espera 2 min, presiona ENTER:");
            Serial.print("> ");
        }
        else if (cal_paso == 2 && Serial.available() > 0) {
            vaciarSerial();
            Serial.print("  Leyendo...");
            cal_v2  = cal_leerVoltajePromedio();
            cal_ph2 = (modoCal == 1) ? CAL_PH_4 : CAL_PH_10;
            Serial.print(" V2="); Serial.println(cal_v2, 4);
            cal_imprimirResultados(cal_v7, CAL_PH_7, cal_v2, cal_ph2, cal_v7);
            cal_paso = 0; modoCal = 0; modoActivo = 0; imprimirMenu();
        }
        return;
    }

    // ── Modo 5: humedad ───────────────────────────────────────
    if (modoActivo == 5) {
        if (Serial.available() > 0 && Serial.peek() == '0') {
            while (Serial.available() > 0) Serial.read();
            modoActivo = 0; _buf = ""; imprimirMenu(); return;
        }
        int   adc     = hum_leerADC();
        float humedad = hum_calcularHumedad(adc);
        if (Serial.available() > 0 && Serial.peek() == '0') {
            while (Serial.available() > 0) Serial.read();
            modoActivo = 0; _buf = ""; imprimirMenu(); return;
        }
        Serial.print("Humedad: "); Serial.print(humedad, 1);
        Serial.print("%  (ADC: "); Serial.print(adc); Serial.println(")");
        for (int i = 0; i < 10; i++) {
            delay(100);
            if (Serial.available() > 0 && Serial.peek() == '0') {
                while (Serial.available() > 0) Serial.read();
                modoActivo = 0; _buf = ""; imprimirMenu(); return;
            }
        }
        return;
    }

    // ── Modo 6: pH + temperatura ──────────────────────────────
    if (modoActivo == 6) {
        if (Serial.available() > 0 && Serial.peek() == '0') {
            while (Serial.available() > 0) Serial.read();
            modoActivo = 0; _buf = ""; imprimirMenu(); return;
        }
        float t       = temp_leerCelsius();
        float voltaje = ph_leerVoltaje();
        float pH      = ph_calcularPH(voltaje);
        if (Serial.available() > 0 && Serial.peek() == '0') {
            while (Serial.available() > 0) Serial.read();
            modoActivo = 0; _buf = ""; imprimirMenu(); return;
        }
        Serial.print(pH, 2); Serial.print("      | ");
        if (t == TEMP_ERROR) Serial.println("ERROR DS18B20");
        else { Serial.print(t, 2); Serial.println(" °C"); }
        delay(200);
        return;
    }

    // ── Modo 7: calibración humedad ───────────────────────────
    if (modoActivo == 7) {

        if (humCal_paso == 0 && Serial.available() > 0) {
            char c = (char)Serial.read();
            vaciarSerial();
            if (c == '\r' || c == '\n') return;
            if (c == '0') { modoActivo = 0; imprimirMenu(); return; }
            humCal_seco = hum_leerADC();
            Serial.print("  ADC seco: "); Serial.println(humCal_seco);
            Serial.println("  Paso 2: sonda en agua o suelo SATURADO.");
            Serial.println("  Presiona cualquier tecla cuando este lista.");
            Serial.print("> ");
            humCal_paso = 1;
        }
        else if (humCal_paso == 1 && Serial.available() > 0) {
            char c = (char)Serial.read();
            vaciarSerial();
            if (c == '\r' || c == '\n') return;
            if (c == '0') { modoActivo = 0; imprimirMenu(); return; }
            int humedo = hum_leerADC();
            Serial.print("  ADC humedo: "); Serial.println(humedo);
            hum_guardarCalibracion(humCal_seco, humedo);
            Serial.println("=========================================");
            Serial.println("  ✔ Calibracion guardada en EEPROM.");
            Serial.print("  Seco="); Serial.print(humCal_seco);
            Serial.print("  Humedo="); Serial.println(humedo);
            Serial.println("=========================================");
            humCal_paso = 0; modoActivo = 0; imprimirMenu();
        }
        return;
    }

    // ── Modo 8: distancia HC-SR04 ─────────────────────────────
    if (modoActivo == 8) {
        if (Serial.available() > 0 && Serial.peek() == '0') {
            while (Serial.available() > 0) Serial.read();
            modoActivo = 0; _buf = ""; imprimirMenu(); return;
        }
        float distancia = ultra_leerDistanciaCM();
        if (Serial.available() > 0 && Serial.peek() == '0') {
            while (Serial.available() > 0) Serial.read();
            modoActivo = 0; _buf = ""; imprimirMenu(); return;
        }
        if (distancia == ULTRA_ERROR) Serial.println("ERROR: fuera de rango o sin eco.");
        else { Serial.print("Distancia: "); Serial.print(distancia, 1); Serial.println(" cm"); }
        for (int i = 0; i < 10; i++) {
            delay(100);
            if (Serial.available() > 0 && Serial.peek() == '0') {
                while (Serial.available() > 0) Serial.read();
                modoActivo = 0; _buf = ""; imprimirMenu(); return;
            }
        }
        return;
    }

    // ── Modo 9: escaneo de los 4 conectores ───────────────────
    if (modoActivo == 9) {
        if (Serial.available() > 0 && Serial.peek() == '0') {
            while (Serial.available() > 0) Serial.read();
            modoActivo = 0; _buf = ""; imprimirMenu(); return;
        }

        Serial.println("-----------------------------------------");
        for (int i = 0; i < NUM_CONECTORES; i++) {
            leerYMostrarConector(i + 1, conectores[i]);
            if (Serial.available() > 0 && Serial.peek() == '0') {
                while (Serial.available() > 0) Serial.read();
                modoActivo = 0; _buf = ""; imprimirMenu(); return;
            }
        }

        for (int i = 0; i < 10; i++) {
            delay(100);
            if (Serial.available() > 0 && Serial.peek() == '0') {
                while (Serial.available() > 0) Serial.read();
                modoActivo = 0; _buf = ""; imprimirMenu(); return;
            }
        }
        return;
    }
}
