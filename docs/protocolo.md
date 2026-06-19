# Protocolo de comunicación — Sensei

Este documento es la referencia definitiva para implementar tanto el firmware del ESP32 como la app Android. Todo lo que está aquí está acordado entre las dos partes. No modificar sin coordinación.

---

## 1. Nombre BLE del kit

El ESP32 se anuncia por Bluetooth con el nombre:

```
SENSEI-XXXX
```

Donde `XXXX` son los últimos 4 caracteres de su dirección MAC en mayúsculas. Ejemplo: `SENSEI-A1B2`.

La app filtra dispositivos BLE con `startsWith("SENSEI-")`.

---

## 2. Perfil GATT — BLE

### Servicio principal

UUID: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`

### Características

| Nombre | UUID | Dirección | Descripción |
|---|---|---|---|
| **WIFI_CRED** | `beb5483e-36e1-4688-b7f5-ea07361b26a9` | App → ESP32 | Credenciales WiFi |
| **WIFI_STAT** | `beb5483e-36e1-4688-b7f5-ea07361b26aa` | ESP32 → App | Progreso de conexión WiFi |
| **IP_ADDR** | `beb5483e-36e1-4688-b7f5-ea07361b26ab` | ESP32 → App | IP asignada por el router |
| **WIFI_SSID** | `243c4f09-ebd7-4026-940a-10afc3c8993e` | ESP32 → App | Nombre de la red WiFi activa |
| **WIFI_STATE** | `4499e77a-31ec-4153-bcde-00e90bb808c9` | ESP32 → App | Estado actual del ESP32 |
| **CCCD** | `00002902-0000-1000-8000-00805f9b34fb` | — | Descriptor estándar para activar notificaciones |

### Detalle de cada característica

**WIFI_CRED** — La app escribe el SSID y la contraseña juntos, separados por `\n`:
```
MiRedWiFi\nMiClave123
```**WIFI_STAT** — El ESP32 notifica el progreso al conectarse. Valores exactos (respetar mayúsculas):
| Valor | Significado |
|---|---|
| `CONNECTING` | El ESP32 está intentando conectarse |
| `CONNECTED` | Conexión WiFi exitosa |
| `FAILED` | La conexión falló |

**IP_ADDR** — El ESP32 notifica su IP como texto plano, sin prefijos:
```
192.168.1.5
```

**WIFI_SSID** — El ESP32 notifica el nombre de la red WiFi a la que está conectado. La app lo usa para mostrar al usuario a qué red debe conectar su celular si no puede alcanzar el WebSocket.

**WIFI_STATE** — La app lee esta característica nada más conectarse por BLE para saber qué pantalla mostrar:
| Valor | Significado | Qué hace la app |
|---|---|---|
| `UNCONFIGURED` | El ESP32 nunca ha tenido credenciales | Muestra pantalla de configuración WiFi |
| `CONNECTED` | El ESP32 está en el WiFi y tiene IP | Lee `IP_ADDR` y `WIFI_SSID`, va directo a los datos |
| `FAILED` | Intentó conectarse pero falló | Muestra pantalla de configuración con mensaje de error |

### Notas importantes

- El ESP32 **mantiene el BLE activo siempre**, incluso después de conectarse al WiFi. Esto permite que cualquier usuario adicional se conecte por BLE para obtener la IP sin reconfigurar nada.
- El ESP32 **no apaga el BLE** después de recibir las credenciales.

---

## 3. Flujo de conexión

### Primer usuario (configura el kit)

1. Abre la app → escanea BLE → selecciona `SENSEI-XXXX`.
2. La app lee `WIFI_STATE` → valor `UNCONFIGURED`.
3. La app muestra pantalla de configuración WiFi.
4. El usuario selecciona su red de la lista del celular y escribe la contraseña.
5. La app escribe `"SSID\nPASSWORD"` en `WIFI_CRED`.
6. El ESP32 notifica `CONNECTING` → `CONNECTED` en `WIFI_STAT`.
7. El ESP32 notifica su IP en `IP_ADDR` y el nombre de la red en `WIFI_SSID`.
8. La app muestra una advertencia antes de continuar:
   > *"Para ver los datos, tu celular debe estar conectado a la red WiFi: **[WIFI_SSID]**"*
   >
   > [ Continuar ]
9. El usuario confirma y presiona **Continuar**.
10. La app se conecta al WebSocket con la IP recibida.
11. Si la conexión falla, la app vuelve a mostrar la advertencia del paso 8.

### Usuarios adicionales (el kit ya está configurado)

1. Abre la app → escanea BLE → selecciona `SENSEI-XXXX`.
2. La app lee `WIFI_STATE` → valor `CONNECTED`.
3. La app lee `WIFI_SSID` y muestra inmediatamente una advertencia antes de continuar:
   > *"Para ver los datos, tu celular debe estar conectado a la red WiFi: **[WIFI_SSID]**"*
   >
   > [ Continuar ]
4. El usuario confirma que está en esa red y presiona **Continuar**.
5. La app lee `IP_ADDR` y se conecta al WebSocket.
6. Si la conexión falla (el celular no estaba en la red correcta), la app vuelve a mostrar la advertencia del paso 3.

---

## 4. Transporte de datos — WebSocket

```
ws://<ip-del-esp32>/ws
```

Puerto TCP: **80**.

- El ESP32 mantiene la conexión abierta.
- El ESP32 empuja datos cada **500ms** automáticamente.
- La app solo escucha, no necesita pedir nada.
- Múltiples celulares pueden conectarse al mismo WebSocket simultáneamente.

---

## 5. Formato JSON de los datos

El Mega manda este JSON al ESP32 por UART cada 500ms. El ESP32 lo reenvía por WebSocket sin modificarlo.

```json
{
  "s": [
    { "p": 1, "c": true,  "id": "ph",      "v": 7.20, "u": "pH" },
    { "p": 2, "c": false },
    { "p": 3, "c": true,  "id": "voltaje", "v": 3.30, "u": "V"  },
    { "p": 4, "c": false }
  ]
}
```

### Campos

| Campo | Tipo | Descripción |
|---|---|---|
| `"s"` | Array | Lista de los 4 puertos RJ45 |
| `"p"` | Número (1–4) | Número de puerto |
| `"c"` | Boolean | `true` si hay sensor conectado, `false` si no |
| `"id"` | String | Identificador del sensor |
| `"v"` | Float | Valor numérico de la lectura |
| `"u"` | String | Unidad de medida |

### Reglas

- Los campos `"id"`, `"v"` y `"u"` solo aparecen cuando `"c"` es `true`.
- `"v"` siempre es número (`float`), nunca string.
- Si `"c"` es `false`, ese objeto solo tiene `"p"` y `"c"`.
- Siempre se mandan los 4 puertos, estén ocupados o no.

### Valores de `"id"` definidos hasta ahora

| `"id"` | Sensor | Unidad |
|---|---|---|
| `"ph"` | Sensor de pH PH-4502C | `"pH"` |
| `"voltaje"` | Sensor de Voltaje AR2657 | `"V"` |
| `"corriente"` | Sensor de Corriente ACS712 | `"A"` |
| `"temperatura"` | Sensor DS18B20 | `"°C"` |
| `"humedad"` | Sensor de Humedad OKY3442 | `"%"` |
| `"co2"` | Sensor de CO2 MG811 | `"ppm"` |
| `"pulso"` | Sensor de Pulso OKY3471-5 | `"bpm"` |
| `"luz"` | Fototransistor PT331C | `""` |
| `"hall"` | Sensor de Efecto Hall SM351LT | `""` |

---

## 6. Intervalo de muestreo

- El Mega manda datos cada **500ms**, siempre, sin parar.
- El ESP32 los reenvía por WebSocket tal cual.
- La app decide con qué frecuencia guarda los datos para el experimento — eso es lógica interna de la app.

---

## 7. Autenticación

Sin autenticación. El WebSocket y el servidor HTTP responden sin credenciales. La red local del aula es suficiente para este contexto.

---

## 8. Gestión de credenciales WiFi

### Comportamiento al encender

El ESP32 **no guarda credenciales** entre sesiones. Cada vez que se apaga, las credenciales se borran. Al encenderse siempre arranca en estado `UNCONFIGURED` y espera que alguien le configure el WiFi por BLE.

Esto garantiza que:
- Ningún alumno puede arruinar la configuración entre clases.
- El kit siempre arranca en un estado predecible.
- No se necesita botón de reset ni lógica de reconfiguración.

### Flujo al inicio de cada clase

1. El profesor enciende el kit — arranca en `UNCONFIGURED`.
2. El profesor abre la app, se conecta por BLE y configura el WiFi.
3. El kit se conecta a la red y queda listo.
4. Los alumnos se conectan por BLE para obtener la IP y ver los datos.

---

## 9. Pendientes

| Tema | Descripción |
|---|---|
| Sensor Hall y Fototransistor en JSON | Estos sensores no dan un float con unidad. Hay que definir cómo representar `"hall"` (detectado/no detectado) y `"luz"` (valor ADC 0–1023) en el JSON antes de implementarlos. |
| Intervalo configurable | Si en el futuro el usuario necesita controlar el intervalo de muestreo desde la app, hay que agregar comunicación de la app hacia el ESP32 por WebSocket. Por ahora el intervalo es fijo. |

