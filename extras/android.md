# Coordinación con el desarrollador de la app Android

Este documento tiene todo lo que hay que acordar antes de que cada parte empiece a implementar. El objetivo es que la app y el servidor de la ESP32 queden compatibles desde el inicio.

---

## 1. Cómo la app identifica el kit por Bluetooth

Cada unidad del kit aparece en Bluetooth con el nombre **Sensei-XXXX**, donde XXXX son los últimos 4 caracteres de la dirección MAC de la ESP32. Por ejemplo: `Sensei-A1B2`.

**Preguntas:**
- ¿La app filtra los dispositivos BLE buscando nombres que empiecen con `Sensei-`?
- ¿O necesita otro criterio para identificar el kit entre los dispositivos Bluetooth cercanos?

---

## 2. Perfil BLE para pasar las credenciales WiFi

La app se conecta a la ESP32 por BLE y le envía el nombre (SSID) y la contraseña de la red WiFi. Hay que definir exactamente cómo se hace esa comunicación usando el protocolo GATT.

**Propuesta:**

Un servicio BLE personalizado con tres características:

| Elemento | UUID (ejemplo) | Descripción |
|----------|----------------|-------------|
| Servicio WiFi Config | `12345678-1234-1234-1234-123456789abc` | Servicio que agrupa la configuración WiFi |
| Característica SSID | `12345678-1234-1234-1234-123456789001` | La app escribe aquí el nombre de la red |
| Característica Password | `12345678-1234-1234-1234-123456789002` | La app escribe aquí la contraseña |
| Característica Status | `12345678-1234-1234-1234-123456789003` | La ESP32 notifica el resultado: `conectado`, `error_ssid`, `error_password` |

Los UUIDs de arriba son ejemplos. Los reales hay que generarlos con un generador de UUID v4 y acordarlos entre las dos partes antes de implementar.

**Preguntas:**
- ¿Este esquema de 3 características funciona para la app?
- ¿Prefiere enviar SSID y contraseña juntos en una sola característica como JSON (`{"ssid":"nombre","password":"clave"}`) o separados como se propone arriba?
- ¿Necesita alguna característica adicional? Por ejemplo: leer la IP que le asignó el router a la ESP32, o saber si el kit ya tiene credenciales guardadas.

---

## 3. Cómo la app obtiene la IP de la ESP32

Cuando la ESP32 se conecta al WiFi, el router le asigna una IP. La app necesita esa IP para hacer las peticiones HTTP al servidor.

**Opciones:**

- **Opción A — La ESP32 notifica su IP por BLE (recomendada):** después de conectarse al WiFi, la ESP32 escribe su IP en una característica BLE adicional. La app la lee antes de cerrar la conexión Bluetooth. Es el flujo más limpio y automático.
- **Opción B — mDNS:** la ESP32 se anuncia en la red con un nombre fijo como `sensei-a1b2.local`. La app hace peticiones a ese nombre en lugar de a una IP numérica. El problema es que el soporte de mDNS en Android no siempre es confiable.
- **Opción C — IP fija:** se configura una IP estática en la ESP32. Evita el problema pero requiere configuración manual en el router.

**Pregunta:**
- ¿Cuál de las tres opciones es más fácil de implementar en Android?

---

## 4. Formato de los datos del servidor HTTP

Una vez que la ESP32 está en el WiFi, levanta un servidor HTTP. La app hace peticiones a:

```
GET http://<ip-de-la-esp32>/datos
```

**Respuesta del servidor (JSON propuesto):**

```json
{
  "sensei": "Sensei-A1B2",
  "sensores": [
    { "puerto": 1, "nombre": "pH", "valor": 7.20, "unidad": "pH" },
    { "puerto": 3, "nombre": "Temperatura", "valor": 24.50, "unidad": "°C" }
  ]
}
```

Descripción de cada campo:

- `sensei`: nombre del kit (Sensei- + últimos 4 de la MAC). Sirve para que la app sepa a qué unidad está conectada.
- `puerto`: número del conector RJ45 donde está enchufado el sensor (1 a 4).
- `nombre`: nombre del sensor detectado automáticamente por el kit.
- `valor`: la medición actual como número decimal.
- `unidad`: unidad de medida del sensor.
- Solo aparecen los sensores que están enchufados en ese momento. Si no hay ninguno, el arreglo `sensores` viene vacío.

**Preguntas:**
- ¿Este formato es fácil de parsear en Android (con Gson, Moshi u otra librería)?
- ¿Necesita algún campo adicional? Por ejemplo:
  - `timestamp`: fecha y hora de la última lectura.
  - `id`: identificador numérico del tipo de sensor.
  - `estado`: para indicar si el sensor está leyendo bien o tiene algún error.
- ¿El campo `valor` debe ser siempre un número (`float`) o prefiere recibirlo como `string` para manejarlo más fácil en la app?
- ¿Con qué frecuencia va a hacer peticiones al endpoint `/datos`? (cada segundo, cada 2 segundos, etc.)

---

## 5. Autenticación del servidor HTTP

Por ahora el servidor HTTP de la ESP32 no tiene autenticación: cualquier dispositivo en la misma red puede consultar el endpoint `/datos`.

**Preguntas:**
- ¿Para el contexto de uso en aula (red local de la escuela) esto es suficiente?
- ¿O necesita algún mecanismo de autenticación, como un token fijo en el header de la petición?

---

## 6. Resumen de lo que hay que acordar antes de implementar

| Punto | Estado |
|-------|--------|
| Criterio de filtrado de dispositivos BLE por nombre | ⬜ Pendiente |
| UUIDs del perfil GATT (servicio + características) | ⬜ Pendiente |
| Formato del mensaje BLE: SSID y password juntos o separados | ⬜ Pendiente |
| Cómo la app obtiene la IP de la ESP32 (Opción A, B o C) | ⬜ Pendiente |
| Confirmar estructura del JSON | ⬜ Pendiente |
| Campos adicionales en el JSON (timestamp, id, estado, etc.) | ⬜ Pendiente |
| Tipo de dato para `valor`: float o string | ⬜ Pendiente |
| Frecuencia de consulta al endpoint `/datos` | ⬜ Pendiente |
| Autenticación del servidor HTTP | ⬜ Pendiente |
