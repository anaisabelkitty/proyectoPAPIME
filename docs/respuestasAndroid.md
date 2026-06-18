# Respuestas al desarrollador Android — Coordinación ESP32 ↔ App

Este documento responde, de forma clara , cada pregunta de [android](docs/android.md). Está basado en el código ya implementado en la app Android. El objetivo es que el firmware del ESP32 quede 100% compatible con la app desde el inicio.

---

## 1. ¿Cómo identifica la app el kit por Bluetooth?

> **Pregunta original:** ¿La app filtra los dispositivos BLE buscando nombres que empiecen con `Sensei-`? ¿O necesita otro criterio?

**Decisión: sí, por prefijo de nombre — formato `SENSEI-XXXX`.**

La app busca dispositivos Bluetooth cuyo nombre empiece con el prefijo `"SENSEI-"`. El ESP32 debe anunciarse con el nombre:

```
SENSEI-XXXX
```

Donde `XXXX` son los últimos 4 caracteres de su dirección MAC. Por ejemplo: `SENSEI-A1B2`.

**¿Por qué así y no con un nombre fijo?**  
En el aula van a estar encendidos varios kits al mismo tiempo (hasta 6 o más). Si todos se llamaran igual (por ejemplo `SENSEI-ESP32`), la app no podría distinguir cuál es cuál y el alumno no sabría a qué kit se está conectando. Al incluir los últimos 4 de la MAC, cada kit tiene un nombre único y el alumno puede identificar visualmente el suyo.

---

## 2. ¿Cómo se pasan las credenciales WiFi por BLE?

> **Pregunta original:** ¿El esquema de 3 características funciona? ¿Se envían SSID y contraseña juntos o separados?

**Decisión: 4 características, credenciales juntas en un solo mensaje.**

Los UUIDs ya están definidos y acordados. El ESP32 **debe usar exactamente estos mismos UUIDs**, no cambiar ninguno:

| Nombre | UUID | ¿Quién escribe / lee? |
|---|---|---|
| **Servicio principal** | `4fafc201-1fb5-459e-8fcc-c5c9c331914b` | — Agrupa todo — |
| **WIFI_SCAN** | `beb5483e-36e1-4688-b7f5-ea07361b26a8` | App → ESP32 (escribe `"SCAN"` para pedir lista de redes) |
| **WIFI_CRED** | `beb5483e-36e1-4688-b7f5-ea07361b26a9` | App → ESP32 (envía las credenciales) |
| **WIFI_STAT** | `beb5483e-36e1-4688-b7f5-ea07361b26aa` | ESP32 → App (notifica el estado de conexión) |
| **IP_ADDR** | `beb5483e-36e1-4688-b7f5-ea07361b26ab` | ESP32 → App (notifica la IP que le asignó el router) |
| **CCCD (descriptor estándar)** | `00002902-0000-1000-8000-00805f9b34fb` | Necesario para activar las notificaciones BLE |

### ¿Cómo se envían el SSID y la contraseña?

Se envían **juntos en un solo mensaje** a `WIFI_CRED`, separados por un salto de línea (`\n`):

```
MiRedWiFi\nMiClave123
```

El ESP32 lee ese string, encuentra el `\n` y lo parte en dos: lo que está antes es el SSID, lo que está después es la contraseña.

**¿Por qué juntos y no separados?** Simplifica el firmware: un solo write, una sola operación. No hay que esperar dos escrituras ni manejar el orden. Es más difícil que algo falle a la mitad.

### ¿Qué estados notifica el ESP32 durante la conexión?

Una vez que la app envía las credenciales, el ESP32 va notificando el progreso en `WIFI_STAT`. La app espera exactamente estos tres textos (respeta mayúsculas):

| Lo que notifica el ESP32 | Qué significa para la app |
|---|---|
| `CONNECTING` | Muestra "El ESP32 está conectándose al WiFi..." |
| `CONNECTED` | Muestra "WiFi conectado, esperando IP..." |
| `FAILED` | Muestra error y regresa a pedir las credenciales de nuevo |

---

## 3. ¿Cómo obtiene la app la IP del ESP32?

> **Pregunta original:** ¿Opción A (BLE), B (mDNS) o C (IP fija)?

**Decisión: Opción A — el ESP32 notifica su IP por BLE. Ya está implementada.**

Cuando el ESP32 se conecta al WiFi y el router le asigna una IP, la escribe en la característica `IP_ADDR` y la app la recibe automáticamente por notificación BLE. La app entonces valida que el servidor HTTP responda y abre la pantalla principal.

El formato que debe enviar el ESP32 es la IP como texto plano, sin nada más:

```
192.168.1.5
```

**Importante:** no enviar `"IP:192.168.1.5"` ni ningún otro formato. Solo los números y los puntos. La app ignora si recibe un string vacío o `"0.0.0.0"`.

**¿Por qué esta opción y no las otras?**
- **mDNS (Opción B):** en Android el soporte de mDNS no es confiable en todos los dispositivos y versiones. Es una fuente de bugs difíciles de reproducir.
- **IP fija (Opción C):** requiere configurar el router manualmente para cada kit. En un aula eso es inviable.
- **BLE (Opción A):** funciona siempre, es automático, no depende del router ni de la versión de Android, y ya está funcionando en el código.

---

## 4. ¿Cómo recibe la app los datos de los sensores y qué formato tiene el JSON?

> **Pregunta original:** ¿El formato propuesto es fácil de parsear? ¿Se necesitan campos adicionales? ¿`valor` como número o string?

### El transporte — WebSocket, no HTTP polling

La app **no** hace peticiones GET repetidas cada segundo. La comunicación de datos de sensores funciona por **WebSocket**:

```
ws://<ip-del-esp32>/ws
```

Puerto TCP: **80**.

¿Cómo funciona? La app se conecta una vez al WebSocket. El ESP32 le manda automáticamente el último estado al conectar, y luego empuja cada actualización nueva en cuanto llega del hardware. La app solo escucha — no tiene que pedir nada.

**¿Por qué WebSocket y no HTTP GET cada segundo?**  
Con HTTP tendría que hacer una petición, esperar respuesta y repetir — es más lento y gasta más batería. Con WebSocket la conexión queda abierta y el dato llega en cuanto existe, sin que la app tenga que preguntar.

---

### El JSON que manda el ESP32 — formato real del firmware

```bash
ESTO ERA COMO EL ESP32 MANDABA LOS DATOS EL COMO TU LO HABIAS HECHO ANTES
LO PONGO AQUI PARA QUE SEPAS EL QUE YO HAGO CON LO QUE TU MANDABAS AL SERVIDOR
ENTONCES ME IMAGINO QUE ESTO ES LO QUE SE PUEDE CAMBIAR Y PODEMOS MEJORAR
```

El ESP32 recibe los datos del Arduino Mega por puerto serie y los reenvía por WebSocket tal cual. El formato es un array de sensores bajo la llave `"s"`:

```json
{
  "s": [
    { "p": 1, "c": true,  "id": "voltaje",     "v": 3.3,  "u": "V"  },
    { "p": 2, "c": true,  "id": "temperatura",  "v": 24.5, "u": "°C" },
    { "p": 3, "c": false },
    { "p": 4, "c": true,  "id": "corriente",    "v": 0.8,  "u": "A"  }
  ],
  "int": 2000
}
```

Significado de cada campo:

| Campo | Tipo | Descripción |
|---|---|---|
| `"s"` | Array | Lista de los 4 puertos |
| `"p"` | Número (1–4) | Número de puerto RJ45 |
| `"c"` | Boolean | `true` si hay sensor conectado, `false` si no |
| `"id"` | String | Nombre del sensor: `"voltaje"`, `"temperatura"`, `"corriente"`, `"humedad"` |
| `"v"` | Float | Valor numérico de la lectura |
| `"u"` | String | Unidad: `"V"`, `"°C"`, `"A"`, etc. |
| `"int"` | Número | Intervalo de muestreo actual en milisegundos |

Reglas:
- Solo tienen los campos `"id"`, `"v"` y `"u"` los puertos donde `"c"` es `true`.
- `"v"` siempre es número (`float`), nunca string.
- Si `"c"` es `false`, ese puerto no trae más datos.

---

### ¿Qué hace la app Android para leer este JSON?

Cuando llega un mensaje por WebSocket, la app recibe un `String` con el JSON. Lo parsea con `org.json` (incluida en Android sin dependencias extra):

```bash
ESTO NO SE SI TE SIRVA DE ALGO OSEA COMO TAL EL JSON SE QUE HAY MUCHAS COSAS POR CAMBIAR Y AGREGAR COMO
LA PARTE DE LA CALIBRACION PH Y MAS COSITAS QUE VEREMOS EN MAS SENSORES
ENTONCES ESTO ES ALGO QUE NO ESTA BIEN DEFINIDO POR LO MISMO QUE HABRA MUCHOS CAMBIOS 
```

```java
JSONObject root     = new JSONObject(json);
JSONArray  sensores = root.getJSONArray("s");

for (int i = 0; i < sensores.length(); i++) {
    JSONObject sensor = sensores.getJSONObject(i);

    int     puerto    = sensor.getInt("p");
    boolean conectado = sensor.getBoolean("c");

    if (conectado) {
        String nombre = sensor.getString("id"); // "voltaje", "temperatura", etc.
        float  valor  = (float) sensor.getDouble("v");
        String unidad = sensor.getString("u");  // "V", "°C", etc.
        // usar puerto, nombre, valor, unidad en la UI
    }
}
```

Eso es todo lo que se necesita. No hay librería extra, no hay configuración especial.

---

### ¿Se necesitan campos adicionales como `timestamp`, `nombre completo`, `estado` o similar?

**Decisión: no se agregan por ahora. Los campos actuales son suficientes.**

Aquí el razonamiento de cada uno:

- **`timestamp`** — La app recibe el dato en cuanto existe (push por WebSocket). No necesita saber cuándo fue — si llegó, es fresco. Agregarla solo complica el firmware sin aportar nada útil en este contexto.

- **`nombre` completo** — El campo `"id"` ya da el nombre (`"voltaje"`, `"temperatura"`, etc.). La app puede usarlo directamente o mapearlo a un texto más bonito internamente. El ESP32 no necesita mandarlo diferente.

- **`unidad`** — Ya viene incluida en el campo `"u"`. Este campo sí existe y está disponible para usarlo en la UI si se quiere mostrar junto al valor.

- **`estado`** — Para indicar si un sensor está enchufado pero leyendo mal. Por ahora `"c": true/false` es suficiente. Si en el futuro se necesita distinguir entre "sin sensor" y "sensor con error", se puede agregar.

**Conclusión:** el formato actual (`"s"`, `"p"`, `"c"`, `"id"`, `"v"`, `"u"`) cubre todo lo que la app necesita. No hace falta agregar nada más en esta etapa.

---

## 5. ¿El servidor HTTP necesita autenticación?

> **Pregunta original:** ¿Para el aula es suficiente sin autenticación? ¿O se necesita un token?

**Decisión: sin autenticación por ahora.**

El servidor HTTP del ESP32 responde a `GET /data` sin pedir ningún tipo de credencial. La app hace la petición directamente sin headers especiales.

**¿Por qué no ponerla?**  
En el aula, la red es local y controlada. El único "riesgo" es que otro dispositivo en la misma red haga `GET` al endpoint, y lo único que obtendría son lecturas de sensores — no hay datos personales, contraseñas ni información sensible. Agregar autenticación en este contexto agrega complejidad (hay que coordinar el token entre el firmware y la app, guardarlo, mandarlo en cada petición) sin proteger nada que valga la pena proteger.

**¿Cuándo sería buena idea ponerla?**  
Si en algún momento el kit se conecta a una red escolar general (no solo la del aula) donde hay cientos de dispositivos y personas que no son del grupo, un token fijo en el header protegería que cualquiera lea los datos del sensor. Sería un header como:

```
X-Sensei-Token: clave-secreta-fija
```

Es un cambio simple si se necesita en el futuro, pero hoy no aporta lo suficiente para justificarlo.

---

## 6. Resumen — Lo que está acordado

| Punto | Decisión |
|---|---|
| Nombre BLE del kit | ✅ `SENSEI-XXXX` (últimos 4 de la MAC) |
| Filtrado en la app | ✅ `startsWith("SENSEI-")` |
| UUIDs del perfil GATT | ✅ Definidos y acordados (ver sección 2, no modificar) |
| Formato de credenciales BLE | ✅ Un solo write: `"SSID\nPASSWORD"` |
| Estados WiFi que notifica el ESP32 | ✅ `"CONNECTING"` / `"CONNECTED"` / `"FAILED"` |
| Cómo la app recibe la IP | ✅ Notificación BLE en `IP_ADDR` — IP como texto plano sin prefijo |
| Transporte de datos de sensores | ✅ WebSocket `ws://<ip>/ws` puerto 80 (push, no polling) |
| Estructura del JSON | ✅ Array `"s"` con objetos por puerto: `"p"`, `"c"`, `"id"`, `"v"`, `"u"` |
| Tipo de dato `"v"` (valor) | ✅ Número (`float`), nunca string |
| Cómo parsea la app el JSON | ✅ `org.json` — `JSONObject` + `JSONArray`, sin librerías extra |
| Campos extra en el JSON | ✅ No se agregan — el formato actual cubre todo lo necesario |
| Autenticación HTTP | ✅ Sin autenticación — red local de aula es suficiente |
