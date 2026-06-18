# Proyecto PAPIME — Sensei

## Contexto

Sensei es un kit de sensores desarrollado como parte del proyecto PAPIME PE309523 del Instituto de Ciencias Aplicadas y Tecnología (ICAT) de la UNAM. Se va a usar en las Escuelas Nacionales Preparatorias 3, 7 y 9. Se van a fabricar varias unidades del kit; cada una funciona de forma independiente.

---

## ¿Qué es Sensei?

Sensei es un kit de sensores que mide variables físicas y muestra los datos al usuario de tres formas: en un display LCD integrado en el kit, en una página web (para pruebas y trabajo de titulación), y en una app Android (el producto final del proyecto). Las tres muestran exactamente los mismos datos: el nombre de cada sensor conectado y su valor actual. Si un conector está vacío, no aparece nada para ese conector.

El kit tiene 4 conectores RJ45 hembra donde se enchufan los sensores mediante un cable UTP. Se pueden conectar hasta 4 sensores al mismo tiempo, en cualquier combinación: los 4 iguales, los 4 diferentes, o cualquier mezcla. El sistema detecta automáticamente qué sensor hay en cada conector.

El kit funciona tanto conectado a la corriente eléctrica como con batería para uso portátil.

---

## Hardware

### Arduino Mega 2560
- Microcontrolador central.
- Lee todos los sensores conectados.
- Muestra los datos en el LCD.
- Envía los datos a la ESP32 por UART.
- Opera a 5 V.
- 16 entradas analógicas (A0–A15) y varios pines digitales.

### ESP32 NodeMCU
- Recibe los datos del Mega por UART.
- Levanta un servidor web HTTP en la red WiFi local.
- Maneja la conectividad WiFi y Bluetooth (BLE).
- No lee sensores directamente.
- Opera a 3.3 V.

### Display LCD 20×4 con I2C
- Conectado al Arduino Mega por I2C (SDA: pin 20, SCL: pin 21).
- Muestra el nombre y valor de cada sensor conectado en tiempo real.
- Solo muestra los sensores que están conectados en ese momento.
- Usa el módulo adaptador I2C (PCF8574): 4 cables (VCC, GND, SDA, SCL).

### Batería
- Batería recargable USB Li-Ion tipo 9V cuadrada, 3600 mWh (Steren SKU BAT-LI-9V USB).
- Se recarga por USB-C sin quitarla del sistema.
- Terminales tipo broche estándar.
- Se conecta al pin VIN del Mega (acepta 7–12 V, con regulador interno a 5 V).
- El kit también funciona conectado a corriente eléctrica.

**Opciones para que batería y corriente eléctrica coexistan (pendiente de decidir):**

- **Diodo OR:** un diodo en la línea de la batería y otro en la de la corriente externa. Cuando ambas están conectadas, la de mayor voltaje toma el control automáticamente. Es simple y barato, pero cada diodo tiene una caída de voltaje de ~0.6 V.
- **Switch de prioridad con MOSFET:** detecta cuándo hay corriente externa y desconecta la batería automáticamente, sin caída de voltaje. Es más preciso pero más complejo de diseñar.

---

## Conexión física de los sensores (RJ45 + cable UTP)

El kit tiene 4 conectores RJ45 hembra. Cada módulo sensor también tiene un conector RJ45 hembra. La conexión entre el kit y el sensor se hace con un cable UTP ponchado con conector RJ45 macho en cada punta. Por ese cable van:

- Alimentación del sensor (VCC y GND).
- El voltaje de identificación del sensor.
- La señal del sensor (analógica o digital según el sensor).

El cable se poncha en los dos extremos siguiendo el estándar **T568B** (cable directo, no cruzado):

| Pin | Color |
|-----|-------|
| 1 | Blanco-Naranja |
| 2 | Naranja |
| 3 | Blanco-Verde |
| 4 | Azul |
| 5 | Blanco-Azul |
| 6 | Verde |
| 7 | Blanco-Café |
| 8 | Café |

La asignación de qué pin va a VCC, GND, ID, señal analógica y señal digital está **pendiente de definir**. Una vez definida, todos los módulos sensores se fabrican con esa misma asignación.

**Longitud máxima del cable:** para uso en aula, 1 metro es suficiente y seguro para señales analógicas. Cables más largos aumentan el ruido en la señal.

---

## Detección automática de sensores

Cada sensor tiene un divisor de voltaje que produce un voltaje único de identificación. Cuando se enchufa el cable, el Mega lee ese voltaje y determina qué sensor es. Con eso también sabe si la señal es analógica (`analogRead`) o digital (`digitalRead` o protocolo 1-Wire).

Los 16 sensores tienen cada uno un voltaje de identificación diferente, distribuidos en el rango 0–5 V con suficiente separación para que el ADC los distinga (resolución del ADC: ≈ 4.88 mV por paso). Los valores exactos se definen al diseñar el circuito de cada sensor.

**Propuesta para manejar pines analógicos y digitales por conector (pendiente de confirmar):**

- **Opción A — Pin dual por conector (recomendada):** cada conector RJ45 tiene asignado un pin analógico y un pin digital en el Mega. El Mega lee el ID y decide cuál usar. Cualquier sensor funciona en cualquier conector sin restricciones.
- **Opción B — Solo pin analógico por conector:** simplifica el cableado pero no es preciso para protocolos digitales como 1-Wire.

### Qué contiene cada módulo sensor

1. El componente sensor.
2. Circuito de acondicionamiento de señal (si aplica): ajusta la señal para que el Mega pueda leerla correctamente. Los sensores con módulo PCB (como el ACS712 o el PH-4502C) ya lo traen incluido.
3. Divisor de voltaje de identificación.
4. Conector RJ45 hembra.

---

## Sensores del proyecto

Documentación técnica completa de cada sensor: [`docs/sensores.md`](sensores.md)

### Sensores analógicos

| # | Sensor | Pin Mega |
|---|--------|----------|
| 1 | Sensor de Voltaje 0–25 V DC (AR2657) | A0 |
| 2 | Sensor de Corriente ACS712 (30 A) | A1 |
| 3 | Sensor de pH PH-4502C | A2 |
| 4 | Fototransistor PT331C | A3 |
| 5 | Sensor de Humedad de Suelo (OKY3442) | A4 |
| 6 | Sensor de CO2 MG811 (OKY3351) | A5 |
| 7 | Sensor de Pulso Cardíaco (OKY3471-5) | A6 |

### Sensores digitales

| # | Sensor | Pin Mega |
|---|--------|----------|
| 8 | Sensor de Efecto Hall SM351LT | Digital 22 |
| 9 | Sensor de Temperatura DS18B20 (sumergible) | Digital 7 |

Los sensores 10–16 no están definidos todavía.

---

## Conexión Mega ↔ ESP32 (UART)

Comunicación por UART serial a 115200 baudios. El Mega envía los datos en JSON.

El Mega opera a 5 V y el ESP32 solo tolera 3.3 V en sus pines de entrada. Por eso la línea TX del Mega necesita adaptación de niveles antes de llegar al pin RX de la ESP32. La línea del ESP32 hacia el Mega no necesita adaptación (3.3 V es suficiente para que el Mega la detecte como HIGH).

```
Mega TX1 (pin 18) → [adaptación 5V→3.3V] → ESP32 RX
Mega RX1 (pin 19) ───────────────────────→ ESP32 TX
```

Circuito de adaptación: divisor de voltaje con resistencias de 1 kΩ y 2 kΩ, o módulo level shifter bidireccional 5V/3.3V.

---

## Nombre del kit en Bluetooth

Cada unidad del kit aparece en Bluetooth con el nombre **SENSEI-XXXX**, donde XXXX son los últimos 4 caracteres de la dirección MAC de la ESP32. La MAC es única por chip, así que el nombre queda único por unidad sin configuración manual. Ejemplo: `SENSEI-A1B2`, `SENSEI-3C4D`.

---

## Configuración de WiFi por BLE

Como el kit se usa en distintos lugares, las credenciales WiFi no se dejan fijas en el código. El usuario las configura desde la app Android o desde la página web cada vez que cambia de red.

### Flujo (igual para la página web y la app Android)

1. El usuario abre la app o la página web.
2. Se conecta a la ESP32 por BLE.
3. Le envía a la ESP32 el nombre (SSID) y la contraseña de la red WiFi.
4. La ESP32 guarda las credenciales, se desconecta del BLE y se conecta al WiFi.
5. Una vez en el WiFi, levanta el servidor HTTP.
6. La app o la página web consultan el endpoint `/datos` y reciben los datos en JSON.

### Opciones de implementación BLE en la ESP32 (pendiente de decidir)

- **Opción A — NimBLE (recomendada):** librería BLE más ligera y eficiente en memoria.
- **Opción B — ESP32 BLE Arduino:** librería estándar, más documentación pero consume más RAM.
- **Opción C — ESP-IDF directo:** API nativa de Espressif, máximo control pero más compleja.

### Sobre la página web

Usa la **Web Bluetooth API** para conectarse a la ESP32 por BLE. Solo funciona en Chrome y Edge; no en Safari ni Firefox.

**Opciones para la página web (pendiente de decidir):**

- Quedarse con Chrome/Edge. Para pruebas y tesis es suficiente.
- Publicar la página como **PWA (Progressive Web App)** instalable desde Chrome.

---

## Servidor web en la ESP32

- Endpoint: `GET /datos`
- Responde con JSON con los valores de los sensores conectados en ese momento.
- Solo aparecen los sensores enchufados; los conectores vacíos no aparecen.

### Estructura del JSON (propuesta — pendiente de confirmar con el desarrollador Android)

```json
{
  "sensei": "Sensei-A1B2",
  "sensores": [
    { "puerto": 1, "nombre": "pH", "valor": 7.20, "unidad": "pH" },
    { "puerto": 3, "nombre": "Temperatura", "valor": 24.50, "unidad": "°C" }
  ]
}
```

### Decimales por sensor (propuesta inicial — se ajusta al implementar cada uno)

| Sensor | Decimales | Razón |
|--------|-----------|-------|
| Voltaje AR2657 | 2 | Resolución ~24 mV |
| Corriente ACS712 | 2 | Resolución ~74 mA |
| pH PH-4502C | 2 | Resolución ~0.03 pH |
| Fototransistor PT331C | 0 | Se reporta como valor ADC (0–1023) |
| Humedad OKY3442 | 1 | Porcentaje aproximado |
| CO2 MG811 | 0 | Se reporta en ppm enteros |
| Pulso OKY3471-5 | 0 | BPM son siempre enteros |
| Efecto Hall SM351LT | — | Digital: "detectado" / "no detectado" |
| Temperatura DS18B20 | 2 | Precisión ±0.5 °C |

### Flujo completo de datos

```
Sensores (conectados vía cable UTP + RJ45)
      ↓
Arduino Mega 2560
(lee sensores, detecta cuáles están conectados, muestra en LCD, empaqueta JSON)
      ↓  UART 115200 baud
ESP32 NodeMCU
(recibe JSON, levanta servidor HTTP)
      ↓  HTTP / WiFi local
      ├──→ Página web (Chrome o Edge) — pruebas y tesis
      └──→ App Android — producto final
```

La página web y la app Android no funcionan al mismo tiempo.

---

## Estructura del repositorio

```
PAPIME/
├── README.md                               ← presentación del proyecto
├── platformio.ini                          ← entorno Mega + entorno ESP32
├── src/                                    ← código del Arduino Mega
│   ├── main.cpp
│   ├── sensores_analogicos/
│   │   ├── ph/                             ✅ implementado
│   │   ├── voltaje/                        ⬜ pendiente
│   │   ├── corriente/                      ⬜ pendiente
│   │   ├── fototransistor/                 ⬜ pendiente
│   │   ├── humedad_suelo/                  ⬜ pendiente
│   │   ├── co2/                            ⬜ pendiente
│   │   └── pulso_cardiaco/                 ⬜ pendiente
│   └── sensores_digitales/
│       ├── hall/                           ⬜ pendiente
│       └── temperatura_ds18b20/            ⬜ pendiente
├── esp32/                                  ← código de la ESP32
│   └── main.cpp                            ⬜ pendiente
├── docs/                                   ← documentación del proyecto
│   ├── proyecto.md                         ← este archivo
│   ├── sensores.md                         ← documentación técnica de cada sensor
│   └── android.md                          ← coordinación con desarrollador Android
└── extras/
    └── ph/                                 ← archivos de calibración del sensor pH
        ├── calibracion_bnc.cpp
        └── calibracion_buffer.cpp
```

### Convenciones de código

- Cada sensor tiene su propia carpeta dentro de `sensores_analogicos/` o `sensores_digitales/`.
- Cada sensor tiene un `.h` con sus constantes y un `.cpp` con su implementación.
- Nombres de funciones: `nombreSensor_leerVoltaje()`, `nombreSensor_calcularMagnitud()`.
- Constantes con `const` en lugar de `#define`.
- Sin variables globales innecesarias.
- Sin `delay()` donde se pueda evitar — se usa lógica no bloqueante con `millis()`.
- Comentarios claros en cada función.

---

## Estado del proyecto

| Componente | Estado |
|---|---|
| Sensor de pH PH-4502C | ✅ Implementado |
| Sensor de Voltaje AR2657 | ⬜ Pendiente |
| Sensor de Corriente ACS712 | ⬜ Pendiente |
| Fototransistor PT331C | ⬜ Pendiente |
| Sensor de Humedad OKY3442 | ⬜ Pendiente |
| Sensor de CO2 MG811 | ⬜ Pendiente |
| Sensor de Pulso OKY3471-5 | ⬜ Pendiente |
| Sensor Hall SM351LT | ⬜ Pendiente |
| Sensor Temperatura DS18B20 | ⬜ Pendiente |
| Sensores 10–16 | ❓ No definidos |
| Display LCD 20×4 con I2C | ⬜ Pendiente |
| Conexión Mega ↔ ESP32 (UART) | ⬜ Pendiente |
| Asignación de pines del cable UTP | ⬜ Pendiente |
| Voltajes de identificación (16 sensores) | ⬜ Pendiente |
| Circuito de alimentación dual | ⬜ Pendiente |
| Servidor web en ESP32 | ⬜ Pendiente |
| WiFi provisioning por BLE | ⬜ Pendiente |
| Página web (Chrome/Edge) | ⬜ Pendiente |
| App Android | 🔄 En desarrollo (otra persona) |

---

## Pendiente de decidir

1. **Asignación de pines del cable UTP** — qué pin va a VCC, GND, ID, señal analógica y señal digital.
2. **Voltajes de identificación de los 16 sensores** — se define al diseñar el circuito de cada sensor.
3. **Confirmar estructura del JSON** con el desarrollador Android (ver `docs/android.md`).
4. **Sensores 10–16** — todavía no están definidos.
5. **Perfil BLE** (UUIDs GATT) — coordinar con el desarrollador Android (ver `docs/android.md`).
6. **Opción de alimentación dual** — diodo OR o switch con MOSFET.
7. **Página web** — quedarse con Chrome/Edge o publicar como PWA.
8. **Confirmar Opción A** para pines por conector (pin analógico + pin digital por cada RJ45).
9. **Decimales por sensor** — se confirman al implementar cada sensor.

---

## Referencias

- PlatformIO, Arduino Mega 2560: https://docs.platformio.org/en/stable/boards/atmelavr/megaatmega2560.html
- Arduino Mega 2560: https://docs.arduino.cc/hardware/mega-2560/
- ESP32 con framework Arduino: https://docs.espressif.com/projects/arduino-esp32/en/latest/
- Web Bluetooth API: https://developer.chrome.com/docs/capabilities/bluetooth
- Batería Steren BAT-LI-9V USB: https://www.steren.com.mx/bateria-recargable-usb-li-ion-tipo-9v-cuadrada-de-500-mah.html
