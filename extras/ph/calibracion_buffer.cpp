// ─────────────────────────────────────────────────────────────
//  CALIBRACIÓN CON SOLUCIONES BUFFER — Sensor pH PH-4502C
//  Arduino Mega 2560 | PlatformIO
//
//  USO:
//    1. Sube este archivo como src/main.cpp (renombra el original)
//    2. Abre el Serial Monitor a 9600 baudios
//    3. Sigue las instrucciones en pantalla
//    4. Al terminar, copia los valores a src/sensores_analogicos/ph/ph.h
//
//  CONEXIONES:
//    VCC del módulo  →  5V del Arduino Mega
//    GND del módulo  →  GND del Arduino Mega
//    Po  del módulo  →  A2 del Arduino Mega
//    Electrodo E201  →  conector BNC del módulo
// ─────────────────────────────────────────────────────────────

#include <Arduino.h>

// --- Constantes del ADC ---
const int   PIN_SENSOR = A2;
const float VREF       = 5.0;
const float ADC_MAX    = 1024.0;
const int   MUESTRAS   = 30;      // Promedio de 30 lecturas para estabilidad

// --- pH conocidos de las soluciones buffer ---
const float PH_BUFFER_1 = 7.00;  // Primera solución (neutra)
const float PH_BUFFER_2 = 4.01;  // Segunda solución (ácida)

// --- Variables de calibración ---
float voltaje_buffer1 = 0.0;
float voltaje_buffer2 = 0.0;

// Lee el voltaje promediando N muestras
float leerVoltajePromedio() {
    long suma = 0;
    for (int i = 0; i < MUESTRAS; i++) {
        suma += analogRead(PIN_SENSOR);
        delay(20);
    }
    float promedio = (float)suma / MUESTRAS;
    return promedio * VREF / ADC_MAX;
}

// Espera a que el usuario envíe cualquier carácter por serial
void esperarEnter() {
    while (Serial.available() == 0) {
        delay(100);
    }
    // Vaciar el buffer
    while (Serial.available() > 0) {
        Serial.read();
    }
}

void setup() {
    Serial.begin(9600);
    pinMode(PIN_SENSOR, INPUT);

    delay(500);

    Serial.println("=========================================");
    Serial.println("  CALIBRACION CON SOLUCIONES BUFFER");
    Serial.println("  Sensor pH PH-4502C");
    Serial.println("=========================================");
    Serial.println();
    Serial.println("Este proceso calculara los valores reales");
    Serial.println("de OFFSET y SENSIBILIDAD para tu modulo.");
    Serial.println();
    Serial.println("Necesitas:");
    Serial.println("  - Solucion buffer pH 7.00");
    Serial.println("  - Solucion buffer pH 4.01");
    Serial.println("  - La sonda limpia y seca entre mediciones");
    Serial.println();
    Serial.println("-----------------------------------------");
    Serial.println("PASO 1: Sumerge la sonda en buffer pH 7");
    Serial.println("Espera 2 minutos y luego envia cualquier");
    Serial.println("caracter por el Serial Monitor.");
    Serial.println("-----------------------------------------");
}

void loop() {
    // --- PASO 1: Medir buffer pH 7 ---
    Serial.println("\nEsperando... (envia cualquier caracter cuando este listo)");
    esperarEnter();

    Serial.println("Leyendo voltaje en buffer pH 7.00...");
    voltaje_buffer1 = leerVoltajePromedio();

    Serial.print("  Voltaje medido: ");
    Serial.print(voltaje_buffer1, 4);
    Serial.println(" V");
    Serial.print("  pH esperado:    ");
    Serial.println(PH_BUFFER_1);

    // --- PASO 2: Medir buffer pH 4 ---
    Serial.println();
    Serial.println("-----------------------------------------");
    Serial.println("PASO 2: Enjuaga la sonda con agua destilada");
    Serial.println("luego sumergela en buffer pH 4.01");
    Serial.println("Espera 2 minutos y luego envia cualquier");
    Serial.println("caracter por el Serial Monitor.");
    Serial.println("-----------------------------------------");
    Serial.println("\nEsperando... (envia cualquier caracter cuando este listo)");
    esperarEnter();

    Serial.println("Leyendo voltaje en buffer pH 4.01...");
    voltaje_buffer2 = leerVoltajePromedio();

    Serial.print("  Voltaje medido: ");
    Serial.print(voltaje_buffer2, 4);
    Serial.println(" V");
    Serial.print("  pH esperado:    ");
    Serial.println(PH_BUFFER_2);

    // --- CÁLCULO DE LAGRANGE ---
    // Puntos: (voltaje_buffer1, PH_BUFFER_1) y (voltaje_buffer2, PH_BUFFER_2)
    // Sensibilidad real = (pH2 - pH1) / (voltaje2 - voltaje1)
    // Como la relación es inversa: sensibilidad = (pH1 - pH2) / (voltaje2 - voltaje1)
    float delta_voltaje = voltaje_buffer2 - voltaje_buffer1;
    float delta_ph      = PH_BUFFER_1 - PH_BUFFER_2;  // 7 - 4 = 3
    float sensibilidad_real = delta_ph / delta_voltaje; // negativo porque relacion inversa

    // Offset real: voltaje donde el pH es 7
    // De la formula pH = 7 + (offset - Vout) / sensibilidad
    // offset = voltaje_buffer1 (el voltaje medido cuando pH = 7)
    float offset_real = voltaje_buffer1;

    // --- RESULTADOS ---
    Serial.println();
    Serial.println("=========================================");
    Serial.println("  RESULTADOS DE CALIBRACION");
    Serial.println("=========================================");
    Serial.println();
    Serial.println("Copia estos valores en ph.h:");
    Serial.println();
    Serial.print("  const float PH_OFFSET_PH7   = ");
    Serial.print(offset_real, 4);
    Serial.println(";");
    Serial.print("  const float PH_SENSIBILIDAD = ");
    // La sensibilidad debe ser positiva en la formula
    // pH = 7 + (offset - Vout) / sensibilidad
    // sensibilidad = delta_voltaje / delta_ph * (-1)
    float sens_formula = (voltaje_buffer2 - voltaje_buffer1) / (PH_BUFFER_1 - PH_BUFFER_2);
    Serial.print(sens_formula, 4);
    Serial.println(";");

    Serial.println();
    Serial.println("Verificacion con los puntos medidos:");
    float ph_verificacion1 = 7.0 + (offset_real - voltaje_buffer1) / sens_formula;
    float ph_verificacion2 = 7.0 + (offset_real - voltaje_buffer2) / sens_formula;
    Serial.print("  Buffer pH 7 → calcula: ");
    Serial.println(ph_verificacion1, 2);
    Serial.print("  Buffer pH 4 → calcula: ");
    Serial.println(ph_verificacion2, 2);

    Serial.println();
    Serial.println("=========================================");
    Serial.println("  Calibracion completada.");
    Serial.println("  Reinicia el Arduino para repetir.");
    Serial.println("=========================================");

    // No repetir — esperar reinicio
    while (true) {
        delay(10000);
    }
}
