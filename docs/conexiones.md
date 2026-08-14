# Conexiones del kit Sensei — resumen

Este documento reúne, en formato de tabla, todas las conexiones físicas que ya están definidas o probadas en el proyecto. Como es un resumen, cada sección enlaza al documento fuente donde está el detalle completo. Por eso, si se modifica un valor aquí, hay que actualizar también el documento fuente correspondiente para que no queden desactualizados entre sí.

---

## Componentes conectados al Arduino Mega 2560

### Display LCD 20×4 (adaptador I2C, chip PCF8574)

Fuente: [`diseno_electronico.md`](diseno_electronico.md#display-lcd-20×4)

| Pin LCD (adaptador I2C) | Destino | Notas |
|---|---|---|
| VCC | 5V del Mega | Alimentación |
| GND | GND del Mega | Tierra común |
| SDA | Pin 20 del Mega | Bus I2C, datos |
| SCL | Pin 21 del Mega | Bus I2C, reloj |

La dirección I2C del módulo todavía no está confirmada. Las más comunes son `0x27` y `0x3F`, pero no se puede asumir ninguna de las dos sin correr un I2C Scanner primero.

---

### ESP32-C6 (Waveshare ESP32-C6-DEV-KIT-N8, comunicación UART)

Fuente: [`proyecto.md`](proyecto.md#conexión-mega--esp32-uart)

| Sentido | Origen | Destino | Notas |
|---|---|---|---|
| Mega → ESP32-C6 | Mega TX1 (pin 18) | ESP32-C6 RX (GPIO17 / U0RXD) | Pasa por un divisor de voltaje (resistencia de 1kΩ y resistencia de 2kΩ) que baja la señal de 5V a 3.3V, porque el ESP32-C6 no tolera más de 3.6V en sus pines |
| ESP32-C6 → Mega | ESP32-C6 TX (GPIO16 / U0TXD) | Mega RX1 (pin 19) | Conexión directa, sin resistencias. El Mega reconoce 3.3V como HIGH sin necesitar que la señal llegue a 5V |

La comunicación va a 115200 baudios. El Mega manda el JSON de sensores cada 500ms y el ESP32 lo reenvía tal cual por WebSocket, sin modificarlo. Este circuito todavía no está armado físicamente.

---

## Sensores por conector RJ45 (núcleo → módulo)

Los 4 conectores siguen el mismo patrón de 8 pines. La convención es fija: de izquierda a derecha, el pin 3 es ID3, el pin 4 es ID2, el pin 5 es ID1 y el pin 6 es ID0. Los pines 7 y 8 son Señal 1 y Señal 2.

### Patrón general del RJ45

| Pin RJ45 | Destino | Notas |
|---|---|---|
| 1 | VCC | 5V |
| 2 | GND | GND |
| 3 | ID3 | Ver tabla de pines por conector |
| 4 | ID2 | Ver tabla de pines por conector |
| 5 | ID1 | Ver tabla de pines por conector |
| 6 | ID0 | Ver tabla de pines por conector |
| 7 | Señal 1 | Ver tabla de pines por conector |
| 8 | Señal 2 | Solo lo usa el HC-SR04 (Echo); los demás sensores lo dejan sin conectar |

### Pines del Mega por conector

Fuente: [`diseno_electronico.md`](diseno_electronico.md#pines-del-mega-dedicados-a-esto)

| Conector | ID3 | ID2 | ID1 | ID0 | Señal 1 | Señal 2 |
|---|---|---|---|---|---|---|
| 1 | 22 | 23 | 24 | 25 | A0 | A1 |
| 2 | 26 | 27 | 28 | 29 | A2 | A3 |
| 3 | 30 | 31 | 32 | 33 | A4 | A5 |
| 4 | 34 | 35 | 36 | 37 | A6 | A7 |

### Sensor conectado hoy en cada conector

| Conector | Sensor | Código | Bit(s) con resistencia 1kΩ a GND | Guía completa |
|---|---|---|---|---|
| 1 | Temperatura DS18B20 | 8 | ID3 | [`prueba_temperatura_conector1.md`](prueba_temperatura_conector1.md) |
| 2 | pH PH-4502C | 2 | ID1 | [`prueba_ph_conector2.md`](prueba_ph_conector2.md) |
| 3 | Ultrasónico HC-SR04 | 9 | ID3, ID0 | [`prueba_ultrasonico_conector3.md`](prueba_ultrasonico_conector3.md) |
| 4 | Humedad OKY3442 | 4 | ID2 | [`prueba_humedad_conector4.md`](prueba_humedad_conector4.md) |

El DS18B20 usa Señal 1 como línea de datos (DQ), y por eso necesita un pull-up de 4.7kΩ a VCC, ya que así lo pide el protocolo 1-Wire. El PH-4502C y el OKY3442 usan Señal 1 como su salida analógica, Po y A0 respectivamente. El HC-SR04 es distinto a los otros tres porque usa las dos señales: Señal 1 es Trig y Señal 2 es Echo.

---

## Pendientes de conexión física

| Componente | Estado |
|---|---|
| LCD 20×4 | Dirección I2C sin confirmar; contraste sin ajustar |
| ESP32 ↔ Mega (UART) | Circuito de adaptación de nivel 5V→3.3V sin armar |
| Alimentación dual (batería + corriente) | Sin resolver, se define al armar la hoja principal en KiCad |

---

## Referencias

- Diseño electrónico completo: [`diseno_electronico.md`](diseno_electronico.md)
- Arquitectura general y protocolo UART: [`proyecto.md`](proyecto.md)
- Protocolo BLE/WiFi/WebSocket: [`protocolo.md`](protocolo.md)
- Datasheets y fórmulas de cada sensor: [`sensores.md`](sensores.md)
