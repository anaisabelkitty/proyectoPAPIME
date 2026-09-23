# Prueba: BLE + WiFi + UART entre la ESP32-C6 y el Mega

Segunda etapa después de [`prueba_uart_esp32_mega.md`](prueba_uart_esp32_mega.md). Primero haz esa (solo cableado); cuando dé `OK`, sigue con esta, que prueba todo el camino: celular → BLE → WiFi → datos del Mega.

Cableado: el mismo de la prueba UART (Mega TX1 pin 18 → divisor 1 kΩ/2 kΩ → GPIO17; GPIO16 → Mega RX1 pin 19; GND común).

## Programas

| Tarjeta | Entorno | Archivo |
|---|---|---|
| Mega | `mega-wifi-ble-prueba` | [`src/mega_pruebas/wifi_ble_uart_prueba.cpp`](../src/mega_pruebas/wifi_ble_uart_prueba.cpp) |
| ESP32-C6 | `esp32-c6-wifi-ble-prueba` | [`src/esp32_pruebas/wifi_ble_uart_prueba.cpp`](../src/esp32_pruebas/wifi_ble_uart_prueba.cpp) |

```
pio run -e mega-wifi-ble-prueba -t upload
pio run -e esp32-c6-wifi-ble-prueba -t upload
```

El firmware de la ESP32 es el de la rama `feature/esp32-mega` con los pines cambiados a GPIO17/16. El monitor de la ESP32 sale por el **USB nativo** (el otro puerto COM de la placa).

## Qué hace cada lado

- **Mega:** manda un JSON simulado (temperatura y pH) cada 500 ms y muestra en su monitor (9600) lo que reporta la ESP32: `#NAME`, `#WIFI`, `#BLE`, `#IP`.
- **ESP32:** se anuncia por BLE como `SENSEI-XXXX`, recibe SSID y contraseña, se conecta al WiFi, levanta el servidor y reenvía por WebSocket el JSON que llega del Mega.

## Pasos de la prueba

1. **UART (Mega → ESP32 y ESP32 → Mega).** En el monitor del Mega, a los pocos segundos debe aparecer cada 5 s algo como:
   `--- JSON enviados: 40 | ESP32: SENSEI-A1B2 | WiFi: UNCONFIGURED | BLE: 0 ---`
   Si dice `SIN RESPUESTA`, la ESP32 no está llegando al Mega (GPIO16 → pin 19 o GND).
2. **BLE.** Con una app como nRF Connect (o la app Android), busca `SENSEI-XXXX` y conéctate. En el monitor del Mega, `BLE` cambia a `1`. Eso confirma BLE y también que la ESP32 sigue avisando al Mega.
3. **WiFi.** Escribe en la característica `WIFI_CRED` (`beb5483e-36e1-4688-b7f5-ea07361b26a9`) el texto `SSID|contraseña` (con nRF Connect usa `|` como separador; la app Android usa salto de línea). La característica `WIFI_STAT` notifica `CONNECTING` y luego `CONNECTED`, y `IP_ADDR` entrega la IP. El monitor del Mega pasa a `WiFi: CONNECTED` y muestra la IP.
   - Debe ser una red de 2.4 GHz.
4. **Datos del Mega por WiFi.** Desde un equipo en la misma red:
   - Navegador: `http://<ip>/data` debe mostrar el JSON con temperatura y pH, distinto en cada recarga.
   - WebSocket: `ws://<ip>/ws` debe recibir un JSON nuevo cada 500 ms.
   Esto prueba el camino completo Mega → UART → ESP32 → WiFi.

## Diagnóstico

| Síntoma | Causa probable |
|---|---|
| Mega: `SIN RESPUESTA` | Falla ESP32 GPIO16 → Mega RX1 (19), o falta GND común |
| `/data` devuelve JSON con los 4 conectores en `false` y no cambia | La ESP32 no recibe del Mega: revisar Mega TX1 (18) → divisor → GPIO17 |
| No aparece `SENSEI-XXXX` en BLE | La ESP32 no arrancó: revisar el monitor del USB nativo |
| `WiFi: FAILED` | Contraseña incorrecta o red de 5 GHz |
| Monitor de la ESP32 en blanco | Usar el otro puerto COM de la placa |
