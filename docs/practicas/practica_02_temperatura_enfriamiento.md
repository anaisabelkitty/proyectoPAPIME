# Práctica 02 — Validación del sensor de temperatura

**Fecha:** _______________
**Responsable:** _______________
**Sensor bajo prueba:** DS18B20 sumergible (kit Sensei)
**Sensor de referencia:** _______________ (modelo del termómetro digital comercial)

---

## Objetivo

Validar estadísticamente que el sensor DS18B20 mide correctamente la temperatura,
tanto en condiciones estáticas (puntos de temperatura conocida) como en condiciones
dinámicas (curva de enfriamiento en tiempo real).

---

## Fórmulas

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

- Kit Sensei con sensor DS18B20 conectado (pin digital 7, con resistencia 4.7 kΩ)
- Termómetro digital de referencia (modelo: _______________)
- Computadora con Serial Monitor abierto a 9600 baudios (escribir `2` en el menú para activar lectura de temperatura)
- Para la Parte A:
  - Vaso con agua y hielo (temperatura ~0–5°C)
  - Vaso con agua a temperatura ambiente (~20–25°C)
  - Vaso con agua tibia (~40–50°C, sin hervir)
- Para la Parte B:
  - 200 ml de agua caliente (~55–65°C, sin hervir)
  - Vaso de vidrio (mínimo 250 ml)
  - Cronómetro

> **Nota de seguridad:** no usar agua hirviendo (100°C).
> El DS18B20 soporta hasta 85°C en agua según su datasheet.

---

## Parte A — Puntos fijos (validación estática)

Medición de tres temperaturas conocidas y estables.
Para cada punto se toman **5 lecturas consecutivas** del Sensei
y se comparan contra el termómetro de referencia.

### Procedimiento Parte A

1. Preparar el vaso con la muestra correspondiente.
2. Sumergir el DS18B20 y el termómetro de referencia en la misma muestra.
3. Esperar **1 minuto** para que ambos se estabilicen.
4. Anotar 5 lecturas consecutivas del Serial Monitor.
5. Anotar el valor del termómetro de referencia.
6. Secar el sensor antes de pasar a la siguiente muestra.

---

### Punto A1 — Agua con hielo (temperatura esperada: 0–5°C)

**Temperatura medida por referencia:** _______________ °C

| Lectura | Temperatura DS18B20 (°C) |
|---------|--------------------------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ °C | **Desv. estándar:** _______________ °C | **Error vs referencia:** _______________ °C

---

### Punto A2 — Agua a temperatura ambiente (esperada: 20–25°C)

**Temperatura medida por referencia:** _______________ °C

| Lectura | Temperatura DS18B20 (°C) |
|---------|--------------------------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ °C | **Desv. estándar:** _______________ °C | **Error vs referencia:** _______________ °C

---

### Punto A3 — Agua tibia (esperada: 40–50°C)

**Temperatura medida por referencia:** _______________ °C

| Lectura | Temperatura DS18B20 (°C) |
|---------|--------------------------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ °C | **Desv. estándar:** _______________ °C | **Error vs referencia:** _______________ °C

---

### Tabla resumen Parte A

| Punto | Temp. esperada | Temp. referencia (°C) | Temp. DS18B20 promedio (°C) | Desv. estándar (°C) | Error absoluto (°C) |
|-------|---------------|----------------------|----------------------------|---------------------|---------------------|
| Agua con hielo | 0–5°C | | | | |
| Agua ambiente | 20–25°C | | | | |
| Agua tibia | 40–50°C | | | | |

**Error absoluto promedio Parte A:** _______________ °C

---

## Parte B — Curva de enfriamiento (validación dinámica)

Monitoreo continuo del descenso de temperatura del agua caliente al enfriarse,
comparando el DS18B20 contra el termómetro de referencia cada 3 minutos.

### Procedimiento Parte B

1. Calentar 200 ml de agua hasta ~60°C.
2. Verter en el vaso.
3. Sumergir el DS18B20 y el termómetro de referencia.
4. Esperar 1 minuto para estabilizar.
5. Cada **3 minutos** anotar 5 lecturas Sensei + valor referencia.
6. Continuar hasta completar 8 puntos o hasta llegar a temperatura ambiente.

---

### Punto B1 — Minuto 0

**Temperatura referencia:** _______________ °C

| Lectura | Temperatura DS18B20 (°C) |
|---------|--------------------------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ °C | **Desv. estándar:** _______________ °C | **Error vs referencia:** _______________ °C

---

### Punto B2 — Minuto 3

**Temperatura referencia:** _______________ °C

| Lectura | Temperatura DS18B20 (°C) |
|---------|--------------------------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ °C | **Desv. estándar:** _______________ °C | **Error vs referencia:** _______________ °C

---

### Punto B3 — Minuto 6

**Temperatura referencia:** _______________ °C

| Lectura | Temperatura DS18B20 (°C) |
|---------|--------------------------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ °C | **Desv. estándar:** _______________ °C | **Error vs referencia:** _______________ °C

---

### Punto B4 — Minuto 9

**Temperatura referencia:** _______________ °C

| Lectura | Temperatura DS18B20 (°C) |
|---------|--------------------------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ °C | **Desv. estándar:** _______________ °C | **Error vs referencia:** _______________ °C

---

### Punto B5 — Minuto 12

**Temperatura referencia:** _______________ °C

| Lectura | Temperatura DS18B20 (°C) |
|---------|--------------------------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ °C | **Desv. estándar:** _______________ °C | **Error vs referencia:** _______________ °C

---

### Punto B6 — Minuto 15

**Temperatura referencia:** _______________ °C

| Lectura | Temperatura DS18B20 (°C) |
|---------|--------------------------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ °C | **Desv. estándar:** _______________ °C | **Error vs referencia:** _______________ °C

---

### Punto B7 — Minuto 18

**Temperatura referencia:** _______________ °C

| Lectura | Temperatura DS18B20 (°C) |
|---------|--------------------------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ °C | **Desv. estándar:** _______________ °C | **Error vs referencia:** _______________ °C

---

### Punto B8 — Minuto 21

**Temperatura referencia:** _______________ °C

| Lectura | Temperatura DS18B20 (°C) |
|---------|--------------------------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ °C | **Desv. estándar:** _______________ °C | **Error vs referencia:** _______________ °C

---

### Tabla resumen Parte B

| Punto | Tiempo (min) | Temp. referencia (°C) | Temp. DS18B20 promedio (°C) | Desv. estándar (°C) | Error absoluto (°C) |
|-------|--------------|----------------------|----------------------------|---------------------|---------------------|
| B1 | 0 | | | | |
| B2 | 3 | | | | |
| B3 | 6 | | | | |
| B4 | 9 | | | | |
| B5 | 12 | | | | |
| B6 | 15 | | | | |
| B7 | 18 | | | | |
| B8 | 21 | | | | |

**Error absoluto promedio Parte B:** _______________ °C

---

## Criterios de evaluación estadística

| Criterio | Meta |
|----------|------|
| Error absoluto en cada punto (A y B) | ≤ 0.5 °C (precisión de fábrica DS18B20) |
| Error absoluto promedio total | ≤ 0.5 °C |
| Desviación estándar por punto | < 0.1 °C |
| Temperatura disminuye progresivamente en Parte B | Sí / No |
| Rango cubierto en Parte B | > 25 °C |

---

## Observaciones

_(Anotar si el sensor tardó en responder a cambios bruscos de temperatura,
si hubo diferencias más grandes en rangos extremos, comportamiento en agua con hielo,
cualquier lectura de -127°C que indique desconexión.)_
