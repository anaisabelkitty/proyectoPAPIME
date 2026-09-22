# Panel web de Sensei

Página de prueba que hace lo mismo que la app Android (ver
[`docs/protocolo.md`](../docs/protocolo.md)), para poder probar el kit
sin depender de la app: conecta por Bluetooth, configura el WiFi del
kit y muestra los datos de los sensores en vivo por WebSocket.

Sin frameworks ni build: HTML + CSS + JS planos, usando la
[Web Bluetooth API](https://developer.chrome.com/docs/capabilities/bluetooth)
del navegador.

## Cómo probarla

Web Bluetooth exige un "contexto seguro" (HTTPS o `localhost`), así que
no basta con abrir `index.html` como archivo. Usa el servidor incluido:

```bash
python web/servidor_local.py
```

Abre la URL que imprime (por defecto `http://localhost:8000`) en
**Chrome o Edge** — de escritorio o Android. No funciona en Safari ni
Firefox (no soportan Web Bluetooth).

## Flujo que sigue

1. **Conectar por Bluetooth** — busca dispositivos `SENSEI-XXXX`.
2. **WIFI_STATE** decide la pantalla: si el kit nunca tuvo credenciales
   (`UNCONFIGURED`) muestra el formulario de WiFi; si ya está
   configurado (`CONNECTED`) pasa directo a la advertencia de red.
3. **Configurar WiFi** — opcionalmente escanea redes (`WIFI_SCAN`),
   escribe `SSID\nPASSWORD` en `WIFI_CRED` y sigue el progreso por
   `WIFI_STAT`.
4. **Advertencia de red** — pide confirmar que el navegador esté en la
   misma red WiFi que el kit antes de abrir el WebSocket.
5. **Datos en vivo** — abre `ws://<ip>/ws` y redibuja las 4 tarjetas de
   sensores con cada JSON que llega (también hay un enlace directo a
   `GET /data` para depurar).

## Notas

- Los UUIDs del perfil GATT están fijos en `app.js` — son los mismos
  que ya implementa `src/esp32/main.cpp`. No cambiarlos sin actualizar
  ambos lados.
- El campo `"id": "distancia"` (sensor ultrasónico) todavía no está
  confirmado con el desarrollador de Android — ver la sección 9 de
  `docs/protocolo.md`.
