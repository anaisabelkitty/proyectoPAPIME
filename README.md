# Sensei — Kit de sensores PAPIME PE309523

Sensei es un kit de sensores desarrollado como parte del proyecto PAPIME PE309523 del Instituto de Ciencias Aplicadas y Tecnología (ICAT) de la UNAM. Se usará en las Escuelas Nacionales Preparatorias 3, 7 y 9.

El kit mide variables físicas con múltiples sensores y los datos quedan disponibles en tiempo real en un display LCD integrado, en una página web y en una app Android.

---

## ¿Cómo funciona?

El kit tiene 4 conectores RJ45 hembra donde se enchufan los sensores mediante un cable UTP. Se pueden conectar hasta 4 sensores al mismo tiempo en cualquier combinación. El sistema detecta automáticamente qué sensor hay en cada conector.

Un **Arduino Mega 2560** lee los sensores y muestra los datos en el LCD. Luego los envía por UART a una **ESP32 NodeMCU**, que levanta un servidor web HTTP en la red local. Los datos se consultan desde una página web (pruebas) o desde la app Android (producción).

```
Sensores → Arduino Mega → ESP32 → Servidor HTTP → Página web / App Android
```

---

## Hardware

| Componente | Función |
|---|---|
| Arduino Mega 2560 | Lee sensores, muestra LCD, envía datos a ESP32 |
| ESP32 NodeMCU | WiFi, BLE, servidor HTTP |
| Display LCD 20×4 con I2C | Muestra datos en tiempo real |
| 4 conectores RJ45 hembra | Puntos de conexión para los sensores |
| Batería Li-Ion 9V 3600 mWh | Uso portátil, recarga por USB-C |

---

## Sensores

### Analógicos
| # | Sensor | Pin |
|---|--------|-----|
| 1 | Sensor de Voltaje 0–25 V DC (AR2657) | A0 |
| 2 | Sensor de Corriente ACS712 (30 A) | A1 |
| 3 | Sensor de pH PH-4502C | A2 |
| 4 | Fototransistor PT331C | A3 |
| 5 | Sensor de Humedad de Suelo (OKY3442) | A4 |
| 6 | Sensor de CO2 MG811 (OKY3351) | A5 |
| 7 | Sensor de Pulso Cardíaco (OKY3471-5) | A6 |

### Digitales
| # | Sensor | Pin |
|---|--------|-----|
| 8 | Sensor de Efecto Hall SM351LT | Digital 22 |
| 9 | Sensor de Temperatura DS18B20 (sumergible) | Digital 7 |

---

## Estructura del repositorio

```
PAPIME/
├── platformio.ini          ← configuración PlatformIO (Mega)
├── src/                    ← código del Arduino Mega
│   ├── main.cpp             ← menú principal con todos los sensores
│   ├── sensores_analogicos/
│   │   ├── ph/               ✅ implementado
│   │   └── humedad_suelo/    ✅ implementado
│   └── sensores_digitales/
│       └── temperatura_ds18b20/  ✅ implementado
├── esp32/                  ← código de la ESP32 (pendiente)
├── docs/                   ← documentación técnica y prácticas
│   ├── sensores.md          ← documentación técnica de cada sensor
│   ├── practicas/           ← prácticas de validación estadística
│   └── android.md           ← coordinación con el desarrollador Android
└── extras/
    └── ph/                  ← sketches de calibración del sensor pH
```

---

## Entorno de desarrollo

- [PlatformIO](https://platformio.org/) en Visual Studio Code
- Lenguaje: C++ con framework Arduino
- Tarjeta: Arduino Mega 2560 (`megaatmega2560`)

---

## Estado del proyecto

Implementados: sensor de pH (PH-4502C, con calibración persistente en EEPROM), sensor de temperatura (DS18B20) y sensor de humedad de suelo (OKY3442). Los tres cuentan con prácticas de validación estadística en `docs/practicas/`. Los demás sensores, la ESP32, el servidor web y la página web están en desarrollo.
