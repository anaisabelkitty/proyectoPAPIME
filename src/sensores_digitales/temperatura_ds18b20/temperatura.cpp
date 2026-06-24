#include "temperatura.h"

// --- Objetos del bus 1-Wire y la librería DallasTemperature ---
// Se declaran estáticos para que solo sean visibles dentro de este archivo.
static OneWire       _onewire(TEMP_PIN_SENSOR);
static DallasTemperature _sensores(&_onewire);

// Inicializa el bus 1-Wire y configura la resolución del sensor.
// Debe llamarse una vez en setup().
void temp_inicializar() {
    _sensores.begin();
    _sensores.setResolution(TEMP_RESOLUCION);
    // setWaitForConversion(false) permitiría no bloquearse 750 ms,
    // pero requiere manejar el temporizador externamente.
    // Por ahora se deja el modo bloqueante (más simple y seguro).
}

// Solicita una medición al sensor y devuelve la temperatura en °C.
// Si el sensor está desconectado o hay un error en el bus,
// devuelve TEMP_ERROR (-127.00 °C).
//
// Nota: requestTemperatures() bloquea ~750 ms con resolución de 12 bits.
// Si se necesita comportamiento no bloqueante, reemplazar por
// requestTemperaturesAsync() + millis().
float temp_leerCelsius() {
    _sensores.requestTemperatures();                     // Dispara la conversión
    float temperatura = _sensores.getTempCByIndex(0);   // Lee el primer sensor del bus

    // getTempCByIndex devuelve DEVICE_DISCONNECTED_C (-127) si hay error
    return temperatura;
}
