# Prueba: sensor ultrasónico HC-SR04 en el conector 3

Guía puntual para conectar el HC-SR04 al conector 3. Referencia completa del sistema: [`diseno_electronico.md`](diseno_electronico.md). Mismo patrón que [`prueba_temperatura_conector1.md`](prueba_temperatura_conector1.md) y [`prueba_ph_conector2.md`](prueba_ph_conector2.md).

**Convención de pines del RJ45 (fija):** de izquierda a derecha, pin 3 = ID3, pin 4 = ID2, pin 5 = ID1, pin 6 = ID0.

Código de identificación del HC-SR04: **10** (binario 1010: ID3=1, ID2=0, ID1=1, ID0=0).

Este es el único de los 4 sensores ya programados que usa **las dos líneas de señal** (Trig y Echo), no solo una.

---

## Lado del núcleo (RJ45 del conector 3 → Mega)

| Pin del RJ45 | Función | Pin del Mega |
|---|---|---|
| 1 | VCC | 5V |
| 2 | GND | GND |
| 3 | ID3 | 30 |
| 4 | ID2 | 31 |
| 5 | ID1 | 32 |
| 6 | ID0 | 33 |
| 7 | Señal 1 | A4 |
| 8 | Señal 2 | A5 |

Sin resistencias de este lado. Los pines 30-33 ya están configurados como `INPUT_PULLUP` en `main.cpp`.

---

## Lado del módulo (HC-SR04 → RJ45 del módulo)

Como el código es 10 (ID3=1, ID2=0, ID1=1, ID0=0), van dos bits en 1 (eléctricamente LOW = resistencia a GND):

| Pin del RJ45 | Función | Conexión en el módulo |
|---|---|---|
| 1 | VCC | Vcc del HC-SR04 |
| 2 | GND | GND del HC-SR04 |
| 3 | ID3 | Resistencia de 1 kΩ hacia GND |
| 4 | ID2 | Sin conectar |
| 5 | ID1 | Resistencia de 1 kΩ hacia GND |
| 6 | ID0 | Sin conectar |
| 7 | Señal 1 | Trig del HC-SR04 |
| 8 | Señal 2 | Echo del HC-SR04 |

En la protoboard del módulo:
- Fila VCC: Vcc del HC-SR04 + pin 1 del RJ45.
- Fila GND: GND del HC-SR04 + pin 2 del RJ45.
- Fila ID3: pin 3 del RJ45 + una pata de una resistencia de 1 kΩ, cuya otra pata va a la fila GND.
- Fila ID1: pin 5 del RJ45 + una pata de otra resistencia de 1 kΩ, cuya otra pata va a la fila GND.
- Fila Señal 1: Trig del HC-SR04 + pin 7 del RJ45.
- Fila Señal 2: Echo del HC-SR04 + pin 8 del RJ45.
- Pines 4 y 6 del RJ45: sin ningún cable.

No necesita resistencia de pull-up aparte (esa solo la usa el DS18B20). Total en este módulo: 2 resistencias de 1 kΩ, nada más.

---

## Cómo probar

1. Armar el módulo según la tabla de arriba.
2. Conectar el cable Cat5 entre el RJ45 del núcleo (conector 3) y el RJ45 del módulo.
3. Subir el `main.cpp` actual al Mega si no lo has hecho.
4. Abrir el monitor serial, escribir `9` (la opción del menú, no confundir con el código del sensor).
5. Debe aparecer: `Conector 3 (codigo 10): Distancia = XX.X cm`.

Si ya tienes el DS18B20 en el conector 1 y el pH en el conector 2, la opción 9 del menú debe mostrar los tres al mismo tiempo, cada uno en su fila.

Si el código no coincide con 10, revisar con multímetro (todo apagado): continuidad entre el pin 3 y GND (~1 kΩ), y entre el pin 5 y GND (~1 kΩ), ambas del RJ45 del módulo. Si el código sí es 10 pero la distancia da error o "sin eco", revisar que Trig y Echo no estén invertidos entre sí (pin 7 = Trig, pin 8 = Echo).
