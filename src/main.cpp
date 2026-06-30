// Kit Sensei — Menú principal
//
// Opciones:
//   1 → Leer pH en tiempo real           (sensor PH-4502C, pin A2)
//   2 → Leer temperatura en tiempo real  (sensor DS18B20, pin 7)
//   3 → Calibración de offset BNC        (ajustar POT2 del módulo pH)
//   4 → Calibración con buffers          (Lagrange — actualiza EEPROM)
//   5 → Leer humedad de suelo            (sensor OKY3442, pin A4)
//   0 → Volver al menú

#include <Arduino.h>
#include "sensores_analogicos/ph/ph.h"
#include "sensores_digitales/temperatura_ds18b20/temperatura.h"
#include "sensores_analogicos/humedad_suelo/humedad.h"

// ─── Constantes de calibración buffer ───────────────────────
const float CAL_PH_4  =  4.01f;
const float CAL_PH_7  =  7.00f;
const float CAL_PH_10 = 10.01f;
const int   CAL_MUESTRAS = 30;

// ─── Estado global ───────────────────────────────────────────
int modoActivo   = 0;
int modoCal      = 0;   // 0=esperando  1=pH4+pH7  2=pH7+pH10
bool calEnCurso  = false;
float cal_v7     = 0.0f;
float cal_v2     = 0.0f;
float cal_ph2    = 0.0f;
int   cal_paso   = 0;   // 0=inicial  1=midiendo pH7  2=midiendo 2° buffer

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
    Serial.println();
    Serial.print("    PH_OFFSET_PH7   = "); Serial.println(offset, 4);
    Serial.print("    PH_SENSIBILIDAD = "); Serial.println(sensibilidad, 4);
    Serial.println();
    Serial.println("  Verificacion:");
    Serial.print("    Buffer pH "); Serial.print(ph_a, 2);
    Serial.print("  →  calcula pH: "); Serial.println(ph_calc_a, 2);
    Serial.print("    Buffer pH "); Serial.print(ph_b, 2);
    Serial.print("  →  calcula pH: "); Serial.println(ph_calc_b, 2);
    Serial.println();

    // Guardar en EEPROM y activar inmediatamente
    ph_guardarCalibracion(offset, sensibilidad);

    Serial.println("  ✔ Valores guardados en EEPROM.");
    Serial.println("  El modo 1 (leer pH) ya usa estos valores.");
    Serial.println("=========================================");
    Serial.println("  Calibracion completada.");
    Serial.println("  Escribe 0 para volver al menu.");
}

// ─── Menú ────────────────────────────────────────────────────

void imprimirMenu() {
    Serial.println();
    Serial.println("=========================================");
    Serial.println("  Kit Sensei");
    Serial.println("=========================================");
    Serial.print("  pH calibracion: offset=");
    Serial.print(ph_offset, 4);
    Serial.print("  sens=");
    Serial.println(ph_sensibilidad, 4);
    if (ph_tieneCalibacionGuardada()) {
        Serial.println("  (calibracion guardada en EEPROM)");
    } else {
        Serial.println("  (valores por defecto)");
    }
    Serial.println("-----------------------------------------");
    Serial.println("  1  →  Leer pH en tiempo real");
    Serial.println("  2  →  Leer temperatura en tiempo real");
    Serial.println("  3  →  Calibrar offset BNC (POT2)");
    Serial.println("  4  →  Calibrar con soluciones buffer");
    Serial.println("  5  →  Leer humedad de suelo");
    Serial.println("  0  →  Volver al menu");
    Serial.println("-----------------------------------------");
    Serial.println("Escribe el numero y presiona ENTER:");
}

// ─── Setup ───────────────────────────────────────────────────

void setup() {
    Serial.begin(9600);
    pinMode(PH_PIN_SENSOR, INPUT);
    ph_inicializar();       // Carga calibración de EEPROM (si existe)
    temp_inicializar();
    imprimirMenu();
}

// ─── Loop ────────────────────────────────────────────────────

void loop() {

    // ── Lectura de opción del menú principal ──────────────────
    if (modoActivo == 0 && !calEnCurso && Serial.available() > 0) {
        int opcion = Serial.parseInt();
        while (Serial.available() > 0) Serial.read();

        if      (opcion == 1) { modoActivo = 1; Serial.println("\n>> Leyendo pH (0 para volver)"); }
        else if (opcion == 2) { modoActivo = 2; Serial.println("\n>> Leyendo temperatura (0 para volver)"); }
        else if (opcion == 3) {
            modoActivo = 3;
            Serial.println();
            Serial.println("=========================================");
            Serial.println("  AJUSTE DE OFFSET — POT2 (mas cercano al BNC)");
            Serial.println("=========================================");
            Serial.println("  1. Desconecta el electrodo del BNC.");
            Serial.println("  2. Cortocircuita el BNC con un cable.");
            Serial.println("  3. Gira POT2 hasta ver: CORRECTO");
            Serial.println("  Escribe 0 para salir.");
            Serial.println();
        }
        else if (opcion == 4) {
            modoActivo  = 4;
            calEnCurso  = false;
            modoCal     = 0;
            cal_paso    = 0;
            Serial.println();
            Serial.println("=========================================");
            Serial.println("  CALIBRACION CON SOLUCIONES BUFFER");
            Serial.println("=========================================");
            Serial.println("  Elige los buffers segun lo que vas a medir:");
            Serial.println("  A → pH 4.01 + pH 7.00  (muestras acidas)");
            Serial.println("  B → pH 7.00 + pH 10.01 (muestras alcalinas)");
            Serial.println("  Escribe A o B y presiona ENTER:");
        }
        else if (opcion == 5) {
            modoActivo = 5;
            Serial.println();
            Serial.println(">> Leyendo humedad de suelo OKY3442 (0 para volver)");
        }
        else if (opcion == 0) { modoActivo = 0; imprimirMenu(); }
        else { Serial.println("Opcion no valida."); imprimirMenu(); }
    }

    // ── Modo 1: leer pH ───────────────────────────────────────
    if (modoActivo == 1) {
        if (Serial.available() > 0) {
            int v = Serial.parseInt();
            while (Serial.available() > 0) Serial.read();
            if (v == 0) { modoActivo = 0; imprimirMenu(); return; }
        }
        float voltaje = ph_leerVoltaje();
        float pH      = ph_calcularPH(voltaje);
        Serial.print("Voltaje: "); Serial.print(voltaje, 3);
        Serial.print(" V  |  pH: "); Serial.println(pH, 2);
        delay(1000);
    }

    // ── Modo 2: leer temperatura ──────────────────────────────
    else if (modoActivo == 2) {
        if (Serial.available() > 0) {
            int v = Serial.parseInt();
            while (Serial.available() > 0) Serial.read();
            if (v == 0) { modoActivo = 0; imprimirMenu(); return; }
        }
        float t = temp_leerCelsius();
        if (t == TEMP_ERROR) Serial.println("ERROR: DS18B20 desconectado.");
        else { Serial.print("Temperatura: "); Serial.print(t, 2); Serial.println(" °C"); }
        delay(1000);
    }

    // ── Modo 3: calibración offset BNC ───────────────────────
    else if (modoActivo == 3) {
        if (Serial.available() > 0) {
            int v = Serial.parseInt();
            while (Serial.available() > 0) Serial.read();
            if (v == 0) { modoActivo = 0; imprimirMenu(); return; }
        }
        long suma = 0;
        for (int i = 0; i < 20; i++) { suma += analogRead(PH_PIN_SENSOR); delay(10); }
        float voltaje = ((float)suma / 20.0) * PH_VREF / PH_ADC_MAX;
        Serial.print("Voltaje Po: "); Serial.print(voltaje, 3); Serial.print(" V  →  ");
        if      (voltaje < 2.30)                      Serial.println("subir POT2");
        else if (voltaje >= 2.30 && voltaje < 2.48)   Serial.println("subir un poco POT2");
        else if (voltaje >= 2.48 && voltaje <= 2.52)  Serial.println("✔ CORRECTO  (POT2 listo)");
        else if (voltaje > 2.52  && voltaje <= 2.70)  Serial.println("bajar un poco POT2");
        else                                          Serial.println("bajar POT2");
        delay(1000);
    }

    // ── Modo 4: calibración con buffers ──────────────────────
    else if (modoActivo == 4) {

        // Paso 0: esperar A o B
        if (cal_paso == 0 && Serial.available() > 0) {
            char c = (char)Serial.read();
            while (Serial.available() > 0) Serial.read();
            c = tolower(c);

            if (c == 'a') {
                modoCal  = 1; cal_paso = 1;
                Serial.println(">>> Modo A: pH 4.01 + pH 7.00");
            } else if (c == 'b') {
                modoCal  = 2; cal_paso = 1;
                Serial.println(">>> Modo B: pH 7.00 + pH 10.01");
            } else if (c == '0') {
                modoActivo = 0; imprimirMenu(); return;
            } else {
                Serial.println("Opcion no valida. Escribe A, B o 0 para salir:");
                return;
            }

            Serial.println();
            Serial.println("--- PASO 1 de 2 --- Buffer pH 7.00");
            Serial.println("  Sumerge la sonda en la solucion pH 7.00.");
            Serial.println("  Espera 2 minutos y luego presiona ENTER:");
        }

        // Paso 1: medir pH 7
        else if (cal_paso == 1 && Serial.available() > 0) {
            while (Serial.available() > 0) Serial.read();
            Serial.print("  Leyendo");
            for (int i = 0; i < 3; i++) { delay(300); Serial.print("."); }
            cal_v7 = cal_leerVoltajePromedio();
            Serial.print("  Voltaje pH7: "); Serial.print(cal_v7, 4); Serial.println(" V  OK");
            cal_paso = 2;

            float ph2 = (modoCal == 1) ? CAL_PH_4 : CAL_PH_10;
            Serial.println();
            Serial.print("--- PASO 2 de 2 --- Buffer pH ");
            Serial.println(ph2, 2);
            Serial.println("  Enjuaga la sonda con agua destilada.");
            Serial.print("  Sumerge en la solucion pH "); Serial.print(ph2, 2); Serial.println(".");
            Serial.println("  Espera 2 minutos y luego presiona ENTER:");
        }

        // Paso 2: medir 2° buffer y calcular
        else if (cal_paso == 2 && Serial.available() > 0) {
            while (Serial.available() > 0) Serial.read();
            Serial.print("  Leyendo");
            for (int i = 0; i < 3; i++) { delay(300); Serial.print("."); }
            cal_v2  = cal_leerVoltajePromedio();
            cal_ph2 = (modoCal == 1) ? CAL_PH_4 : CAL_PH_10;
            Serial.print("  Voltaje: "); Serial.print(cal_v2, 4); Serial.println(" V  OK");

            cal_imprimirResultados(cal_v7, CAL_PH_7, cal_v2, cal_ph2, cal_v7);

            cal_paso   = 0;
            modoCal    = 0;
            modoActivo = 0;
            imprimirMenu();
        }
    }

    // ── Modo 5: leer humedad de suelo ────────────────────────
    else if (modoActivo == 5) {
        if (Serial.available() > 0) {
            int v = Serial.parseInt();
            while (Serial.available() > 0) Serial.read();
            if (v == 0) { modoActivo = 0; imprimirMenu(); return; }
        }
        int   adc     = hum_leerADC();
        float humedad = hum_calcularHumedad(adc);
        Serial.print("Humedad: ");
        Serial.print(humedad, 1);
        Serial.print("%  (ADC: ");
        Serial.print(adc);
        Serial.println(")");
        delay(1000);
    }
}
