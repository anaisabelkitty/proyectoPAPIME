// ─────────────────────────────────────────────────────────────
//  AJUSTE DEL POTENCIÓMETRO DE OFFSET — Sensor pH PH-4502C
//  Arduino Mega 2560 | PlatformIO
//
//  Este sketch sirve para ajustar POT2 (el potenciómetro azul
//  MÁS CERCANO al conector BNC) hasta que la salida Po mida 2.5V
//  cuando no hay sonda conectada o el BNC está en cortocircuito.
//
//  NOTA: En algunos tutoriales este pot aparece marcado como POT1,
//  pero en nuestro módulo físico verificamos que el pot que afecta
//  la lectura analógica de Po es el MÁS CERCANO al BNC (POT2 según
//  el pinout de UNIT Electronics).
//
//  Hay que repetir este procedimiento si alguien movió POT2.
//  POT1 (el otro, más lejano al BNC) controla la salida digital DO
//  y NO afecta la lectura de pH — no lo toques para esta calibración.
//
//  PROCEDIMIENTO:
//    1. Desconecta la sonda del BNC
//    2. Haz un puente entre el pin central y la carcasa del BNC
//       (usa un trozo de cable pelado)
//    3. Conecta: VCC→5V | GND→GND | Po→A2
//    4. Sube este archivo como src/main.cpp (guarda el original)
//    5. Abre el Serial Monitor a 9600 baudios
//    6. Gira POT2 (el MÁS CERCANO al BNC) hasta ver "✔ CORRECTO"
//    7. Retira el puente y vuelve a conectar la sonda
//
//  CONEXIONES:
//    VCC del módulo → 5V del Arduino Mega
//    GND del módulo → GND del Arduino Mega
//    Po  del módulo → A2 del Arduino Mega
// ─────────────────────────────────────────────────────────────

#include <Arduino.h>

const int   PH_PIN   = A2;
const float VREF     = 5.0;
const int   ADC_R    = 1024;
const int   MUESTRAS = 20;

float leerVoltaje() {
    long suma = 0;
    for (int i = 0; i < MUESTRAS; i++) {
        suma += analogRead(PH_PIN);
        delay(10);
    }
    float promedio = (float)suma / MUESTRAS;
    return promedio * VREF / ADC_R;
}

void setup() {
    Serial.begin(9600);
    pinMode(PH_PIN, INPUT);
    Serial.println("=========================================");
    Serial.println("  AJUSTE DE OFFSET — Modulo pH PH-4502C");
    Serial.println("=========================================");
    Serial.println();
    Serial.println("  Gira POT2 (el MAS CERCANO al BNC)");
    Serial.println("  hasta obtener exactamente 2.50V.");
    Serial.println();
}

void loop() {
    float voltaje = leerVoltaje();
    Serial.print("Voltaje Po: ");
    Serial.print(voltaje, 3);
    Serial.print(" V  →  ");

    if      (voltaje < 2.3)                        Serial.println("⬆ Sube POT2");
    else if (voltaje >= 2.3  && voltaje < 2.48)    Serial.println("⬆ Sube un poco POT2");
    else if (voltaje >= 2.48 && voltaje <= 2.52)   Serial.println("✔ CORRECTO  (POT2 listo)");
    else if (voltaje >  2.52 && voltaje <= 2.7)    Serial.println("⬇ Baja un poco POT2");
    else                                           Serial.println("⬇ Baja POT2");

    delay(1000);
}
