# Prueba: comunicación UART entre el Mega y la ESP32-C6

Guía puntual para comprobar que el cableado Mega ↔ ESP32 está bien. Referencia del diseño: [`proyecto.md`](proyecto.md#conexión-mega--esp32-uart) y [`conexiones.md`](conexiones.md).

El Mega manda `PING <n>` cada segundo y la ESP32 contesta `PONG <n>`. Con eso se comprueban las dos direcciones del cableado por separado:

- Si la ESP32 imprime lo que recibe, **Mega TX1 → ESP32 RX** está bien.
- Si el Mega imprime `OK`, **ESP32 TX → Mega RX1** está bien.

---

## Conexiones

| Desde | Hacia | Nota |
|---|---|---|
| Mega TX1 (pin 18) | Divisor de voltaje → ESP32 GPIO17 (RX) | El divisor baja 5 V a 3.3 V |
| ESP32 GPIO16 (TX) | Mega RX1 (pin 19) | Directo, sin resistencias |
| Mega GND | ESP32 GND | **Obligatorio**: sin GND común no hay comunicación |

Divisor de voltaje en la línea del Mega TX1:

```
Mega TX1 (18) ──[ 1 kΩ ]──┬── ESP32 GPIO17 (RX)
                          │
                       [ 2 kΩ ]
                          │
                         GND
```

Con 1 kΩ arriba y 2 kΩ abajo, 5 V se convierten en 3.33 V. Cada tarjeta se alimenta por su propio USB; no conectes el 5V del Mega al ESP32.

---

## Cargar los programas

Cada tarjeta tiene su entorno de PlatformIO en [`platformio.ini`](../platformio.ini):

| Tarjeta | Entorno | Programa |
|---|---|---|
| Mega | `mega-uart-prueba` | [`src/mega_pruebas/uart_prueba.cpp`](../src/mega_pruebas/uart_prueba.cpp) |
| ESP32-C6 | `esp32-c6-uart-prueba` | [`src/esp32_pruebas/uart_prueba.cpp`](../src/esp32_pruebas/uart_prueba.cpp) |

Desde la terminal del proyecto (con cada tarjeta conectada por su USB):

```
pio run -e mega-uart-prueba -t upload
pio run -e esp32-c6-uart-prueba -t upload
```

Después abre **dos monitores serie**, uno por tarjeta:

```
pio device monitor -e mega-uart-prueba --port COMx
pio device monitor -e esp32-c6-uart-prueba --port COMy
```

Notas sobre los puertos de la ESP32-C6:

- La placa expone dos puertos COM por el mismo cable (CH343 y USB nativo). Para **subir** el programa sirve cualquiera; para **ver el monitor** de esta prueba hay que usar el del USB nativo, porque el programa manda ahí sus mensajes y deja libres los pines GPIO16/17 para el Mega.
- Si un puerto no muestra nada, prueba con el otro COM de la ESP32.
- El Mega usa 9600 baudios en su monitor y la ESP32 usa 115200; la comunicación entre las dos tarjetas va a 115200.

---

## Qué debe verse

**Monitor del Mega**, si todo está bien:

```
#1  OK    recibido: PONG 1
#2  OK    recibido: PONG 2
...
--- Resumen: 10 OK / 0 fallos ---
```

**Monitor de la ESP32**, si todo está bien:

```
Recibido del Mega: "PING 1"
  -> respondido: "PONG 1"
```

---

## Diagnóstico

| Síntoma | Causa probable |
|---|---|
| ESP32 imprime los `PING`, Mega dice "sin respuesta" | Falla la ruta **ESP32 GPIO16 → Mega RX1 (19)**: cable suelto o en el pin equivocado |
| ESP32 dice "Sin datos del Mega", Mega dice "sin respuesta" | Falla la ruta **Mega TX1 (18) → divisor → GPIO17**, o falta el GND común |
| Los dos sin datos aunque revisaste la ruta anterior | TX y RX cruzados de forma inversa: TX debe ir a RX de la otra tarjeta, no a TX |
| Llegan bytes que no son `PING`/`PONG` | Baudios distintos o ruido: revisar que ambos programas sean los de esta prueba y el GND común |
| Solo funciona a ratos | Falso contacto en protoboard o divisor mal armado (medir ~3.3 V en GPIO17 con el Mega mandando) |
| Un puerto COM del monitor de la ESP32 no muestra nada | Usar el otro puerto COM de la placa (el del USB nativo) |

Al reiniciar la ESP32 pueden llegar al Mega unos bytes basura del arranque; es normal y el Mega los descarta antes de cada `PING`.
