# Práctica 01 — Validación del sensor de pH

**Fecha:** _______________
**Responsable:** _______________
**Sensor bajo prueba:** PH-4502C (kit Sensei)
**Sensor de referencia:** _______________ (modelo del pHmetro comercial)

---

## Objetivo

Validar estadísticamente que el sensor PH-4502C mide correctamente el pH,
tanto en condiciones estáticas (líquidos cotidianos con pH conocido) como en
condiciones dinámicas (reacción ácido-base en tiempo real).

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

- Kit Sensei con sensor PH-4502C conectado (pin A2)
- pHmetro digital de referencia (modelo: _______________)
- Computadora con Serial Monitor abierto a 9600 baudios (escribir `1` en el menú para activar lectura de pH)
- Agua destilada para enjuagar el electrodo entre muestras
- Servilletas o papel absorbente
- Vasos pequeños (uno por muestra)
- Para la Parte A:
  - Jugo de limón exprimido
  - Jugo de naranja exprimido
  - Refresco de cola
  - Leche entera
  - Agua de la llave
  - Agua con bicarbonato (3 g en 250 ml de agua)
- Para la Parte B:
  - 150 ml de vinagre blanco de cocina
  - Bicarbonato de sodio (15 g)
  - Cuchara pequeña
  - Vaso de vidrio (mínimo 250 ml)

---

## Parte A — Muestras cotidianas (validación estática)

Medición de seis líquidos cotidianos con pH conocido y estable.
Para cada muestra se toman **5 lecturas consecutivas** del Sensei
y se comparan contra el pHmetro de referencia.

### Procedimiento Parte A

1. Verter la muestra en un vaso limpio.
2. Medir el pH con el pHmetro de referencia y anotar.
3. Enjuagar el electrodo del PH-4502C con agua destilada y secar.
4. Sumergir el electrodo y esperar **2 minutos** para estabilizar.
5. Anotar 5 lecturas consecutivas del Serial Monitor.
6. Enjuagar el electrodo antes de pasar a la siguiente muestra.

> **Importante:** medir primero con el pHmetro de referencia antes de
> meter el electrodo del PH-4502C, para no contaminar la muestra.

---

### Muestra 1 — Jugo de limón (pH esperado: 2.0–3.5)

**pH medido por referencia:** _______________

| Lectura | pH Sensei |
|---------|-----------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ | **Desv. estándar:** _______________ | **Error vs referencia:** _______________

---

### Muestra 2 — Jugo de naranja (pH esperado: 3.5–4.5)

**pH medido por referencia:** _______________

| Lectura | pH Sensei |
|---------|-----------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ | **Desv. estándar:** _______________ | **Error vs referencia:** _______________

---

### Muestra 3 — Refresco de cola (pH esperado: 2.5–3.5)

**pH medido por referencia:** _______________

| Lectura | pH Sensei |
|---------|-----------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ | **Desv. estándar:** _______________ | **Error vs referencia:** _______________

---

### Muestra 4 — Leche entera (pH esperado: 6.5–6.8)

**pH medido por referencia:** _______________

| Lectura | pH Sensei |
|---------|-----------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ | **Desv. estándar:** _______________ | **Error vs referencia:** _______________

---

### Muestra 5 — Agua de la llave (pH esperado: 6.5–8.5)

**pH medido por referencia:** _______________

| Lectura | pH Sensei |
|---------|-----------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ | **Desv. estándar:** _______________ | **Error vs referencia:** _______________

---

### Muestra 6 — Agua con bicarbonato (pH esperado: 8.0–9.0)

**pH medido por referencia:** _______________

| Lectura | pH Sensei |
|---------|-----------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ | **Desv. estándar:** _______________ | **Error vs referencia:** _______________

---

### Tabla resumen Parte A

| # | Muestra | pH esperado | pH referencia | pH Sensei promedio | Desv. estándar | Error absoluto |
|---|---------|------------|---------------|--------------------|----------------|----------------|
| 1 | Jugo de limón | 2.0–3.5 | | | | |
| 2 | Jugo de naranja | 3.5–4.5 | | | | |
| 3 | Refresco de cola | 2.5–3.5 | | | | |
| 4 | Leche entera | 6.5–6.8 | | | | |
| 5 | Agua de llave | 6.5–8.5 | | | | |
| 6 | Agua + bicarbonato | 8.0–9.0 | | | | |

**Error absoluto promedio Parte A:** _______________

---

## Parte B — Reacción ácido-base (validación dinámica)

Reacción de neutralización: vinagre (ácido) + bicarbonato de sodio (base).
El pH sube progresivamente de ~2.5 a ~8.0 con cada adición de bicarbonato.

**Fundamento:**
> CH₃COOH + NaHCO₃ → CH₃COONa + H₂O + CO₂

### Procedimiento Parte B

1. Verter 150 ml de vinagre en un vaso limpio.
2. Enjuagar el electrodo y sumergirlo en el vinagre.
3. Esperar 2 minutos. Anotar 5 lecturas + valor del pHmetro de referencia.
4. Agregar **2.5 g** de bicarbonato de sodio, mezclar suavemente y esperar
   60 segundos hasta que las burbujas se calmen.
5. Anotar 5 lecturas + valor referencia.
6. Repetir pasos 4 y 5 hasta completar 6 adiciones.

> **Importante:** agregar el bicarbonato poco a poco para no salpicarse.
> No meter el módulo electrónico en el líquido, solo el electrodo.

---

### Punto B0 — Solo vinagre

**pH referencia:** _______________

| Lectura | pH Sensei |
|---------|-----------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ | **Desv. estándar:** _______________ | **Error vs referencia:** _______________

---

### Punto B1 — 2.5 g de bicarbonato

**pH referencia:** _______________

| Lectura | pH Sensei |
|---------|-----------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ | **Desv. estándar:** _______________ | **Error vs referencia:** _______________

---

### Punto B2 — 5 g de bicarbonato

**pH referencia:** _______________

| Lectura | pH Sensei |
|---------|-----------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ | **Desv. estándar:** _______________ | **Error vs referencia:** _______________

---

### Punto B3 — 7.5 g de bicarbonato

**pH referencia:** _______________

| Lectura | pH Sensei |
|---------|-----------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ | **Desv. estándar:** _______________ | **Error vs referencia:** _______________

---

### Punto B4 — 10 g de bicarbonato

**pH referencia:** _______________

| Lectura | pH Sensei |
|---------|-----------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ | **Desv. estándar:** _______________ | **Error vs referencia:** _______________

---

### Punto B5 — 12.5 g de bicarbonato

**pH referencia:** _______________

| Lectura | pH Sensei |
|---------|-----------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ | **Desv. estándar:** _______________ | **Error vs referencia:** _______________

---

### Punto B6 — 15 g de bicarbonato

**pH referencia:** _______________

| Lectura | pH Sensei |
|---------|-----------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |

**Promedio:** _______________ | **Desv. estándar:** _______________ | **Error vs referencia:** _______________

---

### Tabla resumen Parte B

| Punto | Bicarbonato | pH referencia | pH Sensei promedio | Desv. estándar | Error absoluto |
|-------|-------------|---------------|--------------------|----------------|----------------|
| B0 | 0 g | | | | |
| B1 | 2.5 g | | | | |
| B2 | 5 g | | | | |
| B3 | 7.5 g | | | | |
| B4 | 10 g | | | | |
| B5 | 12.5 g | | | | |
| B6 | 15 g | | | | |

**Error absoluto promedio Parte B:** _______________

---

## Criterios de evaluación estadística

| Criterio | Meta |
|----------|------|
| Error absoluto en cada punto (A y B) | < 0.3 pH |
| Error absoluto promedio total | < 0.3 pH |
| Desviación estándar por punto | < 0.15 pH |
| Orden correcto de muestras Parte A (de más ácido a más alcalino) | Sí / No |
| pH sube progresivamente en Parte B | Sí / No |
| Rango cubierto en Parte B (pH final − pH inicial) | > 4 unidades |

---

## Observaciones

_(Anotar si el sensor tardó en estabilizarse entre muestras, si hubo lecturas
atípicas en alguna muestra cotidiana, comportamiento durante la reacción,
diferencias entre Parte A y Parte B.)_
