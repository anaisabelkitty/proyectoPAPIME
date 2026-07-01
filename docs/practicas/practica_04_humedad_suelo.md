# Práctica 04 — Validación del sensor de humedad de suelo

**Fecha:** _______________
**Responsable:** _______________
**Sensor bajo prueba:** OKY3442 (kit Sensei)

---

## Objetivo

Validar que el sensor de humedad de suelo OKY3442 responde correctamente
a diferentes niveles de humedad, verificando que los valores reportados
son coherentes con las condiciones reales del suelo.

---

## Fórmulas

**Promedio (x̄):**

> x̄ = (x₁ + x₂ + x₃ + x₄ + x₅) / n

Donde n = 5 (número de lecturas por punto).

**Desviación estándar (σ):**

> σ = √[ Σ(xᵢ − x̄)² / (n − 1) ]

Mide qué tanto se dispersan las lecturas respecto al promedio.
Un valor bajo indica que el sensor es estable y repetible.

---

## Materiales

- Kit Sensei con sensor OKY3442 conectado (pin A4)
- Computadora con Serial Monitor abierto a 9600 baudios (escribir `5` en el menú para activar lectura de humedad)
- Para la Parte A:
  - Sonda sin tocar nada (en el aire)
  - Un vaso con agua
  - Tierra seca (puede ser de una maceta sin regar)
  - Tierra húmeda (la misma maceta recién regada, o tierra con agua agregada)
- Para la Parte B:
  - Una maceta con tierra seca
  - Un vaso con agua (para ir agregando poco a poco)

---

## Parte A — Condiciones extremas (validación estática)

Medición en cuatro condiciones conocidas de humedad.
Para cada condición se toman **5 lecturas consecutivas** del Sensei.

### Procedimiento Parte A

1. Conectar el sensor y seleccionar modo 5 en el Serial Monitor.
2. Para cada condición: insertar la sonda, esperar 10 segundos y anotar 5 lecturas.
3. Limpiar/secar la sonda antes de la siguiente condición.

---

### Condición A1 — Sonda en el aire (esperado: 0–10%)

| Lectura | Humedad (%) | ADC |
|---------|-------------|-----|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio:** _______________%  | **Desv. estándar:** _______________%

---

### Condición A2 — Sonda en agua (esperado: 80–100%)

| Lectura | Humedad (%) | ADC |
|---------|-------------|-----|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio:** _______________%  | **Desv. estándar:** _______________%

---

### Condición A3 — Sonda en tierra seca (esperado: 10–40%)

| Lectura | Humedad (%) | ADC |
|---------|-------------|-----|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio:** _______________%  | **Desv. estándar:** _______________%

---

### Condición A4 — Sonda en tierra húmeda (esperado: 50–90%)

| Lectura | Humedad (%) | ADC |
|---------|-------------|-----|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio:** _______________%  | **Desv. estándar:** _______________%

---

### Tabla resumen Parte A

| Condición | Humedad esperada | Humedad Sensei promedio (%) | Desv. estándar (%) |
|-----------|-----------------|----------------------------|---------------------|
| Aire | 0–10% | | |
| Agua | 80–100% | | |
| Tierra seca | 10–40% | | |
| Tierra húmeda | 50–90% | | |

---

## Parte B — Proceso de riego (validación dinámica)

Monitoreo continuo de la humedad mientras se agrega agua gradualmente
a una maceta con tierra seca.

### Procedimiento Parte B

1. Insertar la sonda en la tierra seca de una maceta.
2. Esperar 30 segundos para estabilizar. Anotar el valor inicial.
3. Agregar **50 ml de agua** a la maceta (no directamente sobre la sonda).
4. Esperar **60 segundos** para que el agua se distribuya.
5. Anotar 5 lecturas.
6. Repetir los pasos 3, 4 y 5 hasta completar 5 adiciones de agua (250 ml total).

---

### Punto B0 — Tierra seca (antes de regar)

| Lectura | Humedad (%) | ADC |
|---------|-------------|-----|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio:** _______________%

---

### Punto B1 — Después de 50 ml de agua

| Lectura | Humedad (%) | ADC |
|---------|-------------|-----|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio:** _______________%

---

### Punto B2 — Después de 100 ml de agua

| Lectura | Humedad (%) | ADC |
|---------|-------------|-----|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio:** _______________%

---

### Punto B3 — Después de 150 ml de agua

| Lectura | Humedad (%) | ADC |
|---------|-------------|-----|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio:** _______________%

---

### Punto B4 — Después de 200 ml de agua

| Lectura | Humedad (%) | ADC |
|---------|-------------|-----|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio:** _______________%

---

### Punto B5 — Después de 250 ml de agua

| Lectura | Humedad (%) | ADC |
|---------|-------------|-----|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |

**Promedio:** _______________%

---

### Tabla resumen Parte B

| Punto | Agua total (ml) | Humedad promedio (%) |
|-------|-----------------|----------------------|
| B0 | 0 | |
| B1 | 50 | |
| B2 | 100 | |
| B3 | 150 | |
| B4 | 200 | |
| B5 | 250 | |

---

## Criterios de evaluación

| Criterio | Meta |
|----------|------|
| Aire reporta menos humedad que agua | Sí / No |
| Tierra seca reporta menos que tierra húmeda | Sí / No |
| Humedad sube progresivamente con cada riego (Parte B) | Sí / No |
| Desviación estándar por punto (Parte A) | < 5% |
| Rango cubierto entre seco y mojado | > 50 puntos porcentuales |

---

## Observaciones

_(Anotar si el sensor tardó en responder al cambio de humedad, si la
posición de la sonda afecta la lectura, si hay diferencia entre insertar
solo la punta o toda la sonda, comportamiento al sacar y meter la sonda.)_
