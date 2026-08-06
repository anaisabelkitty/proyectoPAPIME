# Prueba: sensor de temperatura DS18B20 en el conector 1

Guía puntual para conectar el DS18B20 al conector 1 y probar la identificación con los pines 22, 23, 24 y 25. Referencia completa del sistema: [`diseno_electronico.md`](diseno_electronico.md).

Código de identificación del DS18B20: **8** (ID0=0, ID1=0, ID2=0, ID3=1).

---

## Lado del núcleo (RJ45 del conector 1 → Mega)

| Pin del RJ45 | Función | Pin del Mega |
|---|---|---|
| 1 | VCC | 5V |
| 2 | GND | GND |
| 3 | ID0 | 22 |
| 4 | ID1 | 23 |
| 5 | ID2 | 24 |
| 6 | ID3 | 25 |
| 7 | Señal 1 | A0 |
| 8 | Señal 2 | A1 (sin usar en esta prueba) |

Sin resistencias de este lado. Los 4 pines de ID (22-25) se configuran como `INPUT_PULLUP` en el código (ya está hecho en `main.cpp`).

---

## Lado del módulo (DS18B20 → RJ45 del módulo)

Como el código es 8 (ID0=0, ID1=0, ID2=0, ID3=1), solo el bit ID3 va en 0. Eso significa una sola resistencia de identificación:

| Pin del RJ45 | Función | Conexión en el módulo |
|---|---|---|
| 1 | VCC | Cable rojo del DS18B20 (VCC) |
| 2 | GND | Cable negro del DS18B20 (GND) |
| 3 | ID0 | Sin conectar |
| 4 | ID1 | Sin conectar |
| 5 | ID2 | Sin conectar |
| 6 | ID3 | Resistencia de 1 kΩ hacia GND |
| 7 | Señal 1 | Cable de datos (DQ) del DS18B20 |
| 8 | Señal 2 | Sin conectar |

**Además, el DS18B20 necesita su resistencia de pull-up de 4.7 kΩ**, que no tiene nada que ver con la identificación — va entre el pin 7 (Señal 1 / DQ) y el pin 1 (VCC) del mismo RJ45 del módulo, para que el protocolo 1-Wire funcione.

```
Módulo (protoboard chica):

  RJ45 pin 1 (VCC) ──┬──────────────► Cable rojo DS18B20
                      │
                    [4.7kΩ]  (pull-up 1-Wire)
                      │
  RJ45 pin 7 (Señal 1) ┴──────────────► Cable amarillo/blanco DS18B20 (DQ)

  RJ45 pin 2 (GND) ──────────────────► Cable negro DS18B20

  RJ45 pin 6 (ID3) ──[1kΩ]── GND      (fija el bit ID3 = 0)

  RJ45 pines 3, 4, 5, 8: sin conectar
```

---

## Cómo probar

1. Armar el módulo según la tabla de arriba.
2. Conectar el cable Cat5 entre el RJ45 del núcleo (conector 1) y el RJ45 del módulo.
3. Subir el `main.cpp` actual al Mega (ya tiene la opción 9 del menú lista).
4. Abrir el monitor serial, escribir `9`.
5. Debe aparecer: `Conector 1 (codigo 8): Temperatura = XX.XX C`.

Si el código no coincide con 8, revisar que la resistencia de 1 kΩ esté realmente conectada al pin ID3 (pin 6 del RJ45) y no a otro. Si el código sí es 8 pero la temperatura da error, revisar la resistencia de pull-up de 4.7 kΩ (esa es la que afecta la lectura del sensor, no la identificación).
