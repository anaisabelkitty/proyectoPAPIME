// ─────────────────────────────────────────────────────────────
//  VERIFICACIÓN Y CALIBRACIÓN — Sensor DS18B20 (sumergible)
//  Arduino Mega 2560 | PlatformIO
//
//  El DS18B20 sale calibrado de fábrica (±0.5 °C).
//  Este sketch sirve para:
//    1. Confirmar que el sensor está bien conectado y responde.
//    2. Medir el offset real comparando contra una referencia
//       (termómetro de mercurio o hielo/agua hirviendo).
//    3. Si hay un offset medible, calcular el valor de corrección
//       para agregarlo en temperatura.h como TEMP_OFFSET_CORRECCIÓN.
//
//  USO:
//    1. Copia este archivo como src/main.cpp (guarda el original).
//    2. Sube al Arduino y abre el Serial Monitor a 9600 baudios.
//    3. Sigue las instrucciones que aparecen en pantalla.
//    4. Si el offset es significativo (> 0.5 °C), anótalo en temperatura.h.
//
//  CONEXIONES:
//    Cable rojo    (VCC) → 5V del Arduino Mega
//    Cable negro   (GND) → GND del Arduino Mega
//    Cable amarillo (DQ) → Pin digital 7 del Arduino Mega
//    Resistencia 4.7 kΩ  → entre DQ y 5V
// ─────────────────────────────────────────────────────────────

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// --- Constantes ---
const int PIN_SENSOR  = 7;
const int MUESTRAS    = 20;    // Promedio de 20 lecturas para estabilidad

// --- Objetos del bus 1-Wire ---
OneWire          onewire(PIN_SENSOR);
DallasTemperature sensores(&onewire);

// ─────────────────────────────────────────────────────────────
//  Utilidades
// ─────────────────────────────────────────────────────────────

// Lee MUESTRAS temperaturas consecutivas y devuelve el promedio.
// Descarta lecturas de error (-127 °C).
float leerTemperaturaPromedio() {
    float suma  = 0.0;
    int   validas = 0;

    for (int i = 0; i < MUESTRAS; i++) {
        sensores.requestTemperatures();
        float t = sensores.getTempCByIndex(0);

        if (t != DEVICE_DISCONNECTED_C) {
            suma += t;
            validas++;
        }
        delay(100);  // ~750 ms de conversión + margen
    }

    if (validas == 0) return DEVICE_DISCONNECTED_C;
    return suma / validas;
}

// Espera cualquier carácter por Serial y vacía el buffer
void esperarEnter() {
    while (Serial.available() == 0) delay(100);
    while (Serial.available() > 0)  Serial.read();
}

// ─────────────────────────────────────────────────────────────
//  Setup
// ─────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(9600);
    sensores.begin();
    sensores.setResolution(12);  // Máxima resolución: 0.0625 °C
    delay(500);

    Serial.println("=========================================");
    Serial.println("  VERIFICACION — Sensor DS18B20");
    Serial.println("=========================================");
    Serial.println();

    // --- Verificar que el sensor responde ---
    Serial.print("Buscando sensor en pin ");
    Serial.print(PIN_SENSOR);
    Serial.println("...");

    int count = sensores.getDeviceCount();
    if (count == 0) {
        Serial.println();
        Serial.println("ERROR: No se encontro ningun sensor DS18B20.");
        Serial.println("Verifica la conexion y la resistencia de 4.7 kOhm.");
        while (true) delay(1000);
    }

    Serial.print("Sensores encontrados en el bus 1-Wire: ");
    Serial.println(count);
    Serial.println();

    // --- Lectura continua para estabilización ---
    Serial.println("Leyendo temperatura cada segundo...");
    Serial.println("Presiona ENTER para iniciar la calibracion.");
    Serial.println();
}

// ─────────────────────────────────────────────────────────────
//  Loop: lectura continua y luego calibración
// ─────────────────────────────────────────────────────────────
bool calibrando = false;

void loop() {
    if (!calibrando) {
        // Modo lectura continua hasta que el usuario presione ENTER
        sensores.requestTemperatures();
        float t = sensores.getTempCByIndex(0);

        if (t == DEVICE_DISCONNECTED_C) {
            Serial.println("ERROR: sensor desconectado.");
        } else {
            Serial.print("Temperatura: ");
            Serial.print(t, 4);
            Serial.println(" °C");
        }

        // Verificar si llegó algo por Serial
        if (Serial.available() > 0) {
            while (Serial.available() > 0) Serial.read();
            calibrando = true;
            Serial.println();
            Serial.println("=========================================");
            Serial.println("  CALIBRACION CON REFERENCIA EXTERNA");
            Serial.println("=========================================");
            Serial.println();
            Serial.println("Paso 1: Coloca el sensor junto a tu termometro");
            Serial.println("        de referencia en el mismo liquido.");
            Serial.println("        Espera 2 minutos para estabilizar.");
            Serial.println("        Luego presiona ENTER.");
        }

        delay(1000);
        return;
    }

    // --- Modo calibración ---
    esperarEnter();

    Serial.println();
    Serial.print("Promediando ");
    Serial.print(MUESTRAS);
    Serial.println(" lecturas...");

    float t_sensor = leerTemperaturaPromedio();

    if (t_sensor == DEVICE_DISCONNECTED_C) {
        Serial.println("ERROR: sensor desconectado durante la calibracion.");
        calibrando = false;
        return;
    }

    Serial.print("Temperatura medida por el DS18B20: ");
    Serial.print(t_sensor, 4);
    Serial.println(" °C");
    Serial.println();
    Serial.println("Paso 2: Escribe la temperatura de tu referencia");
    Serial.println("        (en grados Celsius, por ejemplo: 25.50)");
    Serial.println("        y presiona ENTER:");

    // Esperar y leer el valor de referencia
    while (Serial.available() == 0) delay(100);
    float t_referencia = Serial.parseFloat();
    while (Serial.available() > 0) Serial.read();

    float offset = t_referencia - t_sensor;

    Serial.println();
    Serial.println("=========================================");
    Serial.println("  RESULTADO");
    Serial.println("=========================================");
    Serial.println();
    Serial.print("  Temperatura sensor:     ");
    Serial.print(t_sensor, 4);
    Serial.println(" °C");
    Serial.print("  Temperatura referencia: ");
    Serial.print(t_referencia, 2);
    Serial.println(" °C");
    Serial.print("  Offset calculado:       ");
    Serial.print(offset, 4);
    Serial.println(" °C");
    Serial.println();

    if (abs(offset) <= 0.5) {
        Serial.println("El offset esta dentro del error de fabrica (±0.5 °C).");
        Serial.println("No es necesario agregar corrección en temperatura.h.");
    } else {
        Serial.println("El offset supera ±0.5 °C. Considera agregar en temperatura.h:");
        Serial.println();
        Serial.print("  const float TEMP_OFFSET_CORRECCION = ");
        Serial.print(offset, 4);
        Serial.println(";");
        Serial.println();
        Serial.println("Y aplicarlo en temp_leerCelsius():");
        Serial.println("  return temperatura + TEMP_OFFSET_CORRECCION;");
    }

    Serial.println();
    Serial.println("=========================================");
    Serial.println("  Verificacion completada.");
    Serial.println("  Reinicia el Arduino para repetir.");
    Serial.println("=========================================");

    while (true) delay(10000);
}
