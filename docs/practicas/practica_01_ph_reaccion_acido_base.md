# Práctica 01 — Validación del sensor de pH

**Fecha:** _______________
**Responsable:** _______________
**Sensor bajo prueba:** PH-4502C (kit Sensei)
**Sensor de referencia:** _______________ (modelo del pHmetro comercial)

---

## Objetivo

Validar estadísticamente que el sensor PH-4502C mide correctamente el pH,
tanto en condiciones estáticas (sustancias con pH conocido) como en condiciones
dinámicas (reacción ácido-base en tiempo real).

---

## Materiales

- Kit Sensei con sensor PH-4502C conectado (pin A2)
- pHmetro digital de referencia (modelo: _______________)
- Computadora con Serial Monitor abierto a 9600 baudios, **modo 1 activo**
- Agua destilada para enjuagar el electrodo entre muestras
- Servilletas o papel absorbente
- Para la Parte A:
  - Agua destilada
  - Vinagre blanco de cocina
  - Solución de agua con bicarbonato (1 cucharadita en 250 ml de agua)
- Para la Parte B:
  - 150 ml de vinagre blanco
  - Bicarbonato de sodio (3 cucharaditas)
  - Cuchara pequeña
  - Vaso de vidrio (mínimo 250 ml)

---

## Parte A — Sustancias fijas (validación estática)

Medición de tres sustancias con pH conocido y estable.
Para cada sustancia se toman **5 lecturas consecutivas** del Sensei
y se comparan contra el pHmetro de referencia.

### Procedimiento Parte A

1. Verter la sustancia en un vaso limpio.
2. Medir el pH con el pHmetro de referencia y anotar.
3. Enjuagar el electrodo del PH-4502C con agua destilada y secar.
4. Sumergir el electrodo y esperar **2 minutos** para estabilizar.
5. Anotar 5 lecturas consecutivas del Serial Monitor.
6. Enjuagar el electrodo antes de pasar a la siguiente sustancia.

---

### Sustancia A1 — Agua destilada (pH esperado: ~7.0)

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

### Sustancia A2 — Vinagre blanco (pH esperado: 2.0 – 3.5)

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

### Sustancia A3 — Agua con bicarbonato (pH esperado: 8.0 – 9.0)

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

| Sustancia | pH esperado | pH referencia | pH Sensei promedio | Desv. estándar | Error absoluto |
|-----------|------------|---------------|--------------------|----------------|----------------|
| Agua destilada | ~7.0 | | | | |
| Vinagre | 2.0–3.5 | | | | |
| Agua + bicarbonato | 8.0–9.0 | | | | |

**Error absoluto promedio Parte A:** _______________

---

## Parte B — Reacción ácido-base (validación dinámica)

Reacción de neutralización: vinagre (ácido) + bicarbonato de sodio (base).
El pH sube progresivamente de ~2.5 a ~8.0 a medida que se agrega bicarbonato.

**Fundamento:**
> CH₃COOH + NaHCO₃ → CH₃COONa + H₂O + CO₂

### Procedimiento Parte B

1. Verter 150 ml de vinagre en un vaso limpio.
2. Enjuagar el electrodo del PH-4502C y sumergirlo en el vinagre.
3. Esperar 2 minutos para estabilizar. Anotar el pH inicial (5 lecturas).
4. Medir simultáneamente con el pHmetro de referencia.
5. Agregar **½ cucharadita** de bicarbonato, mezclar suavemente y esperar
   60 segundos hasta que las burbujas se calmen.
6. Repetir: anotar 5 lecturas Sensei + valor referencia.
7. Repetir pasos 5 y 6 hasta completar 6 adiciones.

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

### Punto B1 — ½ cucharadita de bicarbonato

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

### Punto B2 — 1 cucharadita

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

### Punto B3 — 1½ cucharaditas

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

### Punto B4 — 2 cucharaditas

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

### Punto B5 — 2½ cucharaditas

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

### Punto B6 — 3 cucharaditas

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
| B0 | 0 | | | | |
| B1 | ½ cdita | | | | |
| B2 | 1 cdita | | | | |
| B3 | 1½ cditas | | | | |
| B4 | 2 cditas | | | | |
| B5 | 2½ cditas | | | | |
| B6 | 3 cditas | | | | |

**Error absoluto promedio Parte B:** _______________

---

## Criterios de evaluación estadística

| Criterio | Meta |
|----------|------|
| Error absoluto en cada punto (A y B) | < 0.3 pH |
| Error absoluto promedio total | < 0.3 pH |
| Desviación estándar por punto | < 0.15 pH |
| pH sube progresivamente en Parte B | Sí / No |
| Rango cubierto en Parte B (pH final − pH inicial) | > 4 unidades |

---

## Observaciones

_(Anotar si el sensor tardó en estabilizarse, si hubo lecturas atípicas,
comportamiento durante la reacción, diferencias entre Parte A y Parte B.)_
