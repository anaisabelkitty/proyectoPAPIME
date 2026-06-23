# Práctica 01 — Verificación del sensor de pH con líquidos cotidianos

**Fecha:** _______________
**Responsable:** _______________
**Sensor bajo prueba:** PH-4502C (kit Sensei)
**Sensor de referencia:** _______________ (modelo del pHmetro de fábrica)

---

## Objetivo

Verificar que el sensor PH-4502C mide correctamente el pH de líquidos cotidianos,
comparando sus lecturas contra un pHmetro de referencia calibrado de fábrica.

---

## Materiales

- Kit Sensei con sensor PH-4502C (calibrado con buffers pH 4.01 y pH 7.00)
- pHmetro de referencia (modelo: _______________)
- Computadora con Serial Monitor abierto a 9600 baudios
- Agua destilada para enjuagar entre muestras
- Servilletas o papel absorbente
- Vasos o recipientes pequeños (uno por muestra)
- Las siguientes muestras:

| # | Muestra | pH de referencia esperado (aprox.) |
|---|---------|-----------------------------------|
| 1 | Jugo de limón (exprimido) | 2.0 – 3.5 |
| 2 | Jugo de naranja (exprimido) | 3.5 – 4.5 |
| 3 | Refresco de cola | 2.5 – 3.5 |
| 4 | Leche entera | 6.5 – 6.8 |
| 5 | Agua de la llave | 6.5 – 8.5 |
| 6 | Agua con bicarbonato (1 cdita en 250 ml) | 8.0 – 9.0 |

---

## Procedimiento

Para **cada muestra** seguir estos pasos en orden:

1. Verter la muestra en un vaso limpio.
2. Medir el pH con el **pHmetro de referencia** y anotar el valor en la tabla.
3. Enjuagar el electrodo del PH-4502C con agua destilada y secar con servilleta.
4. Sumergir el electrodo en la muestra y esperar **2 minutos** sin mover nada.
5. Anotar **10 lecturas consecutivas** del Serial Monitor (una por segundo).
6. Enjuagar el electrodo antes de pasar a la siguiente muestra.

> **Importante:** medir primero con el pHmetro de referencia, antes de meter el
> electrodo del PH-4502C, para no contaminar la muestra.

---

## Tabla de resultados

### Muestra 1 — Jugo de limón

**pH medido por pHmetro de referencia:** _______________

| Lectura | pH (PH-4502C) |
|---------|---------------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |
| 6 | |
| 7 | |
| 8 | |
| 9 | |
| 10 | |

**Promedio:** _______________
**Desviación estándar:** _______________
**Error respecto a referencia:** _______________

---

### Muestra 2 — Jugo de naranja

**pH medido por pHmetro de referencia:** _______________

| Lectura | pH (PH-4502C) |
|---------|---------------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |
| 6 | |
| 7 | |
| 8 | |
| 9 | |
| 10 | |

**Promedio:** _______________
**Desviación estándar:** _______________
**Error respecto a referencia:** _______________

---

### Muestra 3 — Refresco de cola

**pH medido por pHmetro de referencia:** _______________

| Lectura | pH (PH-4502C) |
|---------|---------------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |
| 6 | |
| 7 | |
| 8 | |
| 9 | |
| 10 | |

**Promedio:** _______________
**Desviación estándar:** _______________
**Error respecto a referencia:** _______________

---

### Muestra 4 — Leche entera

**pH medido por pHmetro de referencia:** _______________

| Lectura | pH (PH-4502C) |
|---------|---------------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |
| 6 | |
| 7 | |
| 8 | |
| 9 | |
| 10 | |

**Promedio:** _______________
**Desviación estándar:** _______________
**Error respecto a referencia:** _______________

---

### Muestra 5 — Agua de la llave

**pH medido por pHmetro de referencia:** _______________

| Lectura | pH (PH-4502C) |
|---------|---------------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |
| 6 | |
| 7 | |
| 8 | |
| 9 | |
| 10 | |

**Promedio:** _______________
**Desviación estándar:** _______________
**Error respecto a referencia:** _______________

---

### Muestra 6 — Agua con bicarbonato

**pH medido por pHmetro de referencia:** _______________

| Lectura | pH (PH-4502C) |
|---------|---------------|
| 1 | |
| 2 | |
| 3 | |
| 4 | |
| 5 | |
| 6 | |
| 7 | |
| 8 | |
| 9 | |
| 10 | |

**Promedio:** _______________
**Desviación estándar:** _______________
**Error respecto a referencia:** _______________

---

## Tabla resumen

| Muestra | pH referencia | pH Sensei (promedio) | Desv. estándar | Error absoluto |
|---------|--------------|----------------------|----------------|----------------|
| Jugo de limón | | | | |
| Jugo de naranja | | | | |
| Refresco de cola | | | | |
| Leche entera | | | | |
| Agua de llave | | | | |
| Agua con bicarbonato | | | | |

---

## Criterios de evaluación

| Criterio | Meta |
|----------|------|
| Error absoluto promedio | < 0.3 pH |
| Desviación estándar por muestra | < 0.15 pH |
| Orden correcto de muestras (de más ácido a más alcalino) | Sí / No |

El criterio más importante es el orden: el sensor debe ordenar correctamente
las muestras aunque los valores exactos tengan pequeñas diferencias respecto
al pHmetro de referencia.

---

## Observaciones

_(Anotar aquí cualquier cosa rara que haya pasado durante la práctica: picos,
tiempo de estabilización mayor al esperado, problemas de conexión, etc.)_

