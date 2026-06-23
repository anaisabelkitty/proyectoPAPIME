// ─────────────────────────────────────────────────────────────
//  CALIBRACIÓN CON SOLUCIONES BUFFER — Sensor pH PH-4502C
//  Arduino Mega 2560 | PlatformIO
//
//  Siempre calibra con DOS puntos (Lagrange grado 1).
//  El usuario elige qué par de buffers usar según lo que va a medir:
//
//    A → pH 4.01 + pH 7.00   Muestras ácidas a neutras
//                            (suelo, cultivos, agua, hidroponía)
//
//    B → pH 7.00 + pH 10.01  Muestras neutras a alcalinas
//                            (agua tratada, soluciones básicas)
//
//  USO:
//    1. Copia este archivo como src/main.cpp (guarda el original)
//    2. Sube al Arduino y abre el Serial Monitor a 9600 baudios
//    3. Escribe A o B y sigue las instrucciones
//    4. Copia los dos valores resultantes a ph.h
//
//  CONEXIONES:
//    VCC del módulo  →  5V del Arduino Mega
//    GND del módulo  →  GND del Arduino Mega
//    Po  del módulo  →  A2 del Arduino Mega
//    Electrodo E201C-BNC  →  conector BNC del módulo
// ─────────────────────────────────────────────────────────────

#include <Arduino.h>

// --- Constantes del ADC ---
const int   PIN_SENSOR = A2;
const float VREF       = 5.0;
const float ADC_MAX    = 1024.0;
const int   MUESTRAS   = 30;   // Promedio de 30 lecturas para estabilidad

// --- Valores de pH de las soluciones buffer ---
const float PH_4  =  4.01;
const float PH_7  =  7.00;
const float PH_10 = 10.01;

// --- Estado interno ---
int modo = 0;   // 0=esperando elección  1=pH4+pH7  2=pH7+pH10

// ─────────────────────────────────────────────────────────────
//  Utilidades
// ─────────────────────────────────────────────────────────────

// Lee el voltaje promediando MUESTRAS lecturas con 20 ms entre cada una
float leerVoltajePromedio() {
    long suma = 0;
    for (int i = 0; i < MUESTRAS; i++) {
        suma += analogRead(PIN_SENSOR);
        delay(20);
    }
    return ((float)suma / MUESTRAS) * VREF / ADC_MAX;
}

// Espera cualquier carácter por Serial, devuelve en minúscula y vacía el buffer
char esperarCaracter() {
    while (Serial.available() == 0) delay(100);
    char c = (char)Serial.read();
    delay(10);
    while (Serial.available() > 0) Serial.read();
    return tolower(c);
}

// Muestra instrucciones, espera confirmación y devuelve el voltaje medido
float pedirMedicion(float ph_buffer, int paso, int total) {
    Serial.println();
    Serial.print("--- PASO ");
    Serial.print(paso);
    Serial.print(" de ");
    Serial.print(total);
    Serial.print(" --- Buffer pH ");
    Serial.println(ph_buffer, 2);

    if (paso > 1) {
        Serial.println("    Enjuaga bien la sonda con agua destilada.");
    }
    Serial.print("    Sumerge la sonda en la solucion pH ");
    Serial.print(ph_buffer, 2);
    Serial.println(".");
    Serial.println("    Espera 2 minutos hasta que se estabilice.");
    Serial.println("    Luego presiona ENTER (o cualquier tecla).");

    esperarCaracter();

    Serial.print("    Leyendo");
    for (int i = 0; i < 3; i++) { delay(300); Serial.print("."); }
    float v = leerVoltajePromedio();
    Serial.print("  Voltaje: ");
    Serial.print(v, 4);
    Serial.println(" V  OK");
    return v;
}

// ─────────────────────────────────────────────────────────────
//  Cálculo de Lagrange grado 1 e impresión de resultados
//
//  Dos puntos: (v_a, ph_a) y (v_b, ph_b)
//  Polinomio:  pH = ph_a*(v-v_b)/(v_a-v_b) + ph_b*(v-v_a)/(v_b-v_a)
//
//  Convertido a la forma que usa ph.h:
//    pH = 7 + (PH_OFFSET_PH7 - voltaje) / PH_SENSIBILIDAD
//  donde:
//    PH_OFFSET_PH7   = voltaje medido cuando pH = 7 (= v_7)
//    PH_SENSIBILIDAD = (v_b - v_a) / (ph_a - ph_b)  [positivo]
// ─────────────────────────────────────────────────────────────
void calcularEImprimir(float v_a, float ph_a,
                       float v_b, float ph_b,
                       float v_7) {
    // Sensibilidad: cuántos voltios cambia por cada unidad de pH
    // La relación es inversa (más pH → menos voltaje), por eso
    // se usa (ph_a - ph_b) en el denominador para que quede positivo
    float sensibilidad = (v_b - v_a) / (ph_a - ph_b);

    // Offset: es simplemente el voltaje que midió cuando pH = 7
    float offset = v_7;

    // Verificar que el polinomio pasa exactamente por los dos puntos
    float ph_calc_a = 7.0 + (offset - v_a) / sensibilidad;
    float ph_calc_b = 7.0 + (offset - v_b) / sensibilidad;

    Serial.println();
    Serial.println("=========================================");
    Serial.println("  RESULTADOS DE CALIBRACION");
    Serial.println("=========================================");
    Serial.println();
    Serial.println("  Copia estos valores en ph.h:");
    Serial.println();
    Serial.print("    const float PH_OFFSET_PH7   = ");
    Serial.print(offset, 4);
    Serial.println(";");
    Serial.print("    const float PH_SENSIBILIDAD = ");
    Serial.print(sensibilidad, 4);
    Serial.println(";");
    Serial.println();
    Serial.println("  Verificacion (deben coincidir con los buffers):");
    Serial.print("    Buffer pH ");
    Serial.print(ph_a, 2);
    Serial.print("  →  calcula pH: ");
    Serial.println(ph_calc_a, 2);
    Serial.print("    Buffer pH ");
    Serial.print(ph_b, 2);
    Serial.print("  →  calcula pH: ");
    Serial.println(ph_calc_b, 2);
}

// ─────────────────────────────────────────────────────────────
//  Setup: menú de selección
// ─────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(9600);
    pinMode(PIN_SENSOR, INPUT);
    delay(500);

    Serial.println("=========================================");
    Serial.println("  CALIBRACION — Sensor pH PH-4502C");
    Serial.println("=========================================");
    Serial.println();
    Serial.println("Elige los buffers segun lo que vas a medir:");
    Serial.println();
    Serial.println("  A → pH 4.01 + pH 7.00");
    Serial.println("      Para muestras acidas a neutras");
    Serial.println("      (suelo, cultivos, agua, hidroponia)");
    Serial.println();
    Serial.println("  B → pH 7.00 + pH 10.01");
    Serial.println("      Para muestras neutras a alcalinas");
    Serial.println("      (agua tratada, soluciones basicas)");
    Serial.println();
    Serial.println("Escribe A o B y presiona ENTER:");
}

// ─────────────────────────────────────────────────────────────
//  Loop: ejecuta la calibración según el modo elegido
// ─────────────────────────────────────────────────────────────
void loop() {
    if (modo != 0) return;   // Ya terminó, no repetir

    if (Serial.available() == 0) return;

    char opcion = esperarCaracter();

    if (opcion == 'a') {
        modo = 1;
        Serial.println();
        Serial.println(">>> Modo A: pH 4.01 + pH 7.00");

    } else if (opcion == 'b') {
        modo = 2;
        Serial.println();
        Serial.println(">>> Modo B: pH 7.00 + pH 10.01");

    } else {
        Serial.println("Opcion no valida. Escribe A o B:");
        return;
    }

    // ── PASO 1: siempre pH 7 primero (es el offset central) ──
    float v7 = pedirMedicion(PH_7, 1, 2);

    // ── PASO 2: el segundo buffer según el modo ───────────────
    float v2, ph2;
    if (modo == 1) {
        v2  = pedirMedicion(PH_4, 2, 2);
        ph2 = PH_4;
        calcularEImprimir(v7, PH_7, v2, ph2, v7);
    } else {
        v2  = pedirMedicion(PH_10, 2, 2);
        ph2 = PH_10;
        calcularEImprimir(v7, PH_7, v2, ph2, v7);
    }

    Serial.println();
    Serial.println("=========================================");
    Serial.println("  Calibracion completada.");
    Serial.println("  Reinicia el Arduino para repetir.");
    Serial.println("=========================================");

    // Detener hasta que el usuario reinicie
    while (true) delay(10000);
}
