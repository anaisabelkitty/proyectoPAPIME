#include "ultrasonico.h"

// Definición de los pines (declarados como extern en el .h)
int ULTRA_PIN_TRIG = 8;
int ULTRA_PIN_ECHO = 9;

// Redirige el sensor a otro par de pines Trig/Echo y los reconfigura.
void ultra_configurarPines(int pinTrig, int pinEcho) {
    ULTRA_PIN_TRIG = pinTrig;
    ULTRA_PIN_ECHO = pinEcho;
    ultra_inicializar();
}

// Configura los pines Trig (salida) y Echo (entrada).
// Debe llamarse una vez en setup().
void ultra_inicializar() {
    pinMode(ULTRA_PIN_TRIG, OUTPUT);
    pinMode(ULTRA_PIN_ECHO, INPUT);
    digitalWrite(ULTRA_PIN_TRIG, LOW);
}

// Dispara el pulso de Trig, mide el ancho del pulso de Echo y devuelve
// la distancia en centímetros.
//
// Devuelve ULTRA_ERROR si no hay eco dentro del timeout (nada en rango
// o sensor desconectado), o si la distancia calculada queda fuera del
// rango confiable del sensor (2 a 400 cm).
float ultra_leerDistanciaCM() {
    // Asegura que Trig empiece en LOW
    digitalWrite(ULTRA_PIN_TRIG, LOW);
    delayMicroseconds(2);

    // Pulso de disparo de al menos 10 us
    digitalWrite(ULTRA_PIN_TRIG, HIGH);
    delayMicroseconds(ULTRA_PULSO_TRIG_US);
    digitalWrite(ULTRA_PIN_TRIG, LOW);

    // Mide cuánto dura el pulso HIGH en Echo (tiempo de ida y vuelta del sonido)
    unsigned long duracion = pulseIn(ULTRA_PIN_ECHO, HIGH, ULTRA_TIMEOUT_US);

    if (duracion == 0) {
        return ULTRA_ERROR;  // No llegó eco dentro del timeout
    }

    // distancia = tiempo × velocidad del sonido (340 m/s) / 2 (ida y vuelta)
    // En cm: distancia_cm = duracion_us / 58
    float distancia = duracion / 58.0;

    if (distancia < ULTRA_DIST_MIN_CM || distancia > ULTRA_DIST_MAX_CM) {
        return ULTRA_ERROR;  // Fuera del rango confiable del sensor
    }

    return distancia;
}
