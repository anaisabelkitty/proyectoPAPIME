# Prueba: sensor de humedad de suelo OKY3442 en el conector 4

Guía puntual para conectar el sensor de humedad al conector 4. Referencia completa del sistema: [`diseno_electronico.md`](diseno_electronico.md). Mismo patrón que los otros tres: [`prueba_temperatura_conector1.md`](prueba_temperatura_conector1.md), [`prueba_ph_conector2.md`](prueba_ph_conector2.md), [`prueba_ultrasonico_conector3.md`](prueba_ultrasonico_conector3.md).

**Convención de pines del RJ45 (fija):** de izquierda a derecha, pin 3 = ID3, pin 4 = ID2, pin 5 = ID1, pin 6 = ID0.

Código de identificación de la humedad: **4** (binario 0100: ID3=0, ID2=1, ID1=0, ID0=0).

---

## Lado del núcleo (RJ45 del conector 4 → Mega)

| Pin del RJ45 | Función | Pin del Mega |
|---|---|---|
| 1 | VCC | 5V |
| 2 | GND | GND |
| 3 | ID3 | 34 |
| 4 | ID2 | 35 |
| 5 | ID1 | 36 |
| 6 | ID0 | 37 |
| 7 | Señal 1 | A6 |
| 8 | Señal 2 | A7 (sin usar en esta prueba) |

Sin resistencias de este lado. Los pines 34-37 ya están configurados como `INPUT_PULLUP` en `main.cpp`.

---

## Lado del módulo (OKY3442 → RJ45 del módulo)

Como el código es 4 (ID3=0, ID2=1, ID1=0, ID0=0), solo el bit ID2 va en 1 (eléctricamente LOW = resistencia a GND):

| Pin del RJ45 | Función | Conexión en el módulo |
|---|---|---|
| 1 | VCC | VCC del módulo OKY3442 |
| 2 | GND | GND del módulo OKY3442 |
| 3 | ID3 | Sin conectar |
| 4 | ID2 | Resistencia de 1 kΩ hacia GND |
| 5 | ID1 | Sin conectar |
| 6 | ID0 | Sin conectar |
| 7 | Señal 1 | A0 (salida analógica) del módulo OKY3442 |
| 8 | Señal 2 | Sin conectar |

No necesita resistencia de pull-up aparte. Una sola resistencia de identificación en todo el módulo, en el pin 4 (ID2).

En la protoboard del módulo:
- Fila VCC: VCC del OKY3442 + pin 1 del RJ45.
- Fila GND: GND del OKY3442 + pin 2 del RJ45.
- Fila Señal 1: A0 del OKY3442 + pin 7 del RJ45.
- Fila ID2: pin 4 del RJ45 + una pata de la resistencia de 1 kΩ, cuya otra pata va a la fila GND.
- Pines 3, 5, 6 y 8 del RJ45: sin ningún cable.

**Recordatorio ya documentado en `sensores.md`:** la relación entre el ADC y el porcentaje de humedad depende del tipo de suelo, así que conviene calibrar con la opción 7 del menú (2 puntos: seco y húmedo) antes de confiar en el valor — eso no cambia por usar el sistema de identificación.

---

## Cómo probar

1. Armar el módulo según la tabla de arriba.
2. Conectar el cable Cat5 entre el RJ45 del núcleo (conector 4) y el RJ45 del módulo.
3. Subir el `main.cpp` actual al Mega si no lo has hecho.
4. Abrir el monitor serial, escribir `9`.
5. Debe aparecer: `Conector 4 (codigo 4): Humedad = XX.X %`.

Si ya tienes los otros tres sensores conectados (temperatura en 1, pH en 2, ultrasónico en 3), la opción 9 debe mostrar los 4 al mismo tiempo, cada uno en su fila — ese es el escenario completo de prueba.

Si el código no coincide con 4, revisar con multímetro (todo apagado): continuidad entre el pin 4 del RJ45 del módulo y GND (debe marcar ~1 kΩ), y que el cable Cat5 no tenga ningún pin cruzado.
