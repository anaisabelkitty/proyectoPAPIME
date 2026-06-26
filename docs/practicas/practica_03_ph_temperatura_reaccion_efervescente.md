# Práctica 03 — Validación simultánea de pH y temperatura

**Fecha:** _______________
**Responsable:** _______________
**Sensores bajo prueba:** PH-4502C + DS18B20 (kit Sensei, modo 3)
**Sensores de referencia:** pHmetro _______________ | Termómetro digital _______________

---

## Objetivo

Validar estadísticamente ambos sensores del kit Sensei trabajando simultáneamente,
primero en condiciones estáticas (sustancias con pH y temperatura conocidos) y
después en condiciones dinámicas (reacción efervescente que cambia ambas magnitudes
al mismo tiempo).

---

## Materiales

- Kit Sensei con PH-4502C (pin A2) y DS18B20 (pin 7) conectados simultáneamente
- pHmetro digital de referencia (modelo: _______________)
- Termómetro digital de referencia (modelo: _______________)
- Computadora con Serial Monitor abierto a 9600 baudios, **modo 3 activo**
- Agua destilada para enjuagar el electrodo de pH entre muestras
- Servilletas o papel absorbente
- Para la Parte A:
  - Agua destilada a temperatura ambiente
  - Vinagre blanco a temperatura ambiente
  - Agua con bicarbonato a temperatura ambiente (1 cdita en 250 ml)
  - Vaso de vidrio ancho (mínimo 300 ml, para que quepan ambos sensores)
- Para la Parte B:
  - 200 ml de agua a temperatura ambiente
  - 1 tableta de antiácido efervescente (Alka-Seltzer, Picot, o similar)
  - Vaso de vidrio ancho (mínimo 300 ml)
  - Cronómetro

---

## Parte A — Sustancias fijas (validación estática simultánea)

Medición de tres sustancias con pH conocido a temperatura ambiente,
usando ambos sensores al mismo tiempo. Para cada sustancia se toman
**5 lecturas consecutivas** del Serial Monitor (modo 3 muestra pH y temperatura
en la misma línea) y se comparan contra los dos instrumentos de referencia.

### Procedimiento Parte A

1. Verter la sustancia en el vaso ancho.
2. Medir con pHmetro de referencia y termómetro de referencia. Anotar ambos valores.
3. Enjuagar el electrodo del PH-4502C. Sumergir **ambos sensores** en la sustancia.
4. Esperar **2 minutos** para que se estabilicen.
5. Anotar 5 lecturas consecutivas del Serial Monitor (modo 3: pH | temperatura).
6. Enjuagar el electrodo antes de la siguiente sustancia.

---

### Sustancia A1 — Agua destilada (pH esperado: ~7.0 | temperatura ambiente)

**pH referencia:** _______________ | **Temperatura referencia:** _______________ °C

| Lectura | pH Sensei | Temperatura Sensei (°C) |
|---------|-----------|-------------------------|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio pH:** _______________ | **Desv. std pH:** _______________ | **Error pH:** _______________
**Promedio temp.:** _______________ °C | **Desv. std temp.:** _______________ °C | **Error temp.:** _______________ °C

---

### Sustancia A2 — Vinagre blanco (pH esperado: 2.0–3.5 | temperatura ambiente)

**pH referencia:** _______________ | **Temperatura referencia:** _______________ °C

| Lectura | pH Sensei | Temperatura Sensei (°C) |
|---------|-----------|-------------------------|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio pH:** _______________ | **Desv. std pH:** _______________ | **Error pH:** _______________
**Promedio temp.:** _______________ °C | **Desv. std temp.:** _______________ °C | **Error temp.:** _______________ °C

---

### Sustancia A3 — Agua con bicarbonato (pH esperado: 8.0–9.0 | temperatura ambiente)

**pH referencia:** _______________ | **Temperatura referencia:** _______________ °C

| Lectura | pH Sensei | Temperatura Sensei (°C) |
|---------|-----------|-------------------------|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio pH:** _______________ | **Desv. std pH:** _______________ | **Error pH:** _______________
**Promedio temp.:** _______________ °C | **Desv. std temp.:** _______________ °C | **Error temp.:** _______________ °C

---

### Tabla resumen Parte A

| Sustancia | pH ref. | pH Sensei prom. | Error pH | Temp. ref. (°C) | Temp. Sensei prom. (°C) | Error temp. (°C) |
|-----------|---------|-----------------|----------|-----------------|-------------------------|-----------------|
| Agua destilada | | | | | | |
| Vinagre | | | | | | |
| Agua + bicarbonato | | | | | | |

---

## Parte B — Reacción efervescente (validación dinámica simultánea)

Disolución de antiácido efervescente en agua. La reacción cambia el pH (~7 → ~8)
y la temperatura (reacción endotérmica: baja ligeramente) al mismo tiempo.
Esto permite ver ambos sensores respondiendo simultáneamente a la misma reacción.

### Procedimiento Parte B

1. Verter 200 ml de agua a temperatura ambiente en el vaso ancho.
2. Enjuagar el electrodo. Sumergir **ambos sensores** sin que se toquen entre sí.
3. Esperar 2 minutos. Anotar el punto inicial (5 lecturas + referencias).
4. Sin mover los sensores, agregar la tableta efervescente.
5. Cada **60 segundos** anotar 5 lecturas del Serial Monitor + valores de referencia.
6. Continuar hasta que las burbujas se detengan y los valores se estabilicen (~6 min).

---

### Punto B0 — Antes de la tableta (estado inicial)

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
| DS18B20 | Error absoluto en cada punto (A y B) | ≤ 0.5 °C |
| DS18B20 | Error absoluto promedio total | ≤ 0.5 °C |
| DS18B20 | Desviación estándar por punto | < 0.1 °C |
| Ambos | Registran cambio durante la reacción (Parte B) | Sí / No |

---

## Observaciones

_(Anotar si los sensores se afectaron mutuamente, si el cambio de temperatura
fue visible durante la reacción, cuál sensor respondió más rápido al cambio,
diferencias entre Parte A y Parte B, cualquier lectura atípica.)_
