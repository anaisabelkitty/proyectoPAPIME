# Diseño electrónico — Kit Sensei

## Objetivo de este documento

Este documento define cómo se desarrolla el diseño electrónico del kit, desde la protoboard hasta el esquemático en software. Primero se valida cada circuito de forma física y, una vez comprobado que funciona, se documenta en KiCad. No se diseña nada en software antes de haberlo probado en protoboard.

Este documento no repite lo que ya está en [`proyecto.md`](proyecto.md) (arquitectura general del kit) ni en [`sensores.md`](sensores.md) (fórmulas y datasheets de cada sensor). Enlaza a ellos cuando corresponde.

---

## Núcleo central del kit

El núcleo central tiene los siguientes componentes fijos:

- **Arduino Mega 2560:** lee todos los sensores conectados y controla el LCD.
- **ESP32 NodeMCU:** recibe los datos del Mega por UART y los transmite por WiFi/BLE.
- **4 conectores RJ45 hembra:** uno por cada sensor que se puede conectar al mismo tiempo. Van montados en el núcleo central.

Cada módulo sensor lleva también su propio **conector RJ45 hembra**. La conexión entre el núcleo central y cada sensor se hace con un cable de red categoría 5 (Cat5), ponchado con un conector RJ45 macho en cada extremo, en estándar **T568B**, cable directo (mismo orden de pines en ambas puntas, no cruzado).

---

## Alimentación

La batería actual es la **Steren, recargable, ion de litio, tipo 9V, 3600 mWh (400 mAh)**. Se conecta al pin VIN del Mega, que acepta de 7 a 12 V y los regula internamente a 5 V.

El kit también debe poder funcionar conectado a corriente eléctrica al mismo tiempo que la batería está puesta, sin que se dañen entre sí. Esto sigue pendiente (ver sección de pendientes).

---

## Display LCD 20×4

El display es un **LCD 20×4, fondo azul**, controlado por un módulo adaptador I2C (chip **PCF8574**). Ese adaptador es justamente lo que reduce la comunicación con el LCD a **2 pines de datos**: SDA y SCL, en vez de los 6 a 11 pines que necesitaría un LCD conectado en modo paralelo directo.

### Conexión al Arduino Mega 2560

El módulo adaptador I2C tiene 4 terminales:

- **VCC** → pin **5V** del Mega.
- **GND** → pin **GND** del Mega.
- **SDA** (datos) → pin **20** del Mega (SDA en el Mega 2560).
- **SCL** (reloj) → pin **21** del Mega (SCL en el Mega 2560).

``` 
LCD 20x4 + PCF8574          Arduino  Mega 2560
      VCC ───────────────────► 5V
      GND ───────────────────► GND
      SDA ───────────────────► pin 20 (SDA)
      SCL ───────────────────► pin 21 (SCL)
```

El Mega 2560 tiene pines I2C dedicados (20 y 21), separados de los pines analógicos y digitales normales, así que esta conexión no compite por pines con los sensores ni con el sistema de identificación.

### Librería

Se usa la librería **LiquidCrystal_I2C** (o `hd44780` con la clase `hd44780_I2Cexp`, que es más actualizada y detecta automáticamente algunos parámetros del adaptador). Antes de programarlo hay que confirmar la **dirección I2C** del módulo adaptador, porque varía según el fabricante del PCF8574 (las más comunes son `0x27` y `0x3F`). Esto se obtiene corriendo un sketch de "I2C Scanner" (código estándar y muy usado, disponible en la documentación de la librería Wire de Arduino) antes de programar el LCD definitivo.

### Cómo se arma físicamente

1. Soldar o verificar que el adaptador I2C ya viene soldado en la parte trasera del LCD (la mayoría de los LCD 20x4 se compran con el adaptador ya integrado).
2. Conectar los 4 cables (VCC, GND, SDA, SCL) del adaptador a la protoboard del núcleo, y de ahí al Mega según el diagrama de arriba.
3. Correr el I2C Scanner para obtener la dirección exacta del módulo.
4. Con la dirección confirmada, correr un sketch de prueba simple (imprimir "Hola" en la primera línea) para validar que el LCD enciende y muestra texto antes de integrarlo al `main.cpp` del kit.

### Pendiente

- Confirmar la dirección I2C real del adaptador que se está usando (se obtiene con el scanner, no se puede asumir).
- Definir el contraste del LCD (la mayoría de los adaptadores traen un potenciómetro pequeño para ajustarlo; si el texto no se ve, es lo primero que hay que revisar).

---

## Sensores analógicos y digitales: un solo pin de señal por conector

Los pines A0 a A15 del Mega funcionan tanto en modo analógico (`analogRead()`) como en modo digital (`digitalRead()`, o protocolos como 1-Wire). Por eso no hace falta un pin analógico y un pin digital separados por conector: **un solo pin de señal por conector es suficiente**, y el Mega decide cómo leerlo (analógico o digital) según lo que ya le dijo el código de identificación.

Asignación de los pines de señal del Mega, uno por conector:

| Conector | Pin de señal en el Mega |
|---|---|
| Conector 1 | A0 |
| Conector 2 | A1 |
| Conector 3 | A2 |
| Conector 4 | A3 |

### Caso especial: sensores que necesitan dos líneas de señal

El sensor ultrasónico **HC-SR04** (todavía sin programar) necesita dos pines de datos para funcionar: **Trig** (dispara el pulso) y **Echo** (recibe el rebote). No se puede leer con un solo pin de señal, a diferencia de los demás sensores del proyecto. Por eso el presupuesto de pines del conector (ver siguiente sección) reserva **dos pines de señal por conector**, aunque la mayoría de los sensores solo use uno.

Otro caso distinto (no es lo mismo que el anterior): los módulos de pH-4502C, OKY3442 (humedad) y MG811 (CO2) traen de fábrica una salida analógica y una salida digital de umbral (alarma). El proyecto, tal como está programado, **solo usa la salida analógica de esos tres sensores**; la salida digital de umbral no se conecta al RJ45. Esto es una decisión de diseño, no una limitación técnica.

---

## Identificación del sensor conectado

### Decisión final: código digital binario con pull-up interno

Cada sensor tiene asignada una combinación fija de HIGH/LOW en 4 pines digitales del conector. El Mega no mide un voltaje exacto, solo distingue si cada pin está en HIGH o en LOW, lo cual es tolerante a variaciones de resistencia, resistencia de contacto del RJ45 y ruido del cable. Con 4 pines se cubren hasta 16 combinaciones (2⁴), más que suficiente para los sensores del proyecto.

Se usa el pull-up interno del Mega en vez de resistencias externas hacia VCC, porque el ATmega2560 no tiene pull-down interno, solo pull-up. Esto simplifica el circuito de cada sensor: solo se necesitan resistencias hacia GND, y únicamente en los bits que van en 0.

**En el Mega**, cada pin de ID de un conector se configura así:

```cpp
pinMode(pinID0, INPUT_PULLUP);
pinMode(pinID1, INPUT_PULLUP);
pinMode(pinID2, INPUT_PULLUP);
pinMode(pinID3, INPUT_PULLUP);
```

Sin nada conectado, el pin lee HIGH por defecto, que corresponde a **bit en 0** en la tabla de códigos.

**En cada módulo sensor:**
- Bit en 0 (tabla) = eléctricamente HIGH: la línea de ID correspondiente no se conecta a nada dentro del módulo.
- Bit en 1 (tabla) = eléctricamente LOW: la línea de ID correspondiente se conecta a GND a través de una resistencia de 1 kΩ (la resistencia es protección, para no hacer un corto directo).

```
Bit en 1 (tabla) = LOW:            Bit en 0 (tabla) = HIGH:
  Línea de ID ──[1kΩ]── GND         Línea de ID ── (no conectada)
```

**Lectura en el Mega:**

```cpp
int leerCodigoID(int pinID0, int pinID1, int pinID2, int pinID3) {
    int b0 = !digitalRead(pinID0);
    int b1 = !digitalRead(pinID1);
    int b2 = !digitalRead(pinID2);
    int b3 = !digitalRead(pinID3);
    return (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;
}
```

Se invierte la lectura con `!` porque LOW es el estado que el módulo fuerza a propósito (conectando a GND) para marcar un bit en 1 según la tabla de códigos; hay que mantener esta convención sin cambiarla al implementar, para no confundir el nivel eléctrico (HIGH/LOW) con el valor lógico de la tabla (0/1).

El código resultante (0 a 15) se compara contra una tabla en el firmware para saber qué sensor está conectado en ese conector.

### Tabla de códigos (provisional, sujeta a la lista final de sensores)

| Sensor | ID0 | ID1 | ID2 | ID3 | Código |
|---|---|---|---|---|---|
| Voltaje AR2657 | 0 | 0 | 0 | 0 | 0 |
| Corriente ACS712 | 1 | 0 | 0 | 0 | 1 |
| pH PH-4502C | 0 | 1 | 0 | 0 | 2 |
| Fototransistor PT331C | 1 | 1 | 0 | 0 | 3 |
| Humedad OKY3442 | 0 | 0 | 1 | 0 | 4 |
| CO2 MG811 | 1 | 0 | 1 | 0 | 5 |
| Pulso OKY3471-5 | 0 | 1 | 1 | 0 | 6 |
| Hall SM351LT | 1 | 1 | 1 | 0 | 7 |
| Temperatura DS18B20 | 0 | 0 | 0 | 1 | 8 |
| Ultrasónico HC-SR04 | 1 | 0 | 0 | 1 | 9 |

Faltan por asignar los sensores restantes hasta completar la lista final (ver pendientes: aún no está confirmado si el proyecto cierra en 12 o en 16 sensores).

### Pines del Mega dedicados a esto

Cada conector necesita 4 pines digitales solo para ID. Con 4 conectores son 16 pines digitales dedicados a identificación, más los pines de señal. El Mega tiene 54 pines digitales en total, así que esto no es una limitación — el límite real está en los 8 pines del cable Cat5, no en el Mega.

Asignación fija de pines por conector. Señal 1 y Señal 2 son las dos pines Ax (decisión final: aunque Señal 2 hoy solo la use el HC-SR04, se deja como Ax para no tener que recablear si algún sensor futuro la necesita como analógica).

**Convención de orden de los pines de ID (fija, no cambiar):** en el RJ45, de izquierda a derecha, va **ID3, ID2, ID1, ID0** — así el binario se lee directo en el conector, en el mismo orden en que se escribe (bit de mayor valor primero), sin tener que invertir nada mentalmente:

| Conector | ID3 (RJ45 pin 3) | ID2 (RJ45 pin 4) | ID1 (RJ45 pin 5) | ID0 (RJ45 pin 6) | Señal 1 (Ax) | Señal 2 (Ax) |
|---|---|---|---|---|---|---|
| Conector 1 | 22 | 23 | 24 | 25 | A0 | A1 |
| Conector 2 | 26 | 27 | 28 | 29 | A2 | A3 |
| Conector 3 | 30 | 31 | 32 | 33 | A4 | A5 |
| Conector 4 | 34 | 35 | 36 | 37 | A6 | A7 |

Con esto se usan 8 de los 16 pines Ax del Mega (quedan A8-A15 libres), y los pines de ID quedan consecutivos del 22 al 37 sin huecos.

(Pin 7 sigue libre para el DS18B20 si se prueba suelto sin RJ45, y el pin 22 en la tabla anterior de `sensores.md` para el sensor Hall queda liberado, porque con este sistema el sensor Hall ya no tiene un pin fijo, se detecta por su código.)

### Cómo armarlo ahora mismo, paso a paso

Esto es lo que se hace hoy, con el conector 1 como ejemplo, para dejar la identificación funcionando antes de meter cualquier sensor real.

**1. Preparar el lado del núcleo.**
En la protoboard del núcleo, tomar 4 jumpers y conectar el RJ45 hembra del conector 1 así:
- Pin 3 del RJ45 (ID3) → pin digital 22 del Mega.
- Pin 4 del RJ45 (ID2) → pin digital 23 del Mega.
- Pin 5 del RJ45 (ID1) → pin digital 24 del Mega.
- Pin 6 del RJ45 (ID0) → pin digital 25 del Mega.

No poner ninguna resistencia en este lado. El pull-up lo activa el código, no el cableado.

**2. Subir un sketch de prueba (solo para probar ID, sin sensores todavía).**

```cpp
#include <Arduino.h>

const int PIN_ID3 = 22;
const int PIN_ID2 = 23;
const int PIN_ID1 = 24;
const int PIN_ID0 = 25;

void setup() {
    Serial.begin(9600);
    pinMode(PIN_ID3, INPUT_PULLUP);
    pinMode(PIN_ID2, INPUT_PULLUP);
    pinMode(PIN_ID1, INPUT_PULLUP);
    pinMode(PIN_ID0, INPUT_PULLUP);
}

void loop() {
    int b3 = !digitalRead(PIN_ID3);
    int b2 = !digitalRead(PIN_ID2);
    int b1 = !digitalRead(PIN_ID1);
    int b0 = !digitalRead(PIN_ID0);
    int codigo = (b3 << 3) | (b2 << 2) | (b1 << 1) | b0;

    Serial.print("Codigo ID: ");
    Serial.println(codigo);
    delay(500);
}
```

Con nada conectado en el otro extremo del cable, este sketch debe imprimir `0` de forma constante (los 4 bits en HIGH por el pull-up, que se invierten a 0 en el código). Este es el primer punto de control: si no imprime 0 con el conector vacío, hay un problema de cableado antes de seguir.

**3. Armar el lado del módulo con un código de prueba, por ejemplo el código 5 (binario 0101).**
Según la tabla de códigos, el código 5 corresponde a: ID3=0, ID2=1, ID1=0, ID0=1. Recordando la convención (bit en 0 = se conecta a GND con resistencia; bit en 1 = no se conecta a nada):
- ID3 (bit 0): conectar a GND con una resistencia de 1 kΩ.
- ID2 (bit 1): no conectar.
- ID1 (bit 0): conectar a GND con una resistencia de 1 kΩ.
- ID0 (bit 1): no conectar.

En la protoboard del módulo, el RJ45 hembra del módulo lleva 2 resistencias de 1 kΩ (en los pines 3 y 5, que son ID3 e ID1) hacia el riel de GND del módulo. Los pines 4 y 6 (ID2 e ID0) se dejan sin ningún cable.

**4. Conectar el cable Cat5 entre los dos RJ45 y revisar el monitor serial.**
Debe imprimir `5`. Si imprime otro número, revisar primero que las resistencias estén realmente conectadas a GND (medir continuidad con multímetro si hay duda) y que el cable esté ponchado igual en ambas puntas (T568B, no cruzado).

**5. Una vez que el código 5 se lee bien, repetir con otro código (por ejemplo 0, todo en HIGH) para confirmar que el sistema distingue correctamente entre distintos sensores, no solo que "algo enciende".**

Con estos 5 pasos queda validada la identificación de un conector. Después se repite exactamente igual para los otros 3 conectores, cambiando solo los números de pin según la tabla de asignación.

### Alternativas descartadas por ahora (quedan como respaldo)

Se evaluaron otras dos formas de resolver la identificación, ambas usando solo 1 pin en vez de 4, pero se dejan como opción de respaldo por si en algún momento el presupuesto de 8 pines del RJ45 ya no alcanza (por ejemplo, si aparece un sensor que necesite una tercera línea de señal):

- **Chip de identificación 1-Wire (DS2401):** cada módulo lleva un chip con número de serie único de fábrica, leído por el mismo protocolo 1-Wire que ya usa el DS18B20. Usa 1 solo pin, identificación exacta, pero hay que comprarlo y no se ha confirmado disponibilidad con los proveedores (UNIT Electronics, AG Electrónica).
- **Escalera de resistencias R-2R:** red de resistencias que codifica los 4 bits en un solo pin analógico, con 16 niveles espaciados de forma pareja en 0–5V (≈333 mV entre niveles con 16 sensores, más margen si son menos). No requiere comprar nada nuevo, pero sigue siendo lectura analógica y depende de la tolerancia de las resistencias.

Se descartó por completo el divisor de voltaje simple original (una resistencia fija por sensor, sin espaciado calculado), por el riesgo de confundir sensores con niveles de voltaje muy cercanos.

---

## Presupuesto final de los 8 pines del cable Cat5 (T568B)

| Pin | Uso |
|---|---|
| 1 | VCC |
| 2 | GND |
| 3 | ID3 |
| 4 | ID2 |
| 5 | ID1 |
| 6 | ID0 |
| 7 | Señal 1 |
| 8 | Señal 2 (solo la usa el HC-SR04; los demás sensores la dejan sin conectar) |

Quedan los 8 pines ocupados, sin pines libres de reserva. Si se llegara a necesitar una tercera línea de señal para algún sensor futuro, hay que migrar el sistema de identificación a una de las dos alternativas de la sección anterior (1 pin en vez de 4) para liberar espacio.

---

## Prueba de identificación con los sensores ya programados

Hoy en `main.cpp` ya están programados 4 sensores: **pH (PH-4502C)**, **humedad de suelo (OKY3442)**, **temperatura (DS18B20)** y **ultrasónico (HC-SR04)**. Sirven como el primer caso real para probar si el sistema de identificación funciona.

**Ya está implementado:** el `main.cpp` actual tiene la **opción 9 del menú**, que escanea los 4 conectores, lee el código de identificación de cada uno y llama a la función correcta del sensor detectado (`ph_calcularPH()`, `hum_calcularHumedad()`, `temp_leerCelsius()` o `ultra_leerDistanciaCM()`, según el código). Ya no es solo el circuito de identificación por separado — la opción 9 muestra el valor real de cada sensor, sin importar en cuál de los 4 conectores esté enchufado. Las opciones 1, 2, 5 y 8 del menú siguen existiendo aparte, y leen cada sensor por su pin fijo de protoboard (sin pasar por RJ45), tal como está documentado en `sensores.md` — sirven para probar un sensor suelto sin armar todo el sistema de conectores.

### Códigos y conexión de cada uno de los 4 sensores

Recordatorio de pines físicos del RJ45 del módulo: **pin 3 = ID3, pin 4 = ID2, pin 5 = ID1, pin 6 = ID0**.

**pH PH-4502C — código 2 (binario 0010: ID3=0, ID2=0, ID1=1, ID0=0)**

- Conexión del sensor (sin cambios respecto a `sensores.md`): VCC → 5V, GND → GND, Po → pin de señal del conector.
- En el RJ45 del módulo: resistencia de 1 kΩ a GND solo en el **pin 5 (ID1)**. Pines 3, 4, 6 sin conectar.
- Señal 2 del conector: no se usa (el pH solo necesita una línea de señal).

**Humedad de suelo OKY3442 — código 4 (binario 0100: ID3=0, ID2=1, ID1=0, ID0=0)**

- Conexión del sensor: VCC → 5V, GND → GND, A0 (salida analógica) → pin de señal del conector.
- En el RJ45 del módulo: resistencia de 1 kΩ a GND solo en el **pin 4 (ID2)**. Pines 3, 5, 6 sin conectar.
- Señal 2: no se usa.

**Temperatura DS18B20 — código 8 (binario 1000: ID3=1, ID2=0, ID1=0, ID0=0)**

- Conexión del sensor: VCC → 5V, GND → GND, DQ → pin de señal del conector, con la resistencia de pull-up de 4.7 kΩ entre DQ y 5V (esta resistencia va aparte, es del protocolo 1-Wire, no tiene que ver con la identificación).
- En el RJ45 del módulo: resistencia de 1 kΩ a GND solo en el **pin 3 (ID3)**. Pines 4, 5, 6 sin conectar.
- Señal 2: no se usa.

**Ultrasónico HC-SR04 — código 9 (binario 1001: ID3=1, ID2=0, ID1=0, ID0=1)**

- Conexión del sensor: Vcc → 5V, GND → GND, Trig → pin de **Señal 1** del conector, Echo → pin de **Señal 2** del conector. Este es el único de los 4 que sí usa las dos líneas de señal.
- En el RJ45 del módulo: resistencia de 1 kΩ a GND en los **pines 3 (ID3) y 6 (ID0)**. Pines 4 y 5 sin conectar.

### Cómo probar que la identificación funciona con estos 4

1. Armar cada uno de los 4 módulos con su RJ45 y sus resistencias de código, según la tabla de arriba (ver también la lista de materiales completa más abajo).
2. Subir el `main.cpp` actual al Mega (ya trae la opción 9 lista, no hace falta ningún sketch aparte).
3. Conectar el módulo de pH al conector 1, entrar al menú y escribir `9`. El monitor serial debe mostrar algo como: `Conector 1 (codigo 2): pH = 7.02`.
4. Sin mover ninguna resistencia, desconectar el módulo de pH del conector 1 y conectarlo al conector 3 (o cualquier otro). Al volver a escanear (opción 9), debe seguir mostrando el código `2` y el valor de pH, ahora bajo "Conector 3". Esto confirma que la identificación no depende de en cuál de los 4 conectores esté el sensor.
5. Repetir con los otros 3 módulos (humedad = código 4, temperatura = código 8, ultrasónico = código 9), probando cada uno en al menos dos conectores distintos.
6. Para probar los 4 al mismo tiempo: conectar los 4 módulos, uno por conector, y correr la opción 9. Debe imprimir las 4 filas seguidas, cada una con su sensor y su valor correcto.
7. Si algún conector no coincide con lo esperado, revisar primero ese conector específico: resistencias de ID mal conectadas en el módulo, cable Cat5 mal ponchado, o el cableado de ese conector en el núcleo.

---

## Lista de materiales para armar y probar los 4 sensores ya programados

Todo lo necesario para conectar pH, humedad, temperatura y ultrasónico al sistema de identificación y probarlos con la opción 9 del menú.

### Núcleo (una sola vez, sirve para los 4 sensores)

| Material | Cantidad |
|---|---|
| Arduino Mega 2560 | 1 |
| Protoboard grande (para el núcleo) | 1 |
| Conector RJ45 hembra | 4 |
| Cable jumper macho-macho | ~30 |
| Cable jumper macho-hembra | ~10 |

### Por cada módulo sensor (× 4 sensores)

| Material | Cantidad por módulo | Total (4 módulos) |
|---|---|---|
| Protoboard chica | 1 | 4 |
| Conector RJ45 hembra | 1 | 4 |
| Cable Cat5 con RJ45 macho ponchado en ambas puntas (T568B, directo) | 1 | 4 |

### Resistencias para el código de identificación (1 kΩ, protección)

| Sensor | Código | Bits en 0 (necesitan resistencia) | Resistencias de 1 kΩ |
|---|---|---|---|
| pH PH-4502C | 2 | ID1 | 1 |
| Humedad OKY3442 | 4 | ID2 | 1 |
| Temperatura DS18B20 | 8 | ID3 | 1 |
| Ultrasónico HC-SR04 | 9 | ID0, ID3 | 2 |

**Total: 5 resistencias de 1 kΩ** (o cualquier valor disponible entre 1 kΩ y 10 kΩ, no es crítico — ver la explicación de por qué en la sección de identificación).

### Resistencia aparte, no relacionada con el código de identificación

| Material | Cantidad | Para qué |
|---|---|---|
| Resistencia de 4.7 kΩ | 1 | Pull-up del bus 1-Wire del DS18B20 (obligatoria para que el sensor funcione; va entre DQ y 5V, no tiene relación con el circuito de ID) |

### Los 4 sensores

- 1× PH-4502C con electrodo E201C-BNC
- 1× OKY3442 (humedad de suelo)
- 1× DS18B20 sumergible
- 1× HC-SR04

### Herramienta

- Ponchadora de RJ45 (crimping tool), si los 4 cables Cat5 no vienen ya armados.
- Pelacables.
- Multímetro (para verificar continuidad de las resistencias a GND si algún código no coincide al probar).

No se necesita batería ni fuente externa para esta prueba: el Mega se alimenta por USB desde la computadora mientras se usa el monitor serial.

---

## Software de diseño: KiCad

Para pasar del circuito probado en protoboard al esquemático se usa **KiCad**: gratis, de código abierto, sin límite de tamaño de esquemático ni de capas, y el más usado en el ámbito universitario. Descarga: https://www.kicad.org/download/

Para esta etapa solo se usa el editor de esquemático (Eeschema). El editor de PCB se deja para más adelante, si se decide fabricar una tarjeta en vez de quedarse en protoboard o perfboard.

Cada sensor se documenta en una hoja de esquemático separada dentro del mismo proyecto de KiCad, más una hoja principal con el Mega, la ESP32, el LCD y la alimentación. Cada hoja se agrega solo después de que el circuito correspondiente ya se probó físicamente.

---

## Plan de trabajo

1. ~~Modificar el código para que el Mega lea el código de 4 bits y detecte automáticamente el sensor conectado.~~ Hecho: opción 9 del menú, cubre pH, humedad, temperatura y ultrasónico.
2. Armar físicamente los 4 módulos (pH, humedad, temperatura, ultrasónico) con su RJ45 y sus resistencias de código, siguiendo la lista de materiales y la guía de esta misma sección.
3. Probar los 4 conectados al mismo tiempo con la opción 9, y confirmar que cada uno se detecta igual sin importar en qué conector esté.
4. Confirmar la lista final de sensores (12 o 16) y completar la tabla de códigos de identificación con los que falten.
5. Capturar en KiCad los circuitos ya probados físicamente, cada uno en su propia hoja.
6. Armar en KiCad la hoja principal (Mega, ESP32, LCD, alimentación) y resolver ahí la alimentación dual (batería + corriente eléctrica).
7. Continuar con el resto de los sensores, uno por uno, repitiendo el mismo procedimiento.

---

## Pendientes

| Pendiente | Depende de |
|---|---|
| Confirmar si el proyecto final tiene 12 o 16 sensores, y cuáles | Definición del equipo del proyecto |
| Completar la tabla de códigos de identificación con todos los sensores | Depende del punto anterior |
| Confirmar la disposición exacta de los 8 pines en el conector físico RJ45 (numeración real del conector, no solo del cable) | Se confirma al armar el primer módulo |
| Alimentación dual (batería + corriente eléctrica) | Se resuelve al armar la hoja principal en KiCad |
| Resistencia de pull-up de 4.7 kΩ del DS18B20 dentro del módulo, no en el núcleo | Se confirma al modularizar el sensor de temperatura |

---

## Referencias

- KiCad: https://www.kicad.org/
- Arquitectura general del kit: [`proyecto.md`](proyecto.md)
- Documentación técnica de sensores: [`sensores.md`](sensores.md)
- Protocolo de comunicación: [`protocolo.md`](protocolo.md)
- Batería Steren BAT-LI-9V USB: https://www.steren.com.mx/bateria-recargable-usb-li-ion-tipo-9v-cuadrada-de-500-mah.html
- ATmega2560 Datasheet (pines con pull-up interno): https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf
