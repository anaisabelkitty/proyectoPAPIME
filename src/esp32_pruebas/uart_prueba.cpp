// Prueba de comunicación UART Mega ↔ ESP32-C6 (lado de la ESP32).
// Guía de conexión: docs/prueba_uart_esp32_mega.md
//
// Escucha por UART1 (RX = GPIO17, TX = GPIO16), imprime lo que llega del Mega en el
// monitor y contesta "PONG <n>" a cada "PING <n>". El monitor usa el USB nativo
// (ARDUINO_USB_CDC_ON_BOOT) para no compartir UART0 con los pines de la prueba.
// El programa del Mega está en src/mega_pruebas/uart_prueba.cpp.

#include <Arduino.h>

constexpr int PIN_RX_MEGA = 17;
constexpr int PIN_TX_MEGA = 16;
constexpr unsigned long BAUDIOS_MEGA = 115200;
constexpr unsigned long AVISO_SILENCIO_MS = 3000;

String linea;
unsigned long ultimoRx = 0;
unsigned long recibidos = 0;

void setup() {
  Serial.begin(115200);
  unsigned long inicio = millis();
  while (!Serial && millis() - inicio < 3000) delay(10);

  Serial1.begin(BAUDIOS_MEGA, SERIAL_8N1, PIN_RX_MEGA, PIN_TX_MEGA);
  ultimoRx = millis();

  Serial.println("=== Prueba UART ESP32-C6 <-> Mega ===");
  Serial.printf("UART1 a %lu baudios, RX=GPIO%d, TX=GPIO%d\n", BAUDIOS_MEGA, PIN_RX_MEGA, PIN_TX_MEGA);
  Serial.println("Esperando PING del Mega...");
}

void procesarLinea(const String &l) {
  recibidos++;
  Serial.printf("Recibido del Mega: \"%s\"\n", l.c_str());

  if (l.startsWith("PING ")) {
    String respuesta = "PONG " + l.substring(5);
    Serial1.println(respuesta);
    Serial.printf("  -> respondido: \"%s\"\n", respuesta.c_str());
  } else {
    Serial.println("  (no es un PING valido: revisar baudios o ruido)");
  }
}

void loop() {
  while (Serial1.available()) {
    char c = Serial1.read();
    ultimoRx = millis();
    if (c == '\n') {
      procesarLinea(linea);
      linea = "";
    } else if (c != '\r' && linea.length() < 64) {
      linea += c;
    }
  }

  if (millis() - ultimoRx > AVISO_SILENCIO_MS) {
    ultimoRx = millis();
    Serial.println("Sin datos del Mega (revisar Mega TX1 -> divisor -> GPIO17 y GND comun)");
  }
}
