# Práctica 03 — Validación simultánea de pH y temperatura

**Fecha:** _______________
**Responsable:** _______________
**Sensores bajo prueba:** PH-4502C + DS18B20 (kit Sensei, modo 3)
**Sensores de referencia:** pHmetro _______________ | Termómetro digital _______________

---

## Objetivo

Validar estadísticamente ambos sensores del kit Sensei trabajando simultáneamente,
primero en condiciones estáticas (líquidos cotidianos con pH y temperatura registrados)
y después en condiciones dinámicas (reacción efervescente que cambia ambas magnitudes
al mismo tiempo).

---

## Fórmulas

### Notación

| Símbolo | Significado |
|---------|-------------|
| x̄ | Promedio de las 5 lecturas (se lee "x barra") |
| xᵢ | Cada lectura individual (x₁, x₂, x₃, x₄, x₅) |
| n | Número total de lecturas (n = 5) |
| Σ | Suma de todos los términos que le siguen |
| σ | Desviación estándar (se lee "sigma") |
| √ | Raíz cuadrada |
| \|·\| | Valor absoluto — hace que el resultado siempre sea positivo |

**Promedio (x̄):**

> x̄ = (x₁ + x₂ + x₃ + x₄ + x₅) / n

Donde n = 5 (número de lecturas por punto).

**Desviación estándar (σ):**

> σ = √[ Σ(xᵢ − x̄)² / (n − 1) ]

Mide qué tanto se dispersan las lecturas respecto al promedio.
Un valor bajo indica que el sensor es estable y repetible.

**Error absoluto:**

> Error = | x̄ − valor_referencia |

Diferencia entre lo que midió el Sensei y lo que midió el instrumento de referencia.

---

## Materiales

- Kit Sensei con PH-4502C (pin A2) y DS18B20 (pin 7) conectados simultáneamente
- pHmetro digital de referencia (modelo: _______________)
- Termómetro digital de referencia (modelo: _______________)
- Computadora con Serial Monitor abierto a 9600 baudios (escribir `6` en el menú para activar lectura simultánea de pH y temperatura)
- Agua destilada para enjuagar el electrodo de pH entre muestras
- Servilletas o papel absorbente
- Vasos de vidrio anchos (uno por muestra, mínimo 250 ml, para que quepan ambos sensores)
- Para la Parte A:
  - Jugo de limón exprimido
  - Jugo de naranja exprimido
  - Refresco de cola
  - Leche entera
  - Agua de la llave
  - Agua con bicarbonato (1 cucharadita en 250 ml)
- Para la Parte B:
  - 200 ml de agua a temperatura ambiente
  - 1 tableta de antiácido efervescente (Alka-Seltzer, Picot, o similar)
  - Cronómetro

---

## Parte A — Muestras cotidianas (validación estática simultánea)

Medición de seis líquidos cotidianos usando ambos sensores al mismo tiempo.
Para cada muestra se toman **5 lecturas consecutivas** del Serial Monitor
(modo 3: pH y temperatura en la misma línea) y se comparan contra los dos
instrumentos de referencia.

### Procedimiento Parte A

1. Verter la muestra en un vaso ancho.
2. Medir con pHmetro de referencia y termómetro de referencia. Anotar ambos.
3. Enjuagar el electrodo del PH-4502C. Sumergir **ambos sensores** sin tocarse.
4. Esperar **2 minutos** para que se estabilicen.
5. Anotar 5 lecturas consecutivas del Serial Monitor (pH | temperatura).
6. Enjuagar el electrodo antes de la siguiente muestra.

> **Importante:** los sensores no deben tocarse entre sí.
> Solo sumergir el electrodo del PH-4502C, no el módulo electrónico.

---

### Muestra 1 — Jugo de limón (pH esperado: 2.0–3.5)

**pH referencia:** _______________ | **Temperatura referencia:** _______________ °C

| Lectura | pH Sensei | Temperatura Sensei (°C) |
|---------|-----------|-------------------------|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio pH:** _______________ | **Desv. std:** _______________ | **Error pH:** _______________
**Promedio temp.:** _______________ °C | **Desv. std:** _______________ °C | **Error temp.:** _______________ °C

---

### Muestra 2 — Jugo de naranja (pH esperado: 3.5–4.5)

**pH referencia:** _______________ | **Temperatura referencia:** _______________ °C

| Lectura | pH Sensei | Temperatura Sensei (°C) |
|---------|-----------|-------------------------|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio pH:** _______________ | **Desv. std:** _______________ | **Error pH:** _______________
**Promedio temp.:** _______________ °C | **Desv. std:** _______________ °C | **Error temp.:** _______________ °C

---

### Muestra 3 — Refresco de cola (pH esperado: 2.5–3.5)

**pH referencia:** _______________ | **Temperatura referencia:** _______________ °C

| Lectura | pH Sensei | Temperatura Sensei (°C) |
|---------|-----------|-------------------------|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio pH:** _______________ | **Desv. std:** _______________ | **Error pH:** _______________
**Promedio temp.:** _______________ °C | **Desv. std:** _______________ °C | **Error temp.:** _______________ °C

---

### Muestra 4 — Leche entera (pH esperado: 6.5–6.8)

**pH referencia:** _______________ | **Temperatura referencia:** _______________ °C

| Lectura | pH Sensei | Temperatura Sensei (°C) |
|---------|-----------|-------------------------|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio pH:** _______________ | **Desv. std:** _______________ | **Error pH:** _______________
**Promedio temp.:** _______________ °C | **Desv. std:** _______________ °C | **Error temp.:** _______________ °C

---

### Muestra 5 — Agua de la llave (pH esperado: 6.5–8.5)

**pH referencia:** _______________ | **Temperatura referencia:** _______________ °C

| Lectura | pH Sensei | Temperatura Sensei (°C) |
|---------|-----------|-------------------------|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio pH:** _______________ | **Desv. std:** _______________ | **Error pH:** _______________
**Promedio temp.:** _______________ °C | **Desv. std:** _______________ °C | **Error temp.:** _______________ °C

---

### Muestra 6 — Agua con bicarbonato (pH esperado: 8.0–9.0)

**pH referencia:** _______________ | **Temperatura referencia:** _______________ °C

| Lectura | pH Sensei | Temperatura Sensei (°C) |
|---------|-----------|-------------------------|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio pH:** _______________ | **Desv. std:** _______________ | **Error pH:** _______________
**Promedio temp.:** _______________ °C | **Desv. std:** _______________ °C | **Error temp.:** _______________ °C

---

### Tabla resumen Parte A

| # | Muestra | pH ref. | pH Sensei prom. | Error pH | Temp. ref. (°C) | Temp. Sensei prom. (°C) | Error temp. (°C) |
|---|---------|---------|-----------------|----------|-----------------|-------------------------|-----------------|
| 1 | Jugo de limón | | | | | | |
| 2 | Jugo de naranja | | | | | | |
| 3 | Refresco de cola | | | | | | |
| 4 | Leche entera | | | | | | |
| 5 | Agua de llave | | | | | | |
| 6 | Agua + bicarbonato | | | | | | |

**Error absoluto promedio pH Parte A:** _______________ | **Error absoluto promedio temp. Parte A:** _______________ °C

---

## Parte B — Reacción efervescente (validación dinámica simultánea)

Disolución de antiácido efervescente en agua. La reacción sube el pH (~7 → ~8)
y baja ligeramente la temperatura (reacción endotérmica), permitiendo ver ambos
sensores respondiendo simultáneamente a la misma reacción química.

### Procedimiento Parte B

1. Verter 200 ml de agua a temperatura ambiente en el vaso ancho.
2. Enjuagar el electrodo. Sumergir **ambos sensores** sin tocarse.
3. Esperar 2 minutos. Anotar el punto inicial (5 lecturas + referencias).
4. Sin mover los sensores, agregar la tableta efervescente.
5. Cada **60 segundos** anotar 5 lecturas del Serial Monitor + valores de referencia.
6. Continuar hasta que las burbujas se detengan y los valores se estabilicen (~6 min).

---

### Punto B0 — Antes de la tableta

**pH referencia:** _______________ | **Temperatura referencia:** _______________ °C

| Lectura | pH Sensei | Temperatura Sensei (°C) |
|---------|-----------|-------------------------|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio pH:** _______________ | **Error pH:** _______________ | **Promedio temp.:** _______________ °C | **Error temp.:** _______________ °C

---

### Punto B1 — Minuto 1

**pH referencia:** _______________ | **Temperatura referencia:** _______________ °C

| Lectura | pH Sensei | Temperatura Sensei (°C) |
|---------|-----------|-------------------------|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio pH:** _______________ | **Error pH:** _______________ | **Promedio temp.:** _______________ °C | **Error temp.:** _______________ °C

---

### Punto B2 — Minuto 2

**pH referencia:** _______________ | **Temperatura referencia:** _______________ °C

| Lectura | pH Sensei | Temperatura Sensei (°C) |
|---------|-----------|-------------------------|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio pH:** _______________ | **Error pH:** _______________ | **Promedio temp.:** _______________ °C | **Error temp.:** _______________ °C

---

### Punto B3 — Minuto 3

**pH referencia:** _______________ | **Temperatura referencia:** _______________ °C

| Lectura | pH Sensei | Temperatura Sensei (°C) |
|---------|-----------|-------------------------|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio pH:** _______________ | **Error pH:** _______________ | **Promedio temp.:** _______________ °C | **Error temp.:** _______________ °C

---

### Punto B4 — Minuto 4

**pH referencia:** _______________ | **Temperatura referencia:** _______________ °C

| Lectura | pH Sensei | Temperatura Sensei (°C) |
|---------|-----------|-------------------------|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio pH:** _______________ | **Error pH:** _______________ | **Promedio temp.:** _______________ °C | **Error temp.:** _______________ °C

---

### Punto B5 — Minuto 5

**pH referencia:** _______________ | **Temperatura referencia:** _______________ °C

| Lectura | pH Sensei | Temperatura Sensei (°C) |
|---------|-----------|-------------------------|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio pH:** _______________ | **Error pH:** _______________ | **Promedio temp.:** _______________ °C | **Error temp.:** _______________ °C

---

### Punto B6 — Al estabilizarse (sin burbujas)

**pH referencia:** _______________ | **Temperatura referencia:** _______________ °C

| Lectura | pH Sensei | Temperatura Sensei (°C) |
|---------|-----------|-------------------------|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio pH:** _______________ | **Error pH:** _______________ | **Promedio temp.:** _______________ °C | **Error temp.:** _______________ °C

---

### Tabla resumen Parte B — pH

| Punto | Tiempo | pH referencia | pH Sensei promedio | Desv. estándar | Error absoluto |
|-------|--------|---------------|--------------------|----------------|----------------|
| B0 | Inicial | | | | |
| B1 | 1 min | | | | |
| B2 | 2 min | | | | |
| B3 | 3 min | | | | |
| B4 | 4 min | | | | |
| B5 | 5 min | | | | |
| B6 | Final | | | | |

**Error absoluto promedio pH Parte B:** _______________

### Tabla resumen Parte B — Temperatura

| Punto | Tiempo | Temp. referencia (°C) | Temp. DS18B20 promedio (°C) | Desv. estándar (°C) | Error absoluto (°C) |
|-------|--------|-----------------------|-----------------------------|---------------------|---------------------|
| B0 | Inicial | | | | |
| B1 | 1 min | | | | |
| B2 | 2 min | | | | |
| B3 | 3 min | | | | |
| B4 | 4 min | | | | |
| B5 | 5 min | | | | |
| B6 | Final | | | | |

**Error absoluto promedio temperatura Parte B:** _______________ °C

---

## Criterios de evaluación estadística

| Sensor | Criterio | Meta |
|--------|----------|------|
| PH-4502C | Error absoluto en cada punto (A y B) | < 0.3 pH |
| PH-4502C | Error absoluto promedio total | < 0.3 pH |
| PH-4502C | Desviación estándar por punto | < 0.15 pH |
| PH-4502C | Orden correcto de muestras Parte A | Sí / No |
| DS18B20 | Error absoluto en cada punto (A y B) | ≤ 0.5 °C |
| DS18B20 | Error absoluto promedio total | ≤ 0.5 °C |
| DS18B20 | Desviación estándar por punto | < 0.1 °C |
| Ambos | Registran cambio durante la reacción (Parte B) | Sí / No |

---

## Observaciones

_(Anotar si los sensores se afectaron mutuamente, si el cambio de temperatura
fue visible durante la reacción, cuál sensor respondió más rápido al cambio,
diferencias entre Parte A y Parte B, cualquier lectura atípica.)_
