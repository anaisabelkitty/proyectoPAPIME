# Guía de ejecución — Sensor de pH PH-4502C

Esta guía explica cómo usar el sensor de pH desde el menú integrado del kit Sensei.
Ya no es necesario copiar archivos manualmente — todo se maneja desde el
Serial Monitor a través de las opciones del menú en `src/main.cpp`.

---

## Archivos del sensor de pH

```
PAPIME/
├── src/
│   ├── main.cpp                          ← menú principal del kit (todos los sensores)
│   └── sensores_analogicos/ph/
│       ├── ph.h                          ← constantes y calibración (EEPROM)
│       └── ph.cpp                        ← lectura de voltaje y cálculo de pH
└── extras/ph/
    ├── calibracion_bnc.cpp               ← referencia (ya integrado como opción 3)
    └── calibracion_buffer.cpp            ← referencia (ya integrado como opción 4)
```

> Los archivos en `extras/ph/` se conservan como referencia histórica.
> La funcionalidad equivalente ya está integrada en el menú del `main.cpp`.

---

## Requisitos previos

- PlatformIO instalado en VS Code
- Arduino Mega 2560 conectado por USB
- Módulo PH-4502C conectado:
  - VCC → 5V del Mega
  - GND → GND del Mega
  - Po  → A2 del Mega
  - Electrodo E201C-BNC → conector BNC del módulo

---

## Menú del kit (Serial Monitor a 9600 baudios)

Al subir `main.cpp` y abrir el Serial Monitor, aparece el menú principal:

```
=========================================
  Kit Sensei
=========================================
  1  →  Leer pH en tiempo real
  2  →  Leer temperatura en tiempo real
  3  →  Calibrar offset BNC (POT2)
  4  →  Calibrar con soluciones buffer
  5  →  Leer humedad de suelo
  6  →  pH + Temperatura simultaneos
  7  →  Calibrar humedad de suelo (2 puntos)
  0  →  Volver al menu
```

---

## Opción 1 — Leer pH en tiempo real

**Cuándo usarlo:** para medir el pH de cualquier solución en el uso cotidiano.

**Cómo:**
1. Subir `main.cpp` al Arduino.
2. Abrir Serial Monitor a 9600 baudios.
3. Escribir `1` y presionar ENTER.
4. Sumergir el electrodo y esperar 2 minutos.

**Salida esperada:**
```
Voltaje: 2.502 V  |  pH: 7.00
Voltaje: 2.501 V  |  pH: 7.01
```

Los valores de calibración (offset y sensibilidad) se cargan automáticamente
de la EEPROM al encender el Arduino. Si no hay calibración guardada,
se usan los valores por defecto de `ph.h`.

---

## Opción 3 — Calibrar offset BNC (ajuste del POT2)

**Cuándo usarlo:** solo si alguien movió el POT2 accidentalmente y el sensor
empieza a dar valores incorrectos.

**Procedimiento:**
1. Desconectar el electrodo del BNC.
2. Cortocircuitar el BNC con un trozo de cable pelado (pin central ↔ anillo exterior).
3. Escribir `3` en el menú y presionar ENTER.
4. Girar **POT2** (el MÁS CERCANO al BNC) hasta ver `✔ CORRECTO`.
5. Retirar el puente y volver a conectar el electrodo.
6. Escribir `0` para volver al menú.

---

## Opción 4 — Calibrar con soluciones buffer (Lagrange)

**Cuándo usarlo:**
- Primer uso del sensor.
- Si el sensor da lecturas incorrectas aunque el offset esté bien.
- Cada 2–4 semanas si se usa frecuentemente.
- Al cambiar el electrodo.

**Materiales:**
- Solución buffer pH 7.00
- Solución buffer pH 4.01 (muestras ácidas) **o** pH 10.01 (muestras alcalinas)
- Agua destilada para enjuagar

**Procedimiento:**
1. Escribir `4` y presionar ENTER.
2. Escribir `A` (pH 4.01 + pH 7.00) o `B` (pH 7.00 + pH 10.01) y ENTER.
3. Sumergir la sonda en pH 7.00, esperar 2 minutos, presionar ENTER.
4. Enjuagar con agua destilada.
5. Sumergir en el segundo buffer, esperar 2 minutos, presionar ENTER.
6. El kit calcula los valores con Lagrange y los guarda automáticamente
   en la EEPROM. El modo 1 (leer pH) los usa de inmediato.

**La EEPROM persiste aunque se apague el Arduino.** No es necesario volver
a calibrar cada vez que se enciende, a menos que los valores empiecen a
desviarse.

---

## Orden correcto de calibración

```
¿Primer uso o alguien movió el POT2?
    └─→ Opción 3 (calibrar offset BNC)
        └─→ Opción 4 (calibrar con buffers)
            └─→ Opción 1 para medir

¿Uso normal después de calibrar?
    └─→ Solo opción 1

¿Lecturas incorrectas o recalibración periódica?
    └─→ Opción 4 (no es necesario repetir la opción 3
        a menos que el POT2 se haya movido)
```

---

## Calibración actual activa

Al abrir el Serial Monitor, el menú muestra los valores de calibración activos:

```
  pH calibracion: offset=2.5018  sens=0.1771
  (calibracion guardada en EEPROM)
```

Si dice "valores por defecto", significa que aún no se ha hecho la calibración
con buffers en este Arduino. Se recomienda calibrar antes de usar.
