# Plan de implementación: firmware real del kit Sensei

Define cómo queda el firmware real del Mega y de la ESP32-C6: pantallas del LCD, conexiones, comunicación, JSON y orden de implementación. Todo se escribe desde cero en la rama `feature/comunicacion`, sin copiar código de `feature/esp32-mega`. Los requisitos base están en [`proyecto.md`](proyecto.md) y [`protocolo.md`](protocolo.md); si algo choca, manda `protocolo.md`.

Las pruebas de cableado, BLE y WiFi que ya funcionan están en [`prueba_uart_esp32_mega.md`](prueba_uart_esp32_mega.md) y [`prueba_wifi_ble_esp32_mega.md`](prueba_wifi_ble_esp32_mega.md).

---

## 1. Pantallas del LCD (20×4)

La fila 0 lleva siempre el nombre del kit, con `SENSEI` en mayúsculas. Antes de que la ESP32 lo reporte dice `SENSEI` a secas. Los valores numéricos de los sensores no se muestran en el LCD; esos van por WebSocket a la app y a la web.

**Arranque (1.5 s)**
```
       SENSEI
    Iniciando...
```

**Sin enlace con la ESP32** (el Mega no recibe nada por UART)
```
       SENSEI
 Sin enlace con ESP32
 Revisa cable/energia
```

**Paso 1 de 3: Bluetooth**
```
    SENSEI-85A8
    Paso 1 de 3
 Conecta por Bluetooth
    desde la app
```

**Paso 2 de 3: WiFi.** Cambian las filas 2 y 3 según el estado:

| Estado | Fila 2 | Fila 3 |
|---|---|---|
| Sin configurar | `Elige la red WiFi` | `en la app` |
| Conectando | `Conectando a WiFi...` | vacía |
| Falló | `No se pudo conectar` | `Intenta de nuevo` |

**Paso 3 de 3: sensores**
```
    SENSEI-85A8
P1: PH    P2: HUM
P3: --    P4: TEMP
 3 de 4 conectados
```
Puerto vacío: `--`. Sin sensores: `0 de 4 conectados`. La IP no se muestra: la app y la web la reciben por BLE.

### Cuándo cambia de pantalla

El orden es siempre Bluetooth, luego WiFi, luego sensores. Sin Bluetooth no se avanza.

| Condición | Pantalla |
|---|---|
| La ESP32 no se ha reportado | Sin enlace |
| Hay ESP32, ningún celular por BLE y WiFi no conectado | Paso 1 |
| Hay celular por BLE, o WiFi conectando/fallido | Paso 2 |
| WiFi conectado | Paso 3 (se queda aunque otros celulares entren y salgan por BLE) |
| Se cae el WiFi | Regresa al paso 2 |

---

## 2. Sensores, abreviaturas y decimales

Los decimales salen de [`proyecto.md`](proyecto.md). Las abreviaturas de los sensores pendientes son propuestas.

| Sensor | `id` en el JSON | Unidad | LCD | Decimales | Estado |
|---|---|---|---|---|---|
| pH PH-4502C | `ph` | `pH` | `PH` | 2 | Programado |
| Humedad OKY3442 | `humedad` | `%` | `HUM` | 1 | Programado |
| Temperatura DS18B20 | `temperatura` | `°C` | `TEMP` | 2 | Programado |
| Ultrasónico HC-SR04 | `distancia` | `cm` | `DIST` | 1 | Programado; id por confirmar con Android |
| Voltaje AR2657 | `voltaje` | `V` | `VOLT` | 2 | Pendiente |
| Corriente ACS712 | `corriente` | `A` | `CORR` | 2 | Pendiente |
| CO2 MG811 | `co2` | `ppm` | `CO2` | 0 | Pendiente |
| Pulso OKY3471-5 | `pulso` | `bpm` | `PULS` | 0 | Pendiente |
| Fototransistor PT331C | `luz` | (vacío) | `LUZ` | Por definir | Pendiente |
| Efecto Hall SM351LT | `hall` | (vacío) | `HALL` | Por definir | Pendiente |

- **Programado:** el Mega ya reconoce el código del conector y sabe leer el sensor.
- **Pendiente:** el código de ese sensor aún no existe; si se enchufa, sale como puerto vacío.
- **Id por confirmar:** `distancia` no está en la tabla de [`protocolo.md`](protocolo.md), así que Android no lo ha acordado.

Códigos de identificación del conector: 2 = pH, 4 = humedad, 9 = temperatura, 10 = ultrasónico, 0 = vacío.

---

## 3. Conexiones físicas

| Enlace | Conexión |
|---|---|
| Mega → ESP32 | Mega TX1 (pin 18) → 1 kΩ → GPIO17; del mismo punto, 2 kΩ → GND |
| ESP32 → Mega | GPIO16 → Mega RX1 (pin 19), directo |
| GND | Común entre Mega y ESP32 |
| LCD I2C | SDA pin 20, SCL pin 21, VCC, GND (dirección autodetectada) |
| Conectores RJ45 | ID por pines pares 22–52 con pull-up; señal en A0–A7 |

Los pines GPIO16/17 de la ESP32 comparten UART0 con el chip USB-serial CH343. Por eso el firmware usa `ARDUINO_USB_CDC_ON_BOOT=1` y el monitor sale por el USB nativo.

---

## 4. Canales de comunicación

| # | Canal | Dirección | Contenido | Frecuencia |
|---|---|---|---|---|
| 1 | UART 115200 | Mega → ESP32 | Una línea JSON terminada en `\n` | Cada 500 ms |
| 2 | UART 115200 | ESP32 → Mega | `#NAME`, `#WIFI`, `#BLE`, `#IP` | Cada 3 s y al cambiar algo |
| 3 | BLE | Celular ↔ ESP32 | Credenciales, estado, IP, SSID | Solo al configurar o descubrir la IP |
| 4 | WebSocket `ws://<ip>/ws` (puerto 80) | ESP32 → app/web | El mismo JSON, sin modificarlo | Cada 500 ms |
| 5 | HTTP `GET /data` | ESP32 → cliente | Último JSON | Bajo demanda |

### Tiempos

- El Mega arma el JSON cada 500 ms. La ESP32 lo reenvía en cuanto llega. El JSON pesa unos 250 bytes y cruza el UART en unos 22 ms.
- El Mega manda el JSON siempre, incluso sin WiFi. La ESP32 lo descarta hasta que haya WiFi.
- Al conectarse un cliente nuevo por WebSocket recibe de inmediato el último JSON.
- El DS18B20 tarda unos 750 ms en medir. Para no pasarse de 500 ms, se pide la conversión y se recoge en el ciclo siguiente, sin bloquear.
- El LCD se actualiza en el mismo ciclo y solo reescribe las filas que cambiaron, para que no parpadee.

---

## 5. Secuencia

1. Se enciende el kit. LCD "Iniciando"; la ESP32 arranca BLE como `SENSEI-XXXX` y reporta `#NAME`. El LCD pasa al Paso 1.
2. El celular se conecta por BLE y lee `WIFI_STATE = UNCONFIGURED`. La ESP32 manda `#BLE 1` y el LCD pasa al Paso 2.
3. La app escribe `SSID\nPASSWORD` en `WIFI_CRED`. La ESP32 notifica `CONNECTING`.
4. Si conecta: notifica `CONNECTED`, `IP_ADDR` y `WIFI_SSID`, y levanta el servidor. El LCD pasa al Paso 3. Si falla: notifica `FAILED` y se queda en el Paso 2 para reintentar.
5. El JSON del Mega sale por WebSocket cada 500 ms.
6. Otro celular en la misma red: se conecta por BLE, lee `CONNECTED`, la IP y el SSID, y abre el WebSocket. No configura nada y el LCD no cambia.
7. Se apaga el kit: las credenciales se borran y todo vuelve al paso 1.

---

## 6. JSON

Una sola línea, sin espacios, terminada en `\n`, con los 4 puertos siempre:

```
{"s":[{"p":1,"c":true,"id":"ph","v":7.20,"u":"pH"},{"p":2,"c":true,"id":"humedad","v":42.5,"u":"%"},{"p":3,"c":false},{"p":4,"c":true,"id":"temperatura","v":24.50,"u":"°C"}]}
```

| Campo | Regla |
|---|---|
| `p` | Puerto 1–4 |
| `c` | `true` si el código de identificación es de un sensor conocido. `false` si está vacío, es desconocido, no está programado o la lectura falló |
| `id`, `v`, `u` | Solo con `c:true`. `v` siempre es número, con los decimales de la sección 2 |

- No lleva campo `"int"`: el intervalo es fijo en 500 ms según [`protocolo.md`](protocolo.md).
- No lleva el nombre del kit ni el estado de BLE/WiFi; eso va por otros canales.
- `c:false` no distingue "sin sensor" de "sensor con error". Es lo acordado con Android.

---

## 7. Bluetooth: UUID y textos

Nombre BLE: `SENSEI-XXXX` (últimos 4 de la MAC, en mayúsculas). La app filtra con `startsWith("SENSEI-")`.

Servicio: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`

| Característica | UUID | Quién escribe | Texto |
|---|---|---|---|
| `WIFI_SCAN` | `beb5483e-36e1-4688-b7f5-ea07361b26a8` | App | `SCAN` |
| `WIFI_CRED` | `beb5483e-36e1-4688-b7f5-ea07361b26a9` | App | `SSID\nPASSWORD` |
| `WIFI_STAT` | `beb5483e-36e1-4688-b7f5-ea07361b26aa` | ESP32 | `CONNECTING` / `CONNECTED` / `FAILED` |
| `IP_ADDR` | `beb5483e-36e1-4688-b7f5-ea07361b26ab` | ESP32 | `172.20.10.6` |
| `WIFI_SSID` | `243c4f09-ebd7-4026-940a-10afc3c8993e` | ESP32 | `MiRed` |
| `WIFI_STATE` | `4499e77a-31ec-4153-bcde-00e90bb808c9` | ESP32 | `UNCONFIGURED` / `CONNECTED` / `FAILED` |

Los UUID y las mayúsculas deben ser exactos, porque la app compara el texto. `WIFI_SCAN` está en [`respuestasAndroid.md`](respuestasAndroid.md) pero falta en la tabla de [`protocolo.md`](protocolo.md).

---

## 8. Herramientas

- **ESP32-C6:** NimBLE-Arduino, AsyncTCP y ESPAsyncWebServer. Puente UART → WebSocket sin modificar el JSON.
- **Mega:** `Serial1` a 115200, LiquidCrystal_I2C (hoy falta en `platformio.ini`), OneWire y DallasTemperature.

---

## 9. Orden de implementación

Un commit por cambio, según [`commits.md`](commits.md). Todo se escribe desde cero en esta rama.

1. Módulo de conectores del Mega, con base en la lógica que ya está en `src/main.cpp`.
2. Lectura no bloqueante del DS18B20.
3. Módulo de enlace UART: envío del JSON y lectura de las líneas `#`.
4. Módulo del LCD con los textos y las reglas de la sección 1.
5. Firmware nuevo de la ESP32: BLE, WiFi, servidor, WebSocket y reporte al Mega.
6. `main.cpp` del Mega: ciclo de 500 ms, JSON y LCD.
7. `platformio.ini`: entorno real del Mega y de la ESP32, más los de prueba.
8. Documentación: id `distancia`, `WIFI_SCAN`, estado del proyecto y estructura del repositorio.
9. Pruebas con el hardware: con un sensor, con dos y con ninguno.

---

## 10. Pendientes

| Tema | Detalle |
|---|---|
| Id `distancia` | Avisar al desarrollador Android del id, la unidad `cm` y el decimal, y agregarlo a la tabla de `protocolo.md` |
| Sensores sin programar | Voltaje, corriente, CO2, pulso, luz y Hall quedan fuera de esta etapa |
| Luz y Hall en el JSON | Falta definir cómo representarlos (ver `protocolo.md` sección 9) |
| Abreviaturas | `VOLT`, `CORR`, `PULS`, `LUZ`, `HALL` son propuestas por confirmar |
| Archivo de prueba de la ESP32 | `src/esp32_pruebas/wifi_ble_uart_prueba.cpp` se copió de `feature/esp32-mega`. Decidir si se conserva o se reescribe |
