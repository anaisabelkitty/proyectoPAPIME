# Guía de ejecución — Sensor de pH PH-4502C

Esta guía explica qué hace cada archivo de código del sensor de pH,
cuándo se usa cada uno y cómo ejecutarlo en PlatformIO.

---

## Archivos del sensor de pH

```
PAPIME/
├── src/
│   ├── main.cpp                          ← programa activo (el que corre en el Arduino)
│   └── sensores_analogicos/ph/
│       ├── ph.h                          ← constantes y valores de calibración
│       └── ph.cpp                        ← lógica de lectura y cálculo de pH
└── extras/ph/
    ├── calibracion_bnc.cpp               ← herramienta: ajustar offset del módulo
    └── calibracion_buffer.cpp            ← herramienta: calibrar con soluciones buffer
```

---

## Requisitos previos

- PlatformIO instalado en VS Code
- Arduino Mega 2560 conectado por USB
- Módulo PH-4502C conectado:
  - VCC → 5V del Mega
  - GND → GND del Mega
  - Po  → A2 del Mega

---

## Programa 1 — Medición normal (uso cotidiano)

**Archivo activo:** `src/main.cpp`

**Qué hace:** lee el voltaje del sensor cada segundo, lo convierte a pH usando
los valores de calibración de `ph.h`, e imprime el resultado en el Serial Monitor.

**Cuándo usarlo:** siempre que quieras medir el pH de una solución.
Este es el programa que debe estar en `main.cpp` durante el uso normal del kit.

**Cómo ejecutarlo:**

1. Asegúrate de que `src/main.cpp` tiene el contenido del programa de medición
   (no el de calibración).
2. Conecta el electrodo E201C-BNC al conector BNC del módulo.
3. En PlatformIO, haz clic en **Upload** (flecha →) o presiona `Ctrl+Alt+U`.
4. Abre el Serial Monitor con `Ctrl+Alt+M` (velocidad: 9600 baudios).
5. Sumerge el electrodo en la solución a medir y espera 2 minutos.

**Salida esperada en el Serial Monitor:**
```
=========================================
  Sensor de pH PH-4502C
=========================================

Voltaje: 2.502 V  |  pH: 7.00
Voltaje: 2.501 V  |  pH: 7.01
Voltaje: 2.504 V  |  pH: 6.99
```

---

## Programa 2 — Calibración de offset (mantenimiento)

**Archivo:** `extras/ph/calibracion_bnc.cpp`

**Qué hace:** lee el voltaje del pin Po continuamente e indica si el
potenciómetro POT2 está bien ajustado (debe dar 2.50V con el BNC cortocircuitado).

**Cuándo usarlo:** solo si alguien movió el potenciómetro POT2 accidentalmente
y el sensor empieza a dar valores incorrectos. No se necesita recalibrar los
buffers después de este ajuste.

**Cómo ejecutarlo:**

1. **Desconecta el electrodo** del conector BNC.
2. **Cortocircuita el BNC:** conecta un trozo de cable entre el pin central
   (interior del conector) y el anillo exterior metálico.
3. Copia el contenido de `extras/ph/calibracion_bnc.cpp` a `src/main.cpp`
   (guarda el original primero).
4. Sube al Arduino con **Upload**.
5. Abre el Serial Monitor a 9600 baudios.
6. Gira **POT2** (el potenciómetro MÁS CERCANO al BNC) hasta ver `✔ CORRECTO`.
7. Deja el potenciómetro en esa posición.
8. Retira el puente del BNC y vuelve a conectar el electrodo.
9. Restaura `src/main.cpp` con el programa de medición normal.

**Salida esperada en el Serial Monitor:**
```
  Gira POT2 (el MAS CERCANO al BNC)
  hasta obtener exactamente 2.50V.

Voltaje Po: 2.501 V  →  ✔ CORRECTO  (POT2 listo)
Voltaje Po: 2.499 V  →  ✔ CORRECTO  (POT2 listo)
```

---

## Programa 3 — Calibración con soluciones buffer

**Archivo:** `extras/ph/calibracion_buffer.cpp`

**Qué hace:** guía al usuario para medir dos soluciones buffer de pH conocido,
calcula la sensibilidad real del módulo usando interpolación de Lagrange, e
imprime los dos valores que hay que copiar a `ph.h`.

**Cuándo usarlo:**
- Después de ajustar el offset (Programa 2).
- Si el sensor empieza a dar lecturas incorrectas aunque el offset esté bien.
- Cada 2–4 semanas si se usa frecuentemente.
- Siempre que cambies el electrodo por uno nuevo.

**Materiales necesarios:**
- Solución buffer pH 7.00
- Solución buffer pH 4.01 (para muestras ácidas) **o** pH 10.01 (para muestras alcalinas)
- Agua destilada para enjuagar

**Cómo ejecutarlo:**

1. Copia el contenido de `extras/ph/calibracion_buffer.cpp` a `src/main.cpp`.
2. Sube al Arduino con **Upload**.
3. Abre el Serial Monitor a 9600 baudios.
4. Escribe **A** y presiona Enter (para buffers pH 4.01 + pH 7.00).
5. Sigue las instrucciones paso a paso en el monitor.
6. Al terminar, el sketch imprime dos valores como estos:

```
  Copia estos valores en ph.h:

    const float PH_OFFSET_PH7   = 2.5018;
    const float PH_SENSIBILIDAD = 0.1771;
```

7. Abre `src/sensores_analogicos/ph/ph.h` y reemplaza los valores.
8. Restaura `src/main.cpp` con el programa de medición normal.

---

## Valores actuales de calibración (en `ph.h`)

```cpp
const float PH_OFFSET_PH7   = 2.5018;  // Voltaje medido en buffer pH 7
const float PH_SENSIBILIDAD = 0.1771;  // V/pH (Lagrange con pH 4.01 y pH 7.00)
```

Calibración realizada el: _______________

---

## Flujo de trabajo resumido

```
¿Primer uso o alguien movió el POT2?
    └─→ Ejecutar Programa 2 (calibracion_bnc.cpp)
        └─→ Ejecutar Programa 3 (calibracion_buffer.cpp)
            └─→ Copiar valores a ph.h
                └─→ Usar Programa 1 (main.cpp) para medir

¿Uso normal?
    └─→ Solo Programa 1 (main.cpp)

¿Cada 2–4 semanas?
    └─→ Ejecutar Programa 3 (calibracion_buffer.cpp)
        └─→ Copiar valores nuevos a ph.h
```

---

## Notas importantes

- **No mezcles los programas:** nunca dejes `calibracion_bnc.cpp` o
  `calibracion_buffer.cpp` como `main.cpp` en producción.
- **El orden importa:** siempre ajusta el offset (Programa 2) ANTES de
  calibrar con buffers (Programa 3).
- **POT2 vs POT1:** en nuestro módulo físico, el potenciómetro que afecta
  la lectura analógica es el MÁS CERCANO al BNC (marcado como POT2 en el
  pinout de UNIT Electronics). El otro (POT1) controla solo la salida digital DO.
