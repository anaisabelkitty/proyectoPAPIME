# Prueba: sensor de pH PH-4502C en el conector 2

Guía puntual para conectar el pH al conector 2. Referencia completa del sistema: [`diseno_electronico.md`](diseno_electronico.md). Sigue el mismo patrón que [`prueba_temperatura_conector1.md`](prueba_temperatura_conector1.md), aquí con el conector 2 para tener los dos sensores en conectores distintos al mismo tiempo.

**Convención de pines del RJ45 (fija):** de izquierda a derecha, pin 3 = ID3, pin 4 = ID2, pin 5 = ID1, pin 6 = ID0.

Código de identificación del pH: **2** (binario 0010: ID3=0, ID2=0, ID1=1, ID0=0).

---

## Lado del núcleo (RJ45 del conector 2 → Mega)

| Pin del RJ45 | Función | Pin del Mega |
|---|---|---|
| 1 | VCC | 5V |
| 2 | GND | GND |
| 3 | ID3 | 26 |
| 4 | ID2 | 27 |
| 5 | ID1 | 28 |
| 6 | ID0 | 29 |
| 7 | Señal 1 | A2 |
| 8 | Señal 2 | A3 (sin usar en esta prueba) |

Sin resistencias de este lado. Los pines 26-29 ya están configurados como `INPUT_PULLUP` en `main.cpp` (el `setup()` lo hace para los 4 conectores).

---

## Lado del módulo (PH-4502C → RJ45 del módulo)

Como el código es 2 (ID3=0, ID2=0, ID1=1, ID0=0), solo el bit ID1 va en 1 (eléctricamente LOW = resistencia a GND):

| Pin del RJ45 | Función | Conexión en el módulo |
|---|---|---|
| 1 | VCC | VCC del módulo PH-4502C |
| 2 | GND | GND del módulo PH-4502C |
| 3 | ID3 | Sin conectar |
| 4 | ID2 | Sin conectar |
| 5 | ID1 | Resistencia de 1 kΩ hacia GND |
| 6 | ID0 | Sin conectar |
| 7 | Señal 1 | Po (salida analógica) del módulo PH-4502C |
| 8 | Señal 2 | Sin conectar |

El pH no usa una resistencia de pull-up aparte (eso solo lo necesita el DS18B20 por el protocolo 1-Wire). Aquí la única resistencia de todo el módulo es la de identificación, en el pin 5 (ID1).

En la protoboard del módulo:
- Fila VCC: VCC del PH-4502C + pin 1 del RJ45.
- Fila GND: GND del PH-4502C + pin 2 del RJ45.
- Fila Señal 1: Po del PH-4502C + pin 7 del RJ45.
- Fila ID1: pin 5 del RJ45 + una pata de la resistencia de 1 kΩ, cuya otra pata va a la fila GND.
- Pines 3, 4, 6 y 8 del RJ45: sin ningún cable.

**Recordatorio del módulo pH ya documentado en `sensores.md`:** el electrodo E201C-BNC va en el conector BNC de la tarjeta, y antes de usarlo hay que calibrar el offset con POT2 (el potenciómetro más cercano al BNC) — eso no cambia por usar el sistema de identificación, sigue siendo necesario.

---

## Cómo probar

1. Armar el módulo según la tabla de arriba (con el electrodo ya calibrado, o al menos conectado).
2. Conectar el cable Cat5 entre el RJ45 del núcleo (conector 2) y el RJ45 del módulo.
3. Subir el `main.cpp` actual al Mega si no lo has hecho.
4. Abrir el monitor serial, escribir `9`.
5. Debe aparecer: `Conector 2 (codigo 2): pH = X.XX`.

Si ya tienes el DS18B20 conectado en el conector 1, la opción 9 debe mostrar los dos al mismo tiempo, cada uno en su fila.

Si el código no coincide con 2, revisar con multímetro (todo apagado): continuidad entre el pin 5 del RJ45 del módulo y GND (debe marcar ~1 kΩ), y que el cable Cat5 no tenga ningún pin cruzado.
