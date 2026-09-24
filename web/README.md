# Panel de pruebas web

Página local que hace lo mismo que la app Android, para probar el kit sin el celular: busca el kit por Bluetooth, le manda el WiFi y muestra los datos por WebSocket. Flujo definido en [`docs/protocolo.md`](../docs/protocolo.md) y [`docs/plan_implementacion.md`](../docs/plan_implementacion.md).

## Cómo abrirla

Web Bluetooth solo funciona en **Chrome o Edge** y desde `localhost` (no abriendo el archivo con doble clic). Desde la carpeta del proyecto:

```
python -m http.server 8000 --directory web
```

Luego abre http://localhost:8000.

## Cómo usarla

1. **Paso 1:** pulsa **Buscar kit** y elige `SENSEI-XXXX`.
2. **Paso 2:** escribe la red y la contraseña (o pulsa **Buscar redes**) y pulsa **Conectar**.
3. **Aviso:** esta computadora debe estar en la misma red WiFi que el kit. Pulsa **Continuar**.
4. **Paso 3:** aparecen los 4 puertos con su sensor, valor y unidad. Arriba se ve cada cuánto llega un dato (debe rondar 500 ms).

Atajos para probar por partes:

- **Conectar por IP:** salta el Bluetooth si ya conoces la IP del kit.
- **Simular datos:** muestra la interfaz con datos falsos, sin kit.
- **Registro:** al final de la página va todo lo que pasa por Bluetooth y WebSocket.
- El JSON que llega se ve en el desplegable **JSON recibido**.
