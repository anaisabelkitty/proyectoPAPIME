# Investigación Final (9)

# Conceptos base

## El convertidor analógico-digital (ADC) del Arduino Mega 2560

El Arduino Mega 2560 utiliza el microcontrolador ATmega2560, que tiene integrado un convertidor analógico-digital (ADC). La tarjeta dispone de 16 pines de entrada analógica, etiquetados de A0 a A15.

El ADC convierte un voltaje analógico (continuo) en un número digital (discreto) que el microcontrolador puede procesar. La resolución de este ADC es de **10 bits**.

**¿De dónde salen los 1,024 valores?**

Un bit es la unidad mínima de información digital y solo puede tomar dos valores: 0 o 1. Con 10 bits se pueden formar 2¹⁰ combinaciones diferentes:

> 2¹⁰ = 2 × 2 × 2 × 2 × 2 × 2 × 2 × 2 × 2 × 2 = 1,024
> 

El ADC asigna a cada combinación un número entero, empezando desde 0 (que corresponde a la combinación 0000000000 en binario) hasta 1023 (que corresponde a 1111111111 en binario). Por eso la función `analogRead()` del Arduino devuelve un valor entre 0 y 1023.

**¿Cuánto voltaje representa cada valor?**

El ADC del Arduino Mega utiliza por defecto un voltaje de referencia de 5 V. Esto quiere decir que el rango completo de 0 V a 5 V se reparte entre los 1,024 valores posibles. La resolución de voltaje es:

> Resolución = Vref / 2¹⁰ = 5 V / 1,024 ≈ 0.00488 V ≈ 4.88 mV
> 

Esto significa que el cambio mínimo de voltaje que el ADC puede detectar es de aproximadamente 4.88 mV. Una lectura de 0 corresponde a 0 V, una lectura de 1 corresponde a 4.88 mV, una lectura de 2 corresponde a 9.77 mV, y así sucesivamente hasta que una lectura de 1023 corresponde a 5 V.

Para convertir cualquier lectura del ADC a voltaje:

> V = (lectura_ADC × Vref) / 1,024 = (lectura_ADC × 5.0) / 1,024
> 

**Referencias:**
- Arduino. (s.f.). *analogRead() - Arduino Reference*. https://docs.arduino.cc/language-reference/en/functions/analog-io/analogRead/
- Arduino. (s.f.). *Arduino Mega 2560 Rev3*. https://docs.arduino.cc/hardware/mega-2560/
- Microchip Technology. (2014). *ATmega2560 Datasheet*. https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf

---

## Entorno de desarrollo y programación

Los sensores se programan en **C++** usando **PlatformIO**, una extensión que se instala dentro de Visual Studio Code.

Cuando se crea un proyecto en PlatformIO, se genera una carpeta con la siguiente estructura:

```
nombre_del_proyecto/
├── platformio.ini       ← configuración de la tarjeta y velocidad del serial
├── src/
│   └── main.cpp         ← aquí va el código del programa
└── lib/                 ← aquí van las librerías externas (si se necesitan)
```

El archivo `platformio.ini` para el Arduino Mega 2560:

```
[env:megaatmega2560]
platform= atmelavr          ; familia AVR de Microchip (ATmega2560)
board= megaatmega2560        ; tarjeta Arduino Mega 2560
framework= arduino           ; framework de Arduino
monitor_speed= 9600          ; velocidad del monitor serial en baudios
```

El código se escribe en `src/main.cpp` y tiene dos funciones: `setup()`, que se ejecuta una sola vez al encender el Arduino, y `loop()`, que se repite en ciclo continuo.

**Referencia:**
- PlatformIO. (s.f.). *Arduino Mega or Mega 2560 — PlatformIO Documentation*. https://docs.platformio.org/en/stable/boards/atmelavr/megaatmega2560.html

---

## Método de interpolación de Lagrange

### ¿Qué es interpolar?

Interpolar significa encontrar una función matemática que pase exactamente por un conjunto de puntos conocidos. En este proyecto, esos puntos se obtienen del sensor: se sabe que a cierto voltaje de entrada le corresponde cierto voltaje de salida. La función que se encuentra permite convertir cualquier lectura del sensor al valor real que se quiere medir.

### ¿Por qué se usa un polinomio?

Un polinomio es una expresión como:

> p(x) = a₀ + a₁·x + a₂·x² + … + aₙ·xⁿ
> 

Por ejemplo, `3 + 2x` es un polinomio de grado 1, y `1 + 4x + x²` es uno de grado 2. Se usan polinomios porque hay un teorema que dice: si se tienen n+1 puntos (con valores de x diferentes entre sí), existe un único polinomio de grado ≤ n que pasa por todos ellos.

**¿Por qué es único?** Supongamos que hubiera dos polinomios diferentes, P(x) y Q(x), que pasaran por los mismos n+1 puntos. Si ambos pasan por los mismos puntos, entonces en cada uno de esos puntos P y Q dan el mismo valor, lo que significa que la resta P(x) − Q(x) vale cero en n+1 puntos. Pero P(x) − Q(x) es un polinomio de grado ≤ n, y un polinomio de grado n puede valer cero en máximo n puntos. La única forma de que valga cero en n+1 puntos es que sea el polinomio cero (o sea, que P y Q sean iguales). Por lo tanto, el polinomio es único.

### ¿Cómo funciona el método de Lagrange?

Lagrange propone construir el polinomio usando funciones auxiliares llamadas **polinomios base**, una por cada punto conocido. Cada base Lᵢ(x) se construye de tal forma que:

- Que valga **1** cuando x sea el punto xᵢ.
- Que valga **0** cuando x sea cualquiera de los otros puntos.

¿Para qué sirve esto? Porque al multiplicar cada base por el valor yᵢ que le corresponde y sumar todo, el polinomio resultante pasa por todos los puntos:

> P(x) = y₀·L₀(x) + y₁·L₁(x) + … + yₙ·Lₙ(x)
> 

¿Por qué pasa por todos los puntos? Porque si se evalúa P en el punto x₀, por ejemplo, L₀(x₀) vale 1 y todas las demás bases valen 0, entonces P(x₀) = y₀·1 + y₁·0 + … = y₀. Lo mismo ocurre con cualquier otro punto.

**¿Cómo se construye cada base Lᵢ(x)?**

Cada base se arma multiplicando fracciones. Se hace una fracción por cada punto conocido que no sea el propio i. La fracción tiene en el numerador (x − xⱼ) y en el denominador (xᵢ − xⱼ):

> Lᵢ(x) = [(x − x₀)/(xᵢ − x₀)] × [(x − x₁)/(xᵢ − x₁)] × … (saltando j = i) … × [(x − xₙ)/(xᵢ − xₙ)]
> 

¿Por qué esta fórmula cumple las dos condiciones?

- Si se pone x = xᵢ (el propio punto), en cada fracción el numerador y el denominador son iguales: (xᵢ − xⱼ) / (xᵢ − xⱼ) = 1. Al multiplicar puros unos, el resultado es 1.
- Si se pone x = xₖ (cualquier otro punto), una de las fracciones va a tener en el numerador (xₖ − xₖ) = 0. Como hay un cero en la multiplicación, todo da 0.

### Lagrange con dos puntos (grado 1)

Cuando solo hay dos puntos (x₀, y₀) y (x₁, y₁), se necesitan dos bases y el polinomio resultante es de grado 1, es decir, una recta.

La base L₀(x) se construye con el punto x₁ (el que no es el suyo). Cuando x vale x₁, el numerador se hace cero y L₀ vale 0. Cuando x vale x₀, numerador y denominador son iguales y L₀ vale 1:

> L₀(x) = (x − x₁) / (x₀ − x₁)
> 

La base L₁(x) se construye igual pero con el punto x₀. Cuando x vale x₀, el numerador se hace cero y L₁ vale 0. Cuando x vale x₁, numerador y denominador son iguales y L₁ vale 1:

> L₁(x) = (x − x₀) / (x₁ − x₀)
> 

El polinomio completo queda:

> P₁(x) = y₀ · (x − x₁)/(x₀ − x₁) + y₁ · (x − x₀)/(x₁ − x₀)
> 

### Demostración: el polinomio de grado 1 es la ecuación de la recta

A continuación se demuestra que la fórmula de Lagrange con dos puntos produce la misma ecuación que la recta que pasa por esos dos puntos. Se parte de:

> P₁(x) = y₀ · (x − x₁)/(x₀ − x₁) + y₁ · (x − x₀)/(x₁ − x₀)
> 

El denominador del primer término es (x₀ − x₁). Este valor es lo mismo que −(x₁ − x₀), porque restar en orden inverso cambia el signo. Por ejemplo, si x₀ = 0 y x₁ = 5: (0 − 5) = −5 y −(5 − 0) = −5. Entonces se reescribe:

> P₁(x) = −y₀ · (x − x₁)/(x₁ − x₀) + y₁ · (x − x₀)/(x₁ − x₀)
> 

Ahora ambos términos tienen el mismo denominador (x₁ − x₀), así que se puede sacar como factor común:

> P₁(x) = [1/(x₁ − x₀)] × [y₁·(x − x₀) − y₀·(x − x₁)]
> 

Dentro del corchete hay dos multiplicaciones que se pueden expandir distribuyendo (es decir, multiplicando lo de afuera por cada término de adentro):

- y₁ · (x − x₀) se distribuye como: y₁·x − y₁·x₀
- y₀ · (x − x₁) se distribuye como: y₀·x − y₀·x₁

Sustituyendo estas distribuciones y respetando el signo negativo que ya tenía y₀:

> = [1/(x₁ − x₀)] × [y₁·x − y₁·x₀ − y₀·x + y₀·x₁]
> 

Dentro del corchete hay cuatro términos. Dos de ellos contienen la variable x (y₁·x y −y₀·x) y los otros dos no la contienen (−y₁·x₀ y y₀·x₁). Al agrupar los que tienen x se puede sacar x como factor común:

- Los que tienen x: y₁·x − y₀·x = (y₁ − y₀)·x
- Los que no tienen x: −y₁·x₀ + y₀·x₁ = y₀·x₁ − y₁·x₀

Entonces queda:

> = [1/(x₁ − x₀)] × [(y₁ − y₀)·x + (y₀·x₁ − y₁·x₀)]
> 

Al distribuir el factor 1/(x₁ − x₀) en cada sumando y reorganizar, se llega a:

> **P₁(x) = y₀ + [(y₁ − y₀) / (x₁ − x₀)] · (x − x₀)**
> 

Esta expresión es la **ecuación punto-pendiente de la recta** que pasa por el punto (x₀, y₀). La parte (y₁ − y₀) / (x₁ − x₀) es la pendiente m, que indica cuánto cambia y por cada unidad que cambia x. Es la misma fórmula que se usa en geometría analítica para encontrar la recta que pasa por dos puntos.

### ¿Por qué basta con dos puntos para sensores lineales?

Un sensor con respuesta lineal tiene una relación de la forma y = m·x + b entre su entrada y su salida. Esa relación ya es un polinomio de grado 1. Como se demostró que el polinomio interpolante es único, con dos puntos queda completamente determinado. Usar más puntos no cambiaría el resultado porque la relación ya es una recta.

**Referencias:**
- Burden, R. L., Faires, J. D. y Burden, A. M. (2016). *Numerical Analysis* (10.ª ed.). Cengage Learning.
- Chapra, S. C. y Canale, R. P. (2021). *Numerical Methods for Engineers* (8.ª ed.). McGraw-Hill Education.

---

## Sensores del proyecto

Los sensores se clasifican en dos grupos según el tipo de señal que entregan al microcontrolador.

Los **sensores analógicos** entregan un voltaje variable (entre 0 y 5 V) que se lee a través del ADC. La lectura cruda (un número de 0 a 1023) se convierte a la magnitud física real usando la fórmula obtenida por interpolación de Lagrange.

Los **sensores digitales** entregan datos ya procesados a través de un protocolo de comunicación (como 1-Wire o I2C), o bien una señal de tipo encendido/apagado (HIGH/LOW). En estos sensores la interpolación de Lagrange no se aplica.

### Sensores analógicos

| # | Sensor | Tipo de salida |
| --- | --- | --- |
| 1 | Sensor de Voltaje 0–25V DC (AR2657) | Voltaje proporcional (divisor resistivo) |
| 2 | Sensor de Corriente ACS712 (30A) | Voltaje proporcional a la corriente |
| 3 | Sensor de pH Líquido PH-4502C | Voltaje proporcional al pH |
| 4 | Fototransistor PT331C | Voltaje variable según la luz recibida |
| 5 | Sensor de Humedad de Suelo (OKY3442) | Voltaje variable según la humedad |
| 6 | Sensor de CO2 MG811 (OKY3351) | Voltaje variable según la concentración de CO2 |
| 7 | Sensor de Pulso Cardíaco (OKY3471-5) | Señal analógica de pulso cardíaco |

### Sensores digitales

| # | Sensor | Tipo de salida |
| --- | --- | --- |
| 8 | Sensor de Efecto Hall SM351LT (Honeywell) | Señal digital HIGH/LOW |
| 9 | Sensor de Temperatura DS18B20 (sumergible) | Dato digital por protocolo 1-Wire |

# Sensores analógicos

## 1. Sensor de Voltaje 0–25V DC (AR2657)

Link: https://uelectronics.com/producto/modulo-sensor-de-voltaje-0-25v-dc/

### ¿Qué es?

El módulo de voltaje 0–25V DC (SKU AR2657, UNIT Electronics) es un **divisor de voltaje resistivo**. Está formado por dos resistencias pasivas conectadas en serie dentro de una tarjeta PCB. Su función es reducir un voltaje de entrada en una proporción fija y conocida, de modo que la salida quede dentro del rango que el ADC del Arduino puede leer (0 a 5 V).

Al ser un circuito de solo dos resistencias (sin ningún componente activo), **no requiere alimentación externa para funcionar**. La corriente que necesita proviene de la misma señal que se está midiendo: el voltaje de la fuente entra por R1, baja por R2 hacia tierra, y entre ambas resistencias se toma la lectura.

**Referencia:**
- UNIT Electronics. (s.f.). *Módulo Sensor de Voltaje 0 ~ 25V DC*. https://uelectronics.com/producto/modulo-sensor-de-voltaje-0-25v-dc/

### Características eléctricas

- **Rango de voltaje de entrada:** 0 a 25 V DC.
- **Rango de voltaje de salida:** 0 a 5 V (proporcional al voltaje de entrada).
- **Resistencia R1 (lado de mayor potencial):** 30 kΩ.
- **Resistencia R2 (lado de tierra):** 7.5 kΩ.
- **Relación de división:** 5:1.
- **Tipo de señal que mide:** solo corriente directa (DC).

### Esquema eléctrico y demostración de la fórmula del divisor

A partir de los valores de R1 y R2, se puede demostrar matemáticamente la relación entre el voltaje de entrada y el de salida. El circuito dentro del módulo es el siguiente:

```
    VCC (entrada, 0-25V DC)
        │
       [R1]  30 kΩ
        │
        ├──── S (señal de salida → pin analógico del Arduino)
        │
       [R2]  7.5 kΩ
        │
       GND (tierra común)
```

Cuando se aplica un voltaje Vin en la entrada, las dos resistencias están en serie. En un circuito serie la corriente es la misma en todos los puntos, porque no hay ramificaciones por donde la corriente se pueda dividir. Esa corriente se calcula con la ley de Ohm, donde el voltaje total se divide entre la resistencia total (que es la suma de ambas):

> I = Vin / (R1 + R2)
> 

El voltaje de salida Vout es la caída de tensión que se produce sobre R2. Se calcula así porque el pin S del módulo está conectado justo entre R1 y R2, entonces lo que mide el Arduino es el voltaje que “cae” sobre R2. De nuevo por ley de Ohm (V = I × R):

> Vout = I × R2
> 

Sustituyendo la corriente I que ya se calculó:

> Vout = [Vin / (R1 + R2)] × R2
> 

Esto se puede reordenar como:

> Vout = Vin × R2 / (R1 + R2)
> 

Esta es la **fórmula general del divisor de voltaje**. Ahora se sustituyen los valores concretos de las resistencias del módulo AR2657 (R1 = 30,000 Ω y R2 = 7,500 Ω):

> Vout = Vin × 7,500 / (30,000 + 7,500)
> 

Primero se resuelve la suma del denominador:

> 30,000 + 7,500 = 37,500 Ω
> 

Entonces queda:

> Vout = Vin × 7,500 / 37,500
> 

Para simplificar esta fracción se busca un número que divida exacto tanto al 7,500 como al 37,500. Ese número es 7,500 (el propio numerador). Al dividir ambos entre 7,500:

> 7,500 ÷ 7,500 = 1
> 
> 
> 37,500 ÷ 7,500 = 5
> 

Entonces la fracción simplificada es:

> 7,500 / 37,500 = 1/5 = 0.2
> 

El resultado final es:

> **Vout = Vin / 5**
> 

Esto quiere decir que el voltaje de salida siempre es la quinta parte del voltaje de entrada. Por ejemplo, si Vin = 10 V, entonces Vout = 10/5 = 2 V. Si Vin = 25 V, entonces Vout = 25/5 = 5 V. Esta relación es lineal porque al graficar Vout contra Vin se obtiene una línea recta que pasa por el origen.

**Referencia:**
- Last Minute Engineers. (s.f.). *In-Depth: How Voltage Sensor Module Works & Interface with Arduino*. https://lastminuteengineers.com/voltage-sensor-arduino-tutorial/

### ¿Por qué el rango máximo es 25 V?

El ADC del Arduino Mega usa una referencia de 5 V. Esto significa que el voltaje máximo que puede recibir un pin analógico sin dañar la tarjeta es 5 V. Entonces la pregunta es: ¿qué voltaje de entrada produce exactamente 5 V en la salida del divisor?

Se parte de la fórmula que ya se demostró:

> Vout = Vin / 5
> 

Se sustituye Vout = 5 V (que es el máximo que el ADC puede recibir):

> 5 V = Vin / 5
> 

Para despejar Vin, se multiplican ambos lados de la ecuación por 5:

> Vin = 5 V × 5 = **25 V**
> 

El límite de 25 V no es un dato arbitrario del fabricante, sino que sale directamente de la combinación de dos cosas: la relación de división del módulo (5:1) y el voltaje de referencia del ADC (5 V). Si se aplica un voltaje mayor a 25 V, la salida del divisor supera los 5 V y puede dañar el pin analógico del Arduino.

### Resolución del sensor

Una vez establecido que el rango va de 0 a 25 V, se puede calcular qué tan precisa es la medición. Cada paso del ADC representa 4.88 mV en la salida del divisor (como se calculó en los conceptos base). Como el divisor tiene la relación 5:1, cada paso del ADC equivale a un cambio en el voltaje real de:

> Resolución real = 4.88 mV × 5 = 24.41 mV ≈ 0.0244 V
> 

**¿De dónde sale el 5?** Del factor de escala del divisor. Si la salida del divisor cambia 4.88 mV, el voltaje de entrada que produjo ese cambio tuvo que cambiar 5 veces más (porque Vin = 5 × Vout).

### Conexión al Arduino Mega 2560

El módulo tiene dos lados: una **bornera de tornillo** (donde se conecta la fuente a medir) y un **conector de tres pines** (donde se conecta al Arduino).

**Bornera de tornillo:**
- **VCC** → polo positivo de la fuente cuyo voltaje se quiere medir (máximo 25 V).
- **GND** → polo negativo de esa fuente.

**Conector de pines:**
- **S** (señal) → pin **A0** del Arduino Mega (o cualquier entrada analógica de A0 a A15).
- **–** (tierra) → pin **GND** del Arduino Mega.
- **+** → **no se conecta**. Este pin está puenteado internamente al VCC de la bornera de entrada, es decir, lleva el mismo voltaje de la fuente que se está midiendo. Si la fuente es de 12 V, el pin + tiene 12 V. Si se conectara al Arduino, ese voltaje entraría directo a la tarjeta y la dañaría. Como el módulo es un circuito pasivo (no necesita alimentación), este pin no tiene utilidad para la medición.

La tierra del módulo (pin –) y la tierra del Arduino (GND) deben estar unidas para que el ADC tenga una referencia de voltaje compartida.

### Lagrange aplicado al sensor AR2657

Con el circuito conectado y la fórmula del divisor demostrada, ahora se obtiene la ecuación de conversión usando el método de Lagrange.

**¿Por qué grado 1?**

La relación entre la entrada y la salida del módulo es Vout = Vin / 5. Si se grafica Vout en el eje horizontal y Vin en el eje vertical, se obtiene una línea recta que pasa por el origen. Una recta es un polinomio de grado 1, así que con dos puntos de calibración queda completamente determinada. No se necesita un polinomio de mayor grado porque la relación ya es lineal.

**Puntos de calibración**

Para aplicar Lagrange se necesitan dos puntos conocidos. Se define qué representa cada variable:

- x = voltaje en la salida del divisor (lo que llega al ADC, de 0 a 5 V). Se elige como x porque es el dato que se conoce (lo que el Arduino lee).
- y = voltaje real de entrada al módulo (lo que se quiere conocer, de 0 a 25 V). Se elige como y porque es el valor que se quiere calcular.

Los dos puntos se eligen en los extremos del rango porque son valores que se conocen con certeza a partir de la fórmula del divisor:

Punto 1 — cuando no se aplica voltaje (0 V de entrada), la salida del divisor también es 0 V:

> (x₀, y₀) = (0, 0)
> 

Punto 2 — cuando la entrada es 25 V (el máximo), la salida del divisor es 25/5 = 5 V:

> (x₁, y₁) = (5, 25)
> 

**Sustitución en la fórmula de Lagrange**

La fórmula general de Lagrange para dos puntos es:

> P₁(x) = y₀ · (x − x₁)/(x₀ − x₁) + y₁ · (x − x₀)/(x₁ − x₀)
> 

Se sustituyen los valores de los dos puntos. Donde dice x₀ se pone 0, donde dice y₀ se pone 0, donde dice x₁ se pone 5, y donde dice y₁ se pone 25:

> P₁(x) = 0 · (x − 5)/(0 − 5) + 25 · (x − 0)/(5 − 0)
> 

El primer término es 0 multiplicado por algo, y cualquier número multiplicado por 0 da 0. Entonces todo el primer término se anula y solo queda el segundo:

> P₁(x) = 25 · (x − 0)/(5 − 0)
> 

Se simplifica (x − 0) = x y (5 − 0) = 5:

> P₁(x) = 25 · x/5
> 

Se resuelve 25/5 = 5:

> **P₁(x) = 5x**
> 

Esto significa que para obtener el voltaje real de entrada, se multiplica el voltaje de la salida del divisor por 5. La fórmula de conversión es:

> Vreal = 5 × Vdivisor
> 

El factor 5 tiene sentido porque es exactamente el inverso de la relación del divisor (1/5). Si el divisor divide entre 5, para recuperar el valor original hay que multiplicar por 5.

**Verificación de los polinomios base**

Para confirmar que todo está bien, se verifican los polinomios base con los valores concretos (x₀, x₁) = (0, 5).

El polinomio base L₀(x) se construye con la fórmula L₀(x) = (x − x₁) / (x₀ − x₁), sustituyendo x₁ = 5 y x₀ = 0:

> L₀(x) = (x − 5) / (0 − 5)
> 

El denominador (0 − 5) es igual a −5, entonces:

> L₀(x) = (x − 5) / (−5)
> 

Dividir entre un número negativo es lo mismo que cambiar el signo del numerador. (x − 5) con signo cambiado es (5 − x):

> L₀(x) = (5 − x) / 5
> 

Esto se puede separar como dos fracciones: 5/5 − x/5 = 1 − x/5:

> L₀(x) = 1 − x/5
> 

El polinomio base L₁(x) se construye con la fórmula L₁(x) = (x − x₀) / (x₁ − x₀), sustituyendo x₀ = 0 y x₁ = 5:

> L₁(x) = (x − 0) / (5 − 0)
> 

(x − 0) es simplemente x, y (5 − 0) es 5:

> L₁(x) = x / 5
> 

Ahora se comprueba que cumplan las dos condiciones (valer 1 en su propio punto y 0 en el otro):

- L₀(0) = 1 − 0/5 = 1 ✓ (vale 1 en x₀ = 0, que es su propio punto)
- L₀(5) = 1 − 5/5 = 1 − 1 = 0 ✓ (vale 0 en x₁ = 5, que es el otro punto)
- L₁(0) = 0/5 = 0 ✓ (vale 0 en x₀ = 0, que es el otro punto)
- L₁(5) = 5/5 = 1 ✓ (vale 1 en x₁ = 5, que es su propio punto)
- L₀(x) + L₁(x) = (1 − x/5) + x/5 = 1 para cualquier x ✓ (las dos bases siempre suman 1)

Finalmente, se arma el polinomio completo sustituyendo y₀ = 0 y y₁ = 25:

> P₁(x) = 0 · L₀(x) + 25 · L₁(x) = 0 + 25 · (x/5) = 5x
> 

El resultado coincide con lo obtenido antes. La recta y = 5x pasa por (0, 0) y por (5, 25).

**Fórmula final con el ADC**

`analogRead()` devuelve un entero entre 0 y 1023. Este número no es un voltaje, es la lectura cruda del ADC. Para convertirlo a voltaje en la salida del divisor se usa la fórmula del ADC (explicada en los conceptos base):

> Vdivisor = (lectura_ADC × 5.0) / 1,024
> 

Después se aplica el factor de Lagrange para obtener el voltaje real de entrada:

> Vreal = 5 × Vdivisor
> 

Combinando ambas fórmulas en una sola expresión:

> Vreal = 5 × (lectura_ADC × 5.0) / 1,024 = (lectura_ADC × 25.0) / 1,024
> 

El 25 sale de multiplicar 5.0 (la referencia del ADC) por 5 (el factor de Lagrange). En el código se implementa en dos pasos separados para que quede claro de dónde viene cada operación:

```
Paso 1 (ADC a voltaje):       vDivisor = (lectura × 5.0) / 1024
Paso 2 (factor de Lagrange):  vReal    = vDivisor × 5
```

### Código

El siguiente programa configura el pin A0 como entrada analógica y abre la comunicación serial a 9600 baudios. En cada ciclo del `loop()`, lee el valor del ADC en el pin donde está conectado el sensor, lo convierte a voltaje usando la fórmula del ADC, le aplica el factor 5 que se obtuvo por Lagrange, e imprime el voltaje real por el monitor serial. El ciclo se repite cada segundo.

```cpp
// Archivo: src/main.cpp
// Sensor de Voltaje 0-25V DC (AR2657)
//
// Conexiones:
//   S  del módulo  →  A0 del Arduino Mega
//   -  del módulo  →  GND del Arduino Mega
//   +  del módulo  →  No conectar

#include<Arduino.h>

// --- Constantes ---
const int PIN_SENSOR = A0;         // Pin analógico donde está conectado el sensor
const float VREF = 5.0;            // Voltaje de referencia del ADC del Arduino (5V)
const float ADC_MAX = 1024.0;      // 2¹⁰ = 1024 valores posibles (de 0 a 1023)
const float FACTOR_LAGRANGE = 5.0; // Factor obtenido con Lagrange: P(x) = 5x

void setup() {
  Serial.begin(9600);              // Inicia comunicación serial a 9600 baudios
  pinMode(PIN_SENSOR, INPUT);      // Configura el pin A0 como entrada analógica
}

void loop() {
  // Lee el valor crudo del ADC (un número entre 0 y 1023)
  int lectura = analogRead(PIN_SENSOR);

  // Paso 1: convierte la lectura del ADC a voltaje en la salida del divisor
  // Fórmula: Vdivisor = (lectura × Vref) / 1024
  float vDivisor = (lectura * VREF) / ADC_MAX;

  // Paso 2: aplica el factor de Lagrange para obtener el voltaje real de entrada
  // Fórmula: Vreal = 5 × Vdivisor
  float vReal = vDivisor * FACTOR_LAGRANGE;

  // Envía el resultado por el puerto serial
  Serial.print("Voltaje: ");
  Serial.print(vReal, 2);          // Imprime el valor con 2 decimales
  Serial.println(" V");

  delay(1000);                     // Espera 1 segundo antes de repetir
}
```

### Referencias

Arduino. (s.f.). *analogRead() - Arduino Reference*. https://docs.arduino.cc/language-reference/en/functions/analog-io/analogRead/

Last Minute Engineers. (s.f.). *In-Depth: How Voltage Sensor Module Works & Interface with Arduino*. https://lastminuteengineers.com/voltage-sensor-arduino-tutorial/

UNIT Electronics. (s.f.). *Módulo Sensor de Voltaje 0 ~ 25V DC*. SKU: AR2657. https://uelectronics.com/producto/modulo-sensor-de-voltaje-0-25v-dc/

## 2. Sensor de Corriente ACS712 (30A)

Link: https://uelectronics.com/producto/sensor-de-corriente-acs712-30a/

### ¿Qué es?

El módulo sensor de corriente ACS712 de 30A (SKU AR0595, UNIT Electronics) mide la corriente eléctrica que pasa por un conductor de cobre que está dentro del módulo. Este módulo **necesita alimentación de 5 V** para funcionar, ya que tiene un circuito integrado interno que se encarga de hacer la medición.

El principio de funcionamiento es el **efecto Hall**: cuando la corriente circula por el conductor de cobre, se genera un campo magnético a su alrededor. El circuito integrado dentro del módulo detecta ese campo magnético y lo convierte en un voltaje de salida proporcional a la corriente. De esta forma, el módulo mide corriente sin que el Arduino tenga contacto eléctrico directo con el circuito donde circula la corriente.

La versión que se utiliza en este proyecto es la de 30A, que puede medir corrientes de −30 A a +30 A, tanto en corriente directa (DC) como en corriente alterna (AC).

**Referencia:**
- UNIT Electronics. (s.f.). *Sensor de Corriente ACS712 30A*. https://uelectronics.com/producto/sensor-de-corriente-acs712-30a/

### Características eléctricas

- **Rango de corriente:** −30 A a +30 A (DC o AC).
- **Voltaje de alimentación:** 5 V DC.
- **Sensibilidad:** 66 mV/A. Esto significa que por cada amperio de corriente que pasa por el sensor, el voltaje de salida cambia 66 mV (0.066 V).
- **Voltaje de salida en reposo (0 A):** 2.5 V (que es la mitad de la alimentación: 5 V / 2 = 2.5 V).
- **Rango de voltaje de salida:** 0 a 5 V.
- **Aislamiento eléctrico:** 2100 Vrms entre el lado de potencia (donde pasa la corriente) y el lado de señal (donde se conecta el Arduino).

**Referencia:**
- Allegro MicroSystems. (s.f.). *ACS712 Datasheet*. https://www.allegromicro.com/-/media/files/datasheets/acs712-datasheet.ashx

### ¿Cómo funciona la salida del sensor?

Este sensor no parte de 0 V cuando no hay corriente, sino de **2.5 V**. Este voltaje de 2.5 V es el punto medio o **offset**, y existe porque el sensor puede medir corrientes en ambas direcciones (positiva y negativa).

Cuando **no hay corriente** (0 A), la salida es 2.5 V.

Cuando la corriente **fluye en una dirección** (positiva), el voltaje de salida **sube** desde 2.5 V. Por ejemplo, si pasan 10 A, la salida sube a:

> 2.5 V + (10 A × 0.066 V/A) = 2.5 V + 0.66 V = 3.16 V
> 

Cuando la corriente **fluye en la dirección contraria** (negativa), el voltaje de salida **baja** desde 2.5 V. Por ejemplo, si pasan −10 A, la salida baja a:

> 2.5 V + (−10 A × 0.066 V/A) = 2.5 V − 0.66 V = 1.84 V
> 

La relación general entre el voltaje de salida y la corriente es:

> Vout = 2.5 + (I × 0.066)
> 

Donde Vout está en voltios e I en amperios. El 2.5 es el offset (voltaje sin corriente) y 0.066 es la sensibilidad en V/A.

**¿De dónde sale el 2.5 V?** De la alimentación del sensor. El ACS712 se alimenta con 5 V, y por diseño su salida en reposo es la mitad de la alimentación: 5 / 2 = 2.5 V. Esto le permite subir (corriente positiva) o bajar (corriente negativa) desde ese punto medio.

**¿De dónde sale el 0.066 V/A?** Es la sensibilidad de la versión de 30A, según la hoja de datos del fabricante del circuito integrado (Allegro MicroSystems). Cada versión del ACS712 tiene una sensibilidad diferente: la de 5A tiene 0.185 V/A, la de 20A tiene 0.100 V/A, y la de 30A tiene 0.066 V/A. A mayor rango de corriente, menor sensibilidad (menos voltaje de cambio por cada amperio), porque el mismo rango de salida (0 a 5 V) se reparte entre más amperios.

### ¿Por qué el rango máximo es 30 A?

El voltaje de salida máximo es 5 V (limitado por la alimentación del sensor). El offset es 2.5 V. Entonces el voltaje máximo que puede subir desde el offset es:

> 5 V − 2.5 V = 2.5 V
> 

Con una sensibilidad de 0.066 V/A, la corriente máxima que produce ese cambio de 2.5 V es:

> I = 2.5 V / 0.066 V/A = 37.87 A
> 

Pero el fabricante especifica 30 A como límite porque ahí garantiza la precisión. A corrientes mayores el sensor sigue funcionando pero la linealidad ya no está garantizada.

### Resolución del sensor

Cada paso del ADC del Arduino representa 4.88 mV (como se calculó en los conceptos base). La sensibilidad del ACS712-30A es 66 mV/A, o sea que 1 A de cambio produce 66 mV de cambio en la salida.

La resolución en corriente es:

> Resolución = 4.88 mV / 66 mV/A = 0.0739 A ≈ 73.9 mA
> 

**¿De dónde sale esta división?** Si 66 mV representan 1 A, y el ADC solo puede distinguir cambios de 4.88 mV, entonces 4.88 mV representan una fracción de amperio. Para saber cuántos amperios equivale ese cambio mínimo, se divide 4.88 entre 66.

Esto significa que el sistema puede distinguir cambios de corriente de aproximadamente 74 mA. Para corrientes pequeñas (menores a 1 A), esta resolución puede no ser suficiente.

### Conexión al Arduino Mega 2560

El módulo tiene una **bornera de tornillo** en un lado (donde pasa la corriente que se quiere medir) y **tres pines** en el otro lado (donde se conecta al Arduino).

**Bornera de tornillo (lado de potencia):**

La corriente que se quiere medir debe pasar a través de la bornera, es decir, el sensor se conecta **en serie** con el circuito. Un cable de la carga va a un terminal y el otro cable sale del otro terminal. Si se conectara en paralelo (los dos terminales al mismo punto), se haría un cortocircuito.

**Pines (lado de señal):**
- **VCC** → pin **5V** del Arduino Mega.
- **GND** → pin **GND** del Arduino Mega.
- **OUT** → pin **A1** del Arduino Mega (o cualquier entrada analógica disponible).

El ACS712 necesita los tres pines conectados porque tiene un circuito integrado interno que requiere alimentación para funcionar.

### Lagrange aplicado al sensor ACS712

**¿Por qué grado 1?**

La relación entre la corriente y el voltaje de salida del ACS712 es lineal: Vout = 2.5 + (I × 0.066). Si se grafica Vout contra I, se obtiene una recta. Con dos puntos de calibración queda determinada.

**Puntos de calibración**

Variables:

- x = voltaje de salida del sensor (lo que llega al ADC, de 0 a 5 V). Es el dato conocido.
- y = corriente real que pasa por el sensor (lo que se quiere conocer, en amperios). Es el valor que se quiere calcular.

Los puntos se eligen usando la relación del sensor:

Punto 1 — cuando no hay corriente (0 A), la salida del sensor es 2.5 V:

> (x₀, y₀) = (2.5, 0)
> 

Punto 2 — cuando pasan 30 A, la salida del sensor es 2.5 + (30 × 0.066) = 2.5 + 1.98 = 4.48 V:

> (x₁, y₁) = (4.48, 30)
> 

**¿De dónde sale el 4.48 V?** De la fórmula del sensor: Vout = 2.5 + (I × 0.066). Se sustituye I = 30: Vout = 2.5 + (30 × 0.066) = 2.5 + 1.98 = 4.48 V.

**Sustitución en la fórmula de Lagrange**

> P₁(x) = y₀ · (x − x₁)/(x₀ − x₁) + y₁ · (x − x₀)/(x₁ − x₀)
> 

Se sustituyen los valores. Donde dice x₀ se pone 2.5, y₀ se pone 0, x₁ se pone 4.48, y₁ se pone 30:

> P₁(x) = 0 · (x − 4.48)/(2.5 − 4.48) + 30 · (x − 2.5)/(4.48 − 2.5)
> 

El primer término se anula porque y₀ = 0. Solo queda el segundo:

> P₁(x) = 30 · (x − 2.5)/(4.48 − 2.5)
> 

Se resuelve el denominador: 4.48 − 2.5 = 1.98:

> P₁(x) = 30 · (x − 2.5)/1.98
> 

Se resuelve 30/1.98 = 15.15 (redondeando):

> P₁(x) = 15.15 · (x − 2.5)
> 

Pero 15.15 es lo mismo que 1/0.066. Esto tiene sentido: 0.066 es la sensibilidad del sensor en V/A, entonces 1/0.066 = 15.15 A/V es la inversa de la sensibilidad. La fórmula de conversión queda:

> **I = (Vout − 2.5) / 0.066**
> 

O equivalentemente:

> **I = (Vout − 2.5) × 15.15**
> 

**¿Qué hace esta fórmula?** Primero resta el offset de 2.5 V (para quitar la parte del voltaje que no corresponde a corriente) y luego divide entre la sensibilidad (para convertir voltios a amperios). Si Vout = 2.5 V, la corriente es (2.5 − 2.5) / 0.066 = 0 A. Si Vout = 3.16 V, la corriente es (3.16 − 2.5) / 0.066 = 0.66 / 0.066 = 10 A.

**Verificación de los polinomios base**

Con (x₀, x₁) = (2.5, 4.48):

> L₀(x) = (x − 4.48) / (2.5 − 4.48) = (x − 4.48) / (−1.98)
> 

> L₁(x) = (x − 2.5) / (4.48 − 2.5) = (x − 2.5) / 1.98
> 

Verificaciones:
- L₀(2.5) = (2.5 − 4.48) / (−1.98) = (−1.98) / (−1.98) = 1 ✓
- L₀(4.48) = (4.48 − 4.48) / (−1.98) = 0 / (−1.98) = 0 ✓
- L₁(2.5) = (2.5 − 2.5) / 1.98 = 0 / 1.98 = 0 ✓
- L₁(4.48) = (4.48 − 2.5) / 1.98 = 1.98 / 1.98 = 1 ✓

El polinomio completo:

> P₁(x) = 0 · L₀(x) + 30 · L₁(x) = 30 · (x − 2.5) / 1.98 = (x − 2.5) / 0.066
> 

Coincide con la fórmula obtenida antes.

**Fórmula final con el ADC**

`analogRead()` devuelve un entero entre 0 y 1023. Primero se convierte a voltaje:

> Vsensor = (lectura_ADC × 5.0) / 1,024
> 

Después se aplica la fórmula de Lagrange para obtener la corriente:

> I = (Vsensor − 2.5) / 0.066
> 

En el código esto se hace en tres pasos:

```
Paso 1 (ADC a voltaje):    vSensor  = (lectura × 5.0) / 1024
Paso 2 (restar offset):    vNeto    = vSensor − 2.5
Paso 3 (dividir entre sensibilidad): corriente = vNeto / 0.066
```

### Código

El siguiente programa configura el pin A1 como entrada analógica y abre la comunicación serial. En cada ciclo, lee el valor del ADC, lo convierte a voltaje, le resta el offset de 2.5 V y divide entre la sensibilidad de 0.066 V/A para obtener la corriente en amperios. El resultado se imprime por el monitor serial cada segundo.

```cpp
// Archivo: src/main.cpp
// Sensor de Corriente ACS712 30A
//
// Conexiones:
//   VCC del módulo  →  5V del Arduino Mega
//   GND del módulo  →  GND del Arduino Mega
//   OUT del módulo  →  A1 del Arduino Mega
//   Bornera         →  en serie con el circuito a medir

#include<Arduino.h>

const int PIN_SENSOR = A1;
const float VREF = 5.0;
const float ADC_MAX = 1024.0;
const float OFFSET = 2.5;         // Voltaje de salida cuando la corriente es 0 A (Vcc/2)
const float SENSIBILIDAD = 0.066; // 66 mV por cada amperio (versión 30A)

void setup() {
  Serial.begin(9600);
  pinMode(PIN_SENSOR, INPUT);
}

void loop() {
  int lectura = analogRead(PIN_SENSOR);

  // Paso 1: convertir lectura del ADC a voltaje
  float vSensor = (lectura * VREF) / ADC_MAX;

  // Paso 2: restar el offset de 2.5 V
  // Si no hay corriente, vSensor ≈ 2.5 V y vNeto ≈ 0
  float vNeto = vSensor - OFFSET;

  // Paso 3: dividir entre la sensibilidad para obtener amperios
  // Fórmula de Lagrange: I = (Vout − 2.5) / 0.066
  float corriente = vNeto / SENSIBILIDAD;

  Serial.print("Corriente: ");
  Serial.print(corriente, 2);
  Serial.println(" A");

  delay(1000);
}
```

### Referencias

Allegro MicroSystems. (s.f.). *ACS712 Datasheet — Fully Integrated, Hall-Effect-Based Linear Current Sensor IC*. https://www.allegromicro.com/-/media/files/datasheets/acs712-datasheet.ashx

Components101. (2018). *ACS712 Current Sensor Module Pinout, Specifications, Circuit & Datasheet*. https://components101.com/sensors/acs712-current-sensor-module

ElectronicsHub. (2024). *Interfacing ACS712 Current Sensor with Arduino*. https://www.electronicshub.org/interfacing-acs712-current-sensor-with-arduino/

UNIT Electronics. (s.f.). *Sensor de Corriente ACS712 30A*. SKU: AR0595. https://uelectronics.com/producto/sensor-de-corriente-acs712-30a/

## 3. Sensor de pH Líquido PH-4502C

Link: https://uelectronics.com/producto/sensor-de-ph-liquido/

### ¿Qué es?

El módulo sensor de pH líquido PH-4502C (SKU AR0288, UNIT Electronics) mide el nivel de pH (potencial de hidrógeno) de soluciones líquidas. El pH es una escala que va de 0 a 14 e indica qué tan ácido o alcalino es un líquido: un pH de 7 es neutro (como el agua pura), valores menores a 7 son ácidos (como el jugo de limón) y valores mayores a 7 son alcalinos (como el jabón).

El módulo está compuesto por dos partes: un **electrodo** (sonda E201-BNC) que se sumerge en el líquido y genera una señal eléctrica proporcional al pH, y una **tarjeta** (PH-4502C) que ajusta esa señal al rango de 0 a 5 V para que el Arduino pueda leerla. El electrodo se conecta a la tarjeta a través de un conector BNC.

El módulo **necesita alimentación de 5 V** para funcionar. Además cuenta con dos potenciómetros: uno de **offset** (etiquetado como POT 1 en algunos tutoriales, pero verificado físicamente como el potenciómetro **más cercano al conector BNC** en nuestra unidad — denominado **POT 2** en el pinout de UNIT Electronics) que sirve para calibrar la lectura analógica en el pin Po; y otro que controla una salida digital (Do) que se activa cuando el pH alcanza un límite configurado.

> **Nota de calibración (verificada en el módulo físico del proyecto):** El potenciómetro que afecta la lectura del pin Po es el **más cercano al conector BNC**. En la imagen de pinout de UNIT Electronics ese potenciómetro aparece marcado como **POT 2** ("límite de medición PH"), pero en la práctica es el que cambia el voltaje analógico de salida. La numeración en los tutoriales es inconsistente entre fabricantes; lo que importa es identificar el pot por posición física: **el más cercano al BNC es el de offset**.

**Referencia:**
- UNIT Electronics. (s.f.). *PH-4502C Sensor de PH Líquido con electrodo E201C-BNC*. https://uelectronics.com/producto/sensor-de-ph-liquido/

### Características eléctricas

- **Rango de medición de pH:** 0 a 14.
- **Voltaje de alimentación:** 5 V DC.
- **Corriente de operación:** 5 a 10 mA.
- **Rango de voltaje de salida (pin Po):** 0 a 5 V.
- **Temperatura de operación:** 0 a 60 °C.
- **Tiempo de respuesta:** ≤ 5 segundos.
- **Tiempo de estabilización:** ≤ 60 segundos.

### ¿Cómo funciona la salida del sensor?

Después de calibrar el potenciómetro de offset, la salida del pin Po entrega **2.5 V cuando el pH es 7** (neutro). A partir de ese punto:

Cuando el líquido es **ácido** (pH menor a 7), el voltaje de salida **sube** por encima de 2.5 V. Cuanto más ácido sea (pH más cercano a 0), más alto es el voltaje.

Cuando el líquido es **alcalino** (pH mayor a 7), el voltaje de salida **baja** por debajo de 2.5 V. Cuanto más alcalino sea (pH más cercano a 14), más bajo es el voltaje.

La relación aproximada entre el voltaje y el pH es de **0.18 V por cada unidad de pH**. Esto quiere decir que por cada unidad que el pH se aleja de 7, el voltaje cambia 0.18 V. Este valor de 0.18 V/pH puede variar ligeramente de un módulo a otro, por eso es necesario calibrar.

**¿De dónde sale el 2.5 V?** El módulo se alimenta con 5 V y está diseñado para que su salida en el punto neutro (pH 7) sea la mitad de la alimentación: 5 / 2 = 2.5 V. Esto permite que el voltaje pueda subir (para pH ácido) o bajar (para pH alcalino) desde ese punto medio.

**¿De dónde sale el 0.18 V/pH?** Es la sensibilidad aproximada de este módulo con el electrodo E201-BNC. Se obtiene midiendo el voltaje de salida con dos soluciones de pH conocido (pH 4.01 y pH 7.00) y calculando cuánto cambia el voltaje por cada unidad de pH. Como cada módulo puede tener un valor ligeramente diferente, la calibración es necesaria.

La relación general entre el voltaje de salida y el pH es:

> pH = 7 + (2.5 − Vout) / 0.18
> 

**¿Por qué se resta Vout de 2.5 y no al revés?** Porque la relación es inversa: cuando el pH baja (más ácido), el voltaje sube. Entonces si Vout es mayor que 2.5, el resultado de (2.5 − Vout) es negativo, y al sumarlo a 7 da un pH menor que 7 (ácido). Si Vout es menor que 2.5, el resultado es positivo y da un pH mayor que 7 (alcalino).

Por ejemplo, si Vout = 3.22 V:

> pH = 7 + (2.5 − 3.22) / 0.18 = 7 + (−0.72) / 0.18 = 7 + (−4) = 3
> 

Y si Vout = 1.78 V:

> pH = 7 + (2.5 − 1.78) / 0.18 = 7 + 0.72 / 0.18 = 7 + 4 = 11
> 

### Resolución del sensor

Cada paso del ADC del Arduino representa 4.88 mV. La sensibilidad del módulo es de aproximadamente 180 mV por unidad de pH (0.18 V/pH). La resolución en pH es:

> Resolución = 4.88 mV / 180 mV/pH = 0.027 unidades de pH
> 

**¿De dónde sale esta división?** Si 180 mV representan 1 unidad de pH, y el ADC solo puede distinguir cambios de 4.88 mV, entonces 4.88 mV representan una fracción de unidad de pH. Para saber cuántas unidades de pH equivale ese cambio mínimo, se divide 4.88 entre 180.

Esto significa que el sistema puede distinguir cambios de pH de aproximadamente 0.03 unidades, lo cual es suficiente para la mayoría de las mediciones prácticas.

### Conexión al Arduino Mega 2560

El módulo tiene un **conector BNC** en un lado (donde se conecta el electrodo) y **pines de conexión** en el otro lado.

**Conector BNC:**
- Se conecta el electrodo E201-BNC. El electrodo es la sonda que se sumerge en el líquido.

**Pines:**
- **VCC** → pin **5V** del Arduino Mega.
- **GND** → pin **GND** del Arduino Mega.
- **Po** (salida analógica) → pin **A2** del Arduino Mega.
- **Do** (salida digital) → no se usa en este proyecto.
- **To** (temperatura) → no se usa en este proyecto.

### Calibración

Antes de usar el sensor, se debe calibrar. El proceso tiene dos pasos:

Primero, se ajusta el potenciómetro de offset — el que está **más cerca del conector BNC** (verificado físicamente en nuestra unidad: es el que cambia el voltaje en el pin Po al girarlo) — hasta que la lectura de voltaje en el pin Po sea **2.5 V** cuando el BNC está cortocircuitado (se conecta el centro del BNC con la carcasa exterior con un trozo de cable). Esto se verifica con un multímetro o leyendo el voltaje desde el Arduino. La posición del potenciómetro de offset corresponde al marcado como **POT 2** en el pinout de UNIT Electronics, aunque algunos tutoriales lo llaman POT 1. Identificarlo por posición física es más confiable que por número.

Después, se sumerge el electrodo en al menos dos soluciones con pH conocido (por ejemplo una solución de pH 4.01 y otra de pH 7.00). Se anotan los voltajes que entrega el módulo para cada solución, y con esos dos puntos se calcula la sensibilidad real del módulo. Esos dos puntos son los que se usan en la interpolación de Lagrange.

### Lagrange aplicado al sensor PH-4502C

**¿Por qué grado 1?**

La relación entre el voltaje de salida y el pH es lineal: pH = 7 + (2.5 − Vout) / 0.18. Si se grafica pH contra Vout, se obtiene una recta. Con dos puntos queda determinada.

**Puntos de calibración**

Variables:

- x = voltaje de salida del módulo en el pin Po (lo que llega al ADC, de 0 a 5 V). Es el dato conocido.
- y = valor de pH del líquido (lo que se quiere conocer, de 0 a 14). Es el valor que se quiere calcular.

Los dos puntos se obtienen usando la fórmula del sensor con los valores de calibración típicos:

Punto 1 — solución buffer de pH 7 (neutra), donde la salida es 2.5 V:

> (x₀, y₀) = (2.5, 7)
> 

Punto 2 — solución buffer de pH 4 (ácida), donde la salida sube. Con una sensibilidad de 0.18 V/pH, la diferencia de pH entre 7 y 4 es 3 unidades, y cada unidad cambia 0.18 V: 3 × 0.18 = 0.54 V. Como el voltaje sube para pH ácido, la salida es 2.5 + 0.54 = 3.04 V:

> (x₁, y₁) = (3.04, 4)
> 

**¿De dónde sale el 3.04 V?** De la fórmula: Vout = 2.5 + (7 − pH) × 0.18. Se sustituye pH = 4: Vout = 2.5 + (7 − 4) × 0.18 = 2.5 + 3 × 0.18 = 2.5 + 0.54 = 3.04 V.

**Sustitución en la fórmula de Lagrange**

> P₁(x) = y₀ · (x − x₁)/(x₀ − x₁) + y₁ · (x − x₀)/(x₁ − x₀)
> 

Se sustituyen los valores. Donde dice x₀ se pone 2.5, y₀ se pone 7, x₁ se pone 3.04, y₁ se pone 4:

> P₁(x) = 7 · (x − 3.04)/(2.5 − 3.04) + 4 · (x − 2.5)/(3.04 − 2.5)
> 

Se resuelven los denominadores:

> 2.5 − 3.04 = −0.54
> 
> 
> 3.04 − 2.5 = 0.54
> 

Quedando:

> P₁(x) = 7 · (x − 3.04)/(−0.54) + 4 · (x − 2.5)/0.54
> 

Se puede factorizar 1/0.54:

> P₁(x) = (1/0.54) · [−7·(x − 3.04) + 4·(x − 2.5)]
> 

Se expanden los productos dentro del corchete:

> −7·(x − 3.04) = −7x + 21.28
> 
> 
> 4·(x − 2.5) = 4x − 10
> 

Sumando ambos:

> −7x + 21.28 + 4x − 10 = −3x + 11.28
> 

Entonces:

> P₁(x) = (−3x + 11.28) / 0.54
> 

Se puede separar esta fracción en dos partes:

> P₁(x) = −3/0.54 · x + 11.28/0.54
> 

Se resuelve cada división:

> −3 / 0.54 = −5.556
> 
> 
> 11.28 / 0.54 = 20.889
> 

Entonces:

> P₁(x) = −5.556·x + 20.889
> 

Esto se puede reescribir de forma más clara. Como −3/0.54 = −1/0.18 y sabemos que 0.18 es la sensibilidad:

> **pH = 7 + (2.5 − Vout) / 0.18**
> 

Que es la misma fórmula que se presentó al inicio. El resultado de Lagrange coincide con la relación del sensor.

**Verificación con los puntos de calibración:**

- Si Vout = 2.5 V (pH 7): pH = 7 + (2.5 − 2.5) / 0.18 = 7 + 0 = 7 ✓
- Si Vout = 3.04 V (pH 4): pH = 7 + (2.5 − 3.04) / 0.18 = 7 + (−0.54) / 0.18 = 7 − 3 = 4 ✓

**Fórmula final con el ADC**

`analogRead()` devuelve un entero entre 0 y 1023. Primero se convierte a voltaje:

> Vsensor = (lectura_ADC × 5.0) / 1,024
> 

Después se aplica la fórmula de Lagrange para obtener el pH:

> pH = 7 + (2.5 − Vsensor) / 0.18
> 

En el código esto se hace en tres pasos:

```
Paso 1 (ADC a voltaje):      vSensor = (lectura × 5.0) / 1024
Paso 2 (restar del offset):  diferencia = 2.5 − vSensor
Paso 3 (dividir entre sensibilidad y sumar a 7): pH = 7 + diferencia / 0.18
```

### Código

El siguiente programa configura el pin A2 como entrada analógica. En cada ciclo, lee el valor del ADC, lo convierte a voltaje, y aplica la fórmula obtenida por Lagrange para calcular el pH. El resultado se imprime por el monitor serial cada segundo.

```cpp
// Archivo: src/main.cpp
// Sensor de pH Líquido PH-4502C
//
// Conexiones:
//   VCC del módulo  →  5V del Arduino Mega
//   GND del módulo  →  GND del Arduino Mega
//   Po  del módulo  →  A2 del Arduino Mega
//   Electrodo E201  →  conector BNC del módulo

#include<Arduino.h>

// --- Constantes ---
const int PIN_SENSOR = A2;         // Pin analógico donde está conectado Po
const float VREF = 5.0;            // Voltaje de referencia del ADC
const float ADC_MAX = 1024.0;      // 2¹⁰ = 1024 valores posibles
const float OFFSET_PH7 = 2.5;     // Voltaje de salida cuando el pH es 7 (calibrado)
const float SENSIBILIDAD = 0.18;   // Voltaje que cambia por cada unidad de pH (V/pH)

void setup() {
  Serial.begin(9600);              // Inicia comunicación serial
  pinMode(PIN_SENSOR, INPUT);      // Configura A2 como entrada
}

void loop() {
  // Lee el valor crudo del ADC (entre 0 y 1023)
  int lectura = analogRead(PIN_SENSOR);

  // Paso 1: convierte la lectura del ADC a voltaje
  // Fórmula: Vsensor = (lectura × Vref) / 1024
  float vSensor = (lectura * VREF) / ADC_MAX;

  // Paso 2: calcula la diferencia respecto al voltaje de pH 7
  // Si el voltaje es mayor que 2.5, el pH es ácido (menor a 7)
  // Si el voltaje es menor que 2.5, el pH es alcalino (mayor a 7)
  float diferencia = OFFSET_PH7 - vSensor;

  // Paso 3: divide entre la sensibilidad y suma a 7
  // Fórmula de Lagrange: pH = 7 + (2.5 − Vout) / 0.18
  float pH = 7.0 + (diferencia / SENSIBILIDAD);

  // Envía el resultado por el puerto serial
  Serial.print("pH: ");
  Serial.print(pH, 2);             // Imprime con 2 decimales
  Serial.println();

  delay(1000);                     // Espera 1 segundo
}
```

### Referencias

Sawers Bolivia. (2022). *Módulo PH analógico PH-4502C — Tutorial*. https://sawerssrl.freshdesk.com/support/solutions/articles/31000164678-modulo-ph-analogico-ph-4502c

Talos Electronics. (2023). *¿Cómo usar módulo PH-4502 con Arduino?*. https://www.taloselectronics.com/blogs/tutoriales/como-usar-modulo-ph-4502-con-arduino

UNIT Electronics. (s.f.). *PH-4502C Sensor de PH Líquido con electrodo E201C-BNC*. SKU: AR0288. https://uelectronics.com/producto/sensor-de-ph-liquido/

## 4. Fototransistor PT331C

Link: https://agelectronica.com/detalle?busca=PT+331C

### ¿Qué es?

El PT331C (AG Electrónica) es un **fototransistor**, es decir, un transistor que se activa con la luz en lugar de con una señal eléctrica. Tiene la forma de un LED de 5 mm transparente y tiene dos pines: colector y emisor (la base no se conecta, la luz hace su función).

Cuando la luz (visible o infrarroja) llega al fototransistor, este empieza a conducir corriente. Cuanta más luz recibe, más corriente deja pasar. Cuanta menos luz, menos corriente. Si no hay luz, prácticamente no conduce.

El PT331C es un **componente suelto** que no tiene tarjeta ni circuito adicional. Para poder leerlo con el Arduino se necesita armar un circuito externo con una resistencia, que convierte la variación de corriente del fototransistor en una variación de voltaje que el ADC puede leer.

**Referencia:**
- AG Electrónica. (s.f.). *PT331C*. https://agelectronica.com/detalle?busca=PT+331C
- Everlight Electronics. (s.f.). *PT331C Datasheet — 5mm Phototransistor T-1 3/4*. https://www.alldatasheet.com/datasheet-pdf/pdf/91033/EVERLIGHT/PT331C.html

### Características eléctricas

- **Tipo:** fototransistor NPN de silicio.
- **Encapsulado:** 5 mm transparente (T-1 3/4).
- **Longitud de onda de máxima sensibilidad:** 940 nm (infrarrojo cercano).
- **Rango espectral:** 400 a 1100 nm (responde a luz visible e infrarroja).
- **Voltaje máximo colector-emisor (BVCEO):** 30 V.
- **Corriente de colector con luz (Ic on):** 0.7 a 2.5 mA (con irradiancia de 1 mW/cm² y VCE = 5 V).
- **Corriente en oscuridad (ICEO):** máximo 100 nA (prácticamente cero).
- **Tiempo de respuesta:** 15 µs (subida y bajada).

### ¿Cómo se usa con el Arduino?

Como el PT331C es un componente suelto, se necesita armar un divisor de voltaje con una resistencia. El circuito es el siguiente:

```
    5V (Arduino)
     │
    [PT331C]  colector (pin largo) arriba, emisor (pin corto) abajo
     │
     ├──── pin analógico del Arduino (A3)
     │
    [R]  10 kΩ
     │
    GND (Arduino)
```

**¿Cómo funciona este circuito?**

El fototransistor y la resistencia de 10 kΩ están conectados en serie entre 5 V y GND. El punto donde se conecta el pin analógico del Arduino está entre el fototransistor y la resistencia.

Cuando **hay luz**: el fototransistor conduce más corriente, lo que hace que caiga más voltaje sobre la resistencia de 10 kΩ. El voltaje en el punto medio (lo que lee el Arduino) **sube**.

Cuando **no hay luz**: el fototransistor casi no conduce, casi no hay corriente por la resistencia, y el voltaje en el punto medio **baja** (cerca de 0 V).

El voltaje que lee el Arduino depende de cuánta corriente deja pasar el fototransistor, que a su vez depende de cuánta luz recibe. La relación entre la luz y el voltaje de salida **no es lineal**: a poca luz el voltaje cambia más rápido, y a mucha luz se satura (deja de subir). Por esta razón, el PT331C se usa principalmente para detectar si hay luz o no (encendido/apagado), o para medir cambios relativos de luz, más que para obtener una medición absoluta precisa.

**¿Por qué 10 kΩ?** Esta resistencia determina la sensibilidad del circuito. Con una resistencia grande (por ejemplo 10 kΩ), el circuito es más sensible a poca luz pero se satura más fácil con mucha luz. Con una resistencia pequeña (por ejemplo 1 kΩ), el circuito necesita más luz para producir un cambio notable en el voltaje pero no se satura tan fácil. Se elige 10 kΩ como valor típico para la mayoría de las situaciones.

### Resolución del sensor

Como la respuesta del PT331C no es lineal, no se puede hablar de una resolución fija en unidades de luz. Lo que sí se puede decir es que cada paso del ADC sigue representando 4.88 mV en la lectura. La lectura del ADC (0 a 1023) se interpreta como un valor relativo de intensidad luminosa: valores bajos significan poca luz, valores altos significan mucha luz.

### Conexión al Arduino Mega 2560

**Componentes necesarios:**
- Fototransistor PT331C.
- Resistencia de 10 kΩ.

**Conexiones:**
- **Colector** del PT331C (pin largo) → pin **5V** del Arduino.
- **Emisor** del PT331C (pin corto) → punto de unión con la resistencia y con el pin **A3** del Arduino.
- Un extremo de la resistencia de **10 kΩ** → punto de unión con el emisor y A3.
- Otro extremo de la resistencia → pin **GND** del Arduino.

### Lagrange aplicado al fototransistor PT331C

**¿Se aplica Lagrange aquí?**

La respuesta del fototransistor **no es lineal**, así que un polinomio de grado 1 (dos puntos) no describe su comportamiento con precisión. Sin embargo, para el uso que se le da en este proyecto (detectar la presencia o ausencia de luz infrarroja, no medir una cantidad exacta de lux), la lectura cruda del ADC es suficiente.

Si se quisiera obtener una medición de intensidad luminosa en unidades físicas (lux o mW/cm²), se necesitaría calibrar el sensor con un instrumento de referencia (como un luxómetro), tomar varios puntos de medición y usar un polinomio de grado mayor (grado 2 o 3) o una tabla de interpolación. Eso queda fuera del alcance de este proyecto.

En el código, la lectura del ADC se convierte a voltaje con la fórmula del ADC y se reporta directamente:

```
Paso 1 (ADC a voltaje): vSensor = (lectura × 5.0) / 1024
```

### Código

El siguiente programa configura el pin A3 como entrada analógica. En cada ciclo, lee el valor del ADC, lo convierte a voltaje y lo imprime por el monitor serial. Valores de voltaje altos indican que el fototransistor está recibiendo más luz; valores bajos indican poca o nula luz.

```cpp
// Archivo: src/main.cpp
// Fototransistor PT331C
//
// Conexiones:
//   Colector (pin largo)  →  5V del Arduino
//   Emisor (pin corto)    →  A3 del Arduino + un extremo de R (10kΩ)
//   Otro extremo de R     →  GND del Arduino

#include<Arduino.h>

// --- Constantes ---
const int PIN_SENSOR = A3;         // Pin analógico donde está el punto medio del circuito
const float VREF = 5.0;            // Voltaje de referencia del ADC
const float ADC_MAX = 1024.0;      // 2¹⁰ = 1024 valores posibles

void setup() {
  Serial.begin(9600);              // Inicia comunicación serial
  pinMode(PIN_SENSOR, INPUT);      // Configura A3 como entrada
}

void loop() {
  // Lee el valor crudo del ADC (entre 0 y 1023)
  int lectura = analogRead(PIN_SENSOR);

  // Convierte la lectura a voltaje
  // Fórmula: Vsensor = (lectura × Vref) / 1024
  float vSensor = (lectura * VREF) / ADC_MAX;

  // Imprime la lectura cruda y el voltaje
  // Voltaje alto = más luz, voltaje bajo = menos luz
  Serial.print("ADC: ");
  Serial.print(lectura);
  Serial.print("  |  Voltaje: ");
  Serial.print(vSensor, 3);
  Serial.println(" V");

  delay(1000);                     // Espera 1 segundo
}
```

### Referencias

AG Electrónica. (s.f.). *PT331C*. https://agelectronica.com/detalle?busca=PT+331C

Everlight Electronics. (s.f.). *PT331C Datasheet — 5mm Phototransistor T-1 3/4*. https://www.alldatasheet.com/datasheet-pdf/pdf/91033/EVERLIGHT/PT331C.html

Hardware Hacking MX. (2014). *Lección 20 – Arduino + Sensor Infrarrojo (Básico)*. https://hardwarehackingmx.wordpress.com/2014/01/15/leccion-20-arduino-sensor-infrarrojo-basico/

## 5. Sensor de Humedad de Suelo (OKY3442)

Link: https://agelectronica.com/detalle?busca=OKY3442

### ¿Qué es?

El módulo sensor de humedad de suelo OKY3442 (AG Electrónica) mide qué tan húmedo o seco está el suelo. Está compuesto por dos partes: una **sonda** con dos electrodos metálicos que se inserta en la tierra, y una **tarjeta comparadora** basada en el chip LM393 que procesa la señal y entrega una salida analógica y una digital.

El módulo **necesita alimentación de 3.3 V o 5 V** para funcionar. Cuando la sonda está en suelo húmedo, la conductividad eléctrica entre los dos electrodos es mayor (porque el agua conduce electricidad), lo que produce un voltaje de salida más bajo en el pin analógico. Cuando el suelo está seco, la conductividad es menor y el voltaje de salida sube.

**Referencia:**
- AG Electrónica. (s.f.). *OKY3442 — Módulo con sensor de humedad del suelo*. https://agelectronica.com/detalle?busca=OKY3442

### Características eléctricas

- **Voltaje de alimentación:** 3.3 a 5 V DC.
- **Voltaje de salida máximo:** 4.2 V.
- **Corriente de trabajo:** 20 mA.
- **Salidas:** analógica (A0) y digital (D0).
- **Chip comparador:** LM393.
- **Potenciómetro:** ajusta el umbral de la salida digital.

### ¿Cómo funciona la salida analógica?

El pin A0 del módulo entrega un voltaje entre 0 y 4.2 V que varía según la humedad del suelo:

Cuando el suelo está **muy húmedo**: el voltaje de salida es **bajo** (cercano a 0 V). La lectura del ADC es un número bajo.

Cuando el suelo está **seco**: el voltaje de salida es **alto** (cercano a 4.2 V). La lectura del ADC es un número alto.

La relación entre la humedad real y el voltaje no es exacta ni universal: depende del tipo de suelo, la presencia de fertilizantes o minerales, y la profundidad a la que se inserta la sonda. Por esta razón, la lectura del ADC se usa como un valor relativo (0 = muy húmedo, 1023 = muy seco) y no se convierte a un porcentaje exacto de humedad sin una calibración previa en el terreno real.

### ¿Se aplica Lagrange?

La relación entre la humedad del suelo y el voltaje de salida no es lineal de forma precisa, ya que depende de muchos factores del suelo. Sin embargo, si se calibra el sensor en el terreno real con dos puntos conocidos (por ejemplo, suelo completamente seco y suelo saturado de agua), se puede usar un polinomio de grado 1 para obtener un porcentaje aproximado de humedad.

Para una calibración básica, se mide la lectura del ADC con la sonda al aire (seca) y con la sonda sumergida en agua:

- Lectura en seco (al aire): valor alto, por ejemplo 1023
- Lectura en agua: valor bajo, por ejemplo 300

Con esos dos puntos se puede mapear la lectura a un porcentaje:

> Humedad (%) = [(lectura_seco − lectura_actual) / (lectura_seco − lectura_agua)] × 100
> 

En el código se implementa directamente con esta fórmula, ajustando los valores de calibración al terreno real.

### Conexión al Arduino Mega 2560

- **VCC** del módulo → pin **5V** del Arduino Mega.
- **GND** del módulo → pin **GND** del Arduino Mega.
- **A0** del módulo (salida analógica) → pin **A4** del Arduino Mega.

### Código

El siguiente programa lee la salida analógica del sensor de humedad de suelo y convierte la lectura a un porcentaje aproximado de humedad usando dos valores de calibración. Los valores de calibración (LECTURA_SECO y LECTURA_AGUA) deben ajustarse midiendo en el terreno real.

```cpp
// Archivo: src/main.cpp
// Sensor de Humedad de Suelo (OKY3442)
//
// Conexiones:
//   VCC del módulo  →  5V del Arduino Mega
//   GND del módulo  →  GND del Arduino Mega
//   A0  del módulo  →  A4 del Arduino Mega

#include<Arduino.h>

const int PIN_SENSOR = A4;

// Valores de calibración (ajustar midiendo en el terreno real)
const int LECTURA_SECO = 1023;    // Lectura del ADC con la sonda al aire (seca)
const int LECTURA_AGUA = 300;     // Lectura del ADC con la sonda en agua

void setup() {
  Serial.begin(9600);
  pinMode(PIN_SENSOR, INPUT);
}

void loop() {
  int lectura = analogRead(PIN_SENSOR);

  // Convierte la lectura a porcentaje de humedad
  // Seco = lectura alta, húmedo = lectura baja
  float humedad = ((float)(LECTURA_SECO - lectura) / (LECTURA_SECO - LECTURA_AGUA)) * 100.0;

  // Limita el resultado entre 0 y 100
  if (humedad < 0) humedad = 0;
  if (humedad > 100) humedad = 100;

  Serial.print("Humedad: ");
  Serial.print(humedad, 1);
  Serial.println(" %");

  delay(1000);
}
```

### Referencias

AG Electrónica. (s.f.). *OKY3442 — Módulo con sensor de humedad del suelo*. https://agelectronica.com/detalle?busca=OKY3442

AG Electrónica. (s.f.). *OKY3442 Datasheet (PDF)*. https://agelectronica.lat/pdfs/textos/O/OKY3442.PDF

## 6. Sensor de CO2 MG811 (OKY3351)

Link: https://agelectronica.com/detalle?busca=OKY3351

### ¿Qué es?

El módulo sensor de CO2 OKY3351 (AG Electrónica) mide la concentración de dióxido de carbono (CO2) en el aire. Está basado en el sensor MG811, que funciona con el principio de **celda de electrolito sólido**: tiene dos electrodos que reaccionan frente a la presencia de CO2 y generan un voltaje proporcional a la concentración del gas.

El módulo **necesita alimentación de 6 V DC**. Esto es porque el sensor MG811 tiene un calentador interno que necesita esa tensión para operar correctamente. El calentador consume alrededor de 200 mA, así que no se puede alimentar directamente desde el pin de 5 V del Arduino, se necesita una fuente externa de 6 V.

El módulo tiene salida analógica y salida digital (con potenciómetro para ajustar el umbral de alarma).

**Referencia:**
- AG Electrónica. (s.f.). *OKY3351 — Módulo sensor de CO2 MG811*. https://agelectronica.com/detalle?busca=OKY3351

### Características eléctricas

- **Voltaje de alimentación:** 6 V DC (fuente externa).
- **Corriente de calentamiento:** ~200 mA.
- **Rango de detección:** 350 a 10,000 ppm de CO2.
- **Rango de salida analógica:** variable, depende de la concentración de CO2 (mayor voltaje = menor concentración).
- **Salidas:** analógica (AOUT) y digital (DOUT).
- **Tiempo de calentamiento:** requiere varios minutos para estabilizarse.

### ¿Cómo funciona la salida analógica?

La salida analógica del módulo tiene un comportamiento **inverso**: cuanto mayor es la concentración de CO2, **menor** es el voltaje de salida. Cuanto menor es la concentración de CO2, **mayor** es el voltaje.

El rango de salida del módulo OKY3351 depende del circuito amplificador que trae la tarjeta. A una concentración normal de CO2 en el aire (alrededor de 400 ppm), el voltaje de salida es de aproximadamente 4 a 4.5 V. A concentraciones altas (por ejemplo 4,000 ppm), el voltaje baja a alrededor de 3 a 3.5 V.

### ¿Se aplica Lagrange?

Sí. La relación entre el voltaje de salida y la concentración de CO2 es aproximadamente lineal dentro de un rango limitado. Para obtener la concentración en ppm se necesitan dos puntos de calibración:

Punto 1 — al aire libre, donde la concentración de CO2 es de aproximadamente 400 ppm. Se mide el voltaje de salida (por ejemplo, 4.5 V).

Punto 2 — en un ambiente con alta concentración de CO2 (por ejemplo, respirando dentro de una bolsa cerrada, donde la concentración llega a unos 4,000 ppm). Se mide el voltaje de salida (por ejemplo, 3.2 V).

Con esos dos puntos (x₀ = 4.5, y₀ = 400) y (x₁ = 3.2, y₁ = 4000), se aplica Lagrange:

> P₁(x) = y₀ · (x − x₁)/(x₀ − x₁) + y₁ · (x − x₀)/(x₁ − x₀)
> 

> P₁(x) = 400 · (x − 3.2)/(4.5 − 3.2) + 4000 · (x − 4.5)/(3.2 − 4.5)
> 

Los denominadores son:

> 4.5 − 3.2 = 1.3
> 
> 
> 3.2 − 4.5 = −1.3
> 

Quedando:

> P₁(x) = 400 · (x − 3.2)/1.3 + 4000 · (x − 4.5)/(−1.3)
> 

> P₁(x) = (1/1.3) · [400·(x − 3.2) − 4000·(x − 4.5)]
> 

Expandiendo:

> 400·(x − 3.2) = 400x − 1280
> 
> 
> 4000·(x − 4.5) = 4000x − 18000
> 

Sustituyendo:

> = (1/1.3) · [400x − 1280 − 4000x + 18000]
> 
> 
> = (1/1.3) · [−3600x + 16720]
> 
> = −2769.2·x + 12861.5
> 

La pendiente es negativa, lo cual tiene sentido: cuando el voltaje baja, la concentración de CO2 sube.

**Verificación con los puntos de calibración:**

- Si vSensor = 4.5 V (aire libre): CO2 = 400 + ((4.5 − 4.5) / (4.5 − 3.2)) × 3600 = 400 + 0 = 400 ppm ✓
- Si vSensor = 3.2 V (ambiente cerrado): CO2 = 400 + ((4.5 − 3.2) / (4.5 − 3.2)) × 3600 = 400 + 3600 = 4,000 ppm ✓

**Nota:** Los valores de voltaje (4.5 V y 3.2 V) son aproximados y varían de un sensor a otro. Cada sensor debe calibrarse individualmente.

### Conexión al Arduino Mega 2560

- **VCC** del módulo → **fuente externa de 6 V** (no al pin de 5 V del Arduino).
- **GND** del módulo → pin **GND** del Arduino Mega (tierra común con la fuente de 6 V).
- **AOUT** del módulo (salida analógica) → pin **A5** del Arduino Mega.

### Código

El siguiente programa lee la salida analógica del sensor de CO2 y la convierte a concentración en ppm usando la fórmula obtenida por Lagrange. Los valores de calibración (V_400PPM y V_4000PPM) deben obtenerse midiendo con el sensor en los dos ambientes de referencia.

```cpp
// Archivo: src/main.cpp
// Sensor de CO2 MG811 (OKY3351)
//
// Conexiones:
//   VCC  del módulo  →  Fuente externa de 6V
//   GND  del módulo  →  GND del Arduino Mega (tierra común)
//   AOUT del módulo  →  A5 del Arduino Mega

#include<Arduino.h>

const int PIN_SENSOR = A5;
const float VREF = 5.0;
const float ADC_MAX = 1024.0;

// Valores de calibración (ajustar con mediciones reales)
const float V_400PPM = 4.5;       // Voltaje medido al aire libre (~400 ppm)
const float V_4000PPM = 3.2;      // Voltaje medido en ambiente con ~4000 ppm

void setup() {
  Serial.begin(9600);
  pinMode(PIN_SENSOR, INPUT);
}

void loop() {
  int lectura = analogRead(PIN_SENSOR);

  // ADC a voltaje
  float vSensor = (lectura * VREF) / ADC_MAX;

  // Lagrange: CO2 = 400 + (V_400PPM - vSensor) × (3600 / 1.3)
  // Pendiente = (4000 - 400) / (3.2 - 4.5) = 3600 / (-1.3) = -2769.2
  float co2 = 400.0 + ((V_400PPM - vSensor) / (V_400PPM - V_4000PPM)) * (4000.0 - 400.0);

  Serial.print("CO2: ");
  Serial.print(co2, 0);
  Serial.println(" ppm");

  delay(2000);
}
```

### Referencias

AG Electrónica. (s.f.). *OKY3351 — Módulo sensor de CO2 MG811*. https://agelectronica.com/detalle?busca=OKY3351

Prometec. (s.f.). *Sensor CO2 MG811*. https://www.prometec.net/sensor-co2-mg811/

## 7. Sensor de Pulso Cardíaco (OKY3471-5)

Link: https://agelectronica.com/detalle?busca=OKY3471-5

### ¿Qué es?

El sensor de pulso cardíaco OKY3471-5 (AG Electrónica) mide el ritmo del corazón (pulso) a través de la piel. Se coloca en la punta de un dedo o en el lóbulo de la oreja y detecta los latidos del corazón.

El sensor funciona con un principio óptico: tiene un **LED verde** que ilumina la piel y un **fotodetector** (APDS-9008) que mide cuánta luz se refleja. Cada vez que el corazón late, la sangre fluye por los vasos del dedo y cambia la cantidad de luz que se refleja. El sensor detecta esos cambios y los convierte en una señal eléctrica. Esta señal pasa por un amplificador (MCP6001) y un filtro de ruido que están dentro del módulo, y sale como un voltaje analógico por el pin de señal.

El módulo **necesita alimentación de 3.3 V o 5 V** y consume muy poca corriente (4 mA a 5 V). Tiene tres cables: rojo (VCC), negro (GND) y morado o amarillo (señal analógica).

**Referencia:**
- AG Electrónica. (s.f.). *OKY3471-5 — Sensor de pulso cardíaco*. https://agelectronica.com/detalle?busca=OKY3471-5

### Características eléctricas

- **Voltaje de alimentación:** 3.0 a 5.5 V DC.
- **Corriente de consumo:** 4 mA (a 5 V).
- **Sensor óptico:** APDS-9008.
- **Amplificador:** MCP6001.
- **LED:** verde.
- **Salida:** analógica (voltaje variable).
- **Cables:** 3 (VCC, GND, señal).

### ¿Cómo funciona la salida?

La salida del sensor es una **señal analógica que varía con el tiempo**. No entrega un voltaje fijo proporcional a una magnitud (como el sensor de voltaje o el de corriente), sino una señal que sube y baja con cada latido del corazón, similar a una onda.

Cuando el corazón **late**, la sangre fluye con más fuerza por el dedo y la señal sube (pico). Entre latido y latido, la señal baja (valle). El tiempo entre un pico y el siguiente es el período de un latido.

Para obtener las pulsaciones por minuto (BPM), se cuentan cuántos picos ocurren en un intervalo de tiempo. Esto se puede hacer midiendo el tiempo entre dos picos consecutivos y calculando:

> BPM = 60,000 / tiempo_entre_picos_ms
> 

**¿De dónde sale el 60,000?** Un minuto tiene 60 segundos, y 60 segundos son 60,000 milisegundos. Si el tiempo entre dos picos es de 750 ms, entonces en un minuto caben 60,000 / 750 = 80 latidos, es decir, 80 BPM.

### ¿Se aplica Lagrange?

No de la forma habitual. La señal de este sensor no es un voltaje proporcional a una magnitud fija que se pueda convertir con una fórmula. Es una señal que varía en el tiempo y lo que interesa es detectar los picos (latidos) y medir el tiempo entre ellos.

El procesamiento de la señal se hace con la librería **PulseSensor Playground**, que se encarga de detectar los picos, filtrar el ruido y calcular los BPM. No se necesita aplicar Lagrange ni convertir el voltaje a otra unidad.

### Conexión al Arduino Mega 2560

- **Cable rojo** (VCC) → pin **5V** del Arduino Mega.
- **Cable negro** (GND) → pin **GND** del Arduino Mega.
- **Cable morado/amarillo** (señal) → pin **A6** del Arduino Mega.

El sensor se coloca en la punta del dedo o en el lóbulo de la oreja. Es importante que no le llegue luz ambiental directamente, porque eso interfiere con la lectura.

### Código

El siguiente programa usa la librería **PulseSensor Playground** para detectar los latidos y calcular los BPM. La librería se instala desde PlatformIO agregando `pulsesensor/PulseSensor Playground` en el archivo `platformio.ini`. En cada ciclo, la librería analiza la señal del sensor y cuando detecta un latido imprime los BPM por el monitor serial.

Para instalar la librería, se agrega esta línea al `platformio.ini`:

```
[env:megaatmega2560]
platform= atmelavr
board= megaatmega2560
framework= arduino
monitor_speed=9600
lib_deps= pulsesensor/PulseSensor Playground
```

```cpp
// Archivo: src/main.cpp
// Sensor de Pulso Cardíaco (OKY3471-5)
//
// Conexiones:
//   Cable rojo     →  5V del Arduino Mega
//   Cable negro    →  GND del Arduino Mega
//   Cable señal    →  A6 del Arduino Mega

#include<Arduino.h>
#include<PulseSensorPlayground.h>

// --- Constantes ---
const int PIN_SENSOR = A6;        // Pin analógico donde está la señal del sensor
const int UMBRAL = 550;           // Umbral para detectar un latido (ajustar si es necesario)

// --- Objeto del sensor ---
PulseSensorPlayground pulseSensor;

void setup() {
  Serial.begin(9600);

  // Configura la librería del sensor de pulso
  pulseSensor.analogInput(PIN_SENSOR);  // Pin donde está conectado
  pulseSensor.setThreshold(UMBRAL);     // Umbral de detección de latido

  // Inicia el sensor
  if (pulseSensor.begin()) {
    Serial.println("Sensor de pulso iniciado");
  }
}

void loop() {
  // La librería revisa constantemente la señal
  // Cuando detecta un latido, calcula los BPM
  if (pulseSensor.sawStartOfBeat()) {
    int bpm = pulseSensor.getBeatsPerMinute();

    Serial.print("Latido detectado | BPM: ");
    Serial.println(bpm);
  }

  delay(20);   // Pausa corta para que la librería procese la señal
}
```

### Referencias

AG Electrónica. (s.f.). *OKY3471-5 — Sensor de pulso cardíaco*. https://agelectronica.com/detalle?busca=OKY3471-5

PulseSensor. (s.f.). *PulseSensor Playground — Arduino Library*. https://github.com/WorldFamousElectronics/PulseSensorPlayground

Tecneu. (2024). *Monitor de Ritmo Cardíaco: Arduino y Sensor de Pulso XD58C*. https://www.tecneu.com/blogs/tutoriales-de-electronica/tutorial-creacion-de-un-monitor-de-ritmo-cardiaco-con-arduino-y-un-sensor-de-pulso-xd58c

# Sensores digitales

## 8. Sensor de Efecto Hall SM351LT (Honeywell)

Link: https://agelectronica.com/detalle?busca=SM351LT

### ¿Qué es?

El SM351LT (AG Electrónica) es un sensor magnético fabricado por Honeywell. Detecta la presencia de un imán cercano. Su salida es **digital**: entrega HIGH (voltaje alto) o LOW (voltaje bajo) dependiendo de si hay un imán cerca o no. No entrega un voltaje proporcional a la distancia ni a la fuerza del campo magnético, solo indica si el campo supera un umbral o no.

El SM351LT es **omnipolar**, lo que significa que se activa con cualquier polo del imán (norte o sur). No importa de qué lado se acerque el imán, el sensor lo detecta.

Es un componente muy pequeño en encapsulado SOT-23 (3 pines) y consume muy poca corriente (360 nA en promedio).

**Referencia:**
- AG Electrónica. (s.f.). *SM351LT*. https://agelectronica.com/detalle?busca=SM351LT

### Características eléctricas

- **Tipo de salida:** digital (HIGH/LOW), push-pull.
- **Voltaje de alimentación:** 1.65 V a 5.5 V DC.
- **Corriente de consumo:** 360 nA promedio.
- **Umbral de activación:** 7 Gauss (típico), máximo 11 Gauss.
- **Tipo de detección:** omnipolar (detecta polo norte y polo sur).
- **Encapsulado:** SOT-23 (3 pines: VDD, GND, OUT).
- **Rango de temperatura:** −40 °C a 85 °C.

**¿De dónde salen los 7 Gauss?** Es el umbral de activación según la hoja de datos de Honeywell. Si el campo magnético que llega al sensor es de al menos 7 Gauss, la salida cambia de estado. Un imán pequeño de neodimio genera campos de varios cientos de Gauss a pocos milímetros de distancia, así que el sensor detecta imanes a distancias de varios centímetros.

### ¿Cómo funciona la salida?

La salida del SM351LT es tipo **push-pull**. Esto quiere decir que el propio sensor pone la salida en HIGH o LOW sin necesitar una resistencia externa.

Cuando **no hay imán cerca** (campo magnético menor a 7 Gauss): la salida está en **HIGH** (voltaje cercano a VDD).

Cuando **hay un imán cerca** (campo magnético de 7 Gauss o más): la salida cambia a **LOW** (voltaje cercano a 0 V).

Cuando **el imán se retira**: la salida regresa a HIGH.

Como la salida es digital (solo dos estados), **no se usa el ADC ni `analogRead()`**. En su lugar se usa `digitalRead()`, que devuelve 1 (HIGH) o 0 (LOW).

### ¿Se aplica Lagrange?

No. La interpolación de Lagrange sirve para convertir una lectura analógica a una magnitud física. El SM351LT no entrega una lectura analógica, solo dos estados: hay imán (LOW) o no hay imán (HIGH). No hay conversión que hacer.

### Conexión al Arduino Mega 2560

El SM351LT tiene 3 pines. Visto desde el frente (con el texto legible), de izquierda a derecha son: VDD, OUT, GND.

- **VDD** → pin **5V** del Arduino Mega (también funciona con 3.3V).
- **GND** → pin **GND** del Arduino Mega.
- **OUT** → pin **digital 22** del Arduino Mega (o cualquier pin digital disponible).

No necesita resistencia externa porque la salida es push-pull.

### Código

El siguiente programa configura el pin digital 22 como entrada. En cada ciclo, lee el estado del sensor con `digitalRead()`. Si la lectura es LOW, significa que hay un imán cerca. Si es HIGH, no hay imán. El resultado se imprime por el monitor serial cada 500 milisegundos.

```cpp
// Archivo: src/main.cpp
// Sensor de Efecto Hall SM351LT (Honeywell)
//
// Conexiones:
//   VDD del sensor  →  5V del Arduino Mega
//   GND del sensor  →  GND del Arduino Mega
//   OUT del sensor  →  Pin digital 22 del Arduino Mega

#include<Arduino.h>

// --- Constantes ---
const int PIN_SENSOR = 22;        // Pin digital donde está conectado OUT

void setup() {
  Serial.begin(9600);              // Inicia comunicación serial
  pinMode(PIN_SENSOR, INPUT);      // Configura pin 22 como entrada digital
}

void loop() {
  // Lee el estado del sensor: HIGH o LOW
  int estado = digitalRead(PIN_SENSOR);

  // LOW = hay imán cerca, HIGH = no hay imán
  if (estado == LOW) {
    Serial.println("Iman detectado");
  } else {
    Serial.println("Sin iman");
  }

  delay(500);                      // Espera 500 ms
}
```

### Referencias

AG Electrónica. (s.f.). *SM351LT*. https://agelectronica.com/detalle?busca=SM351LT

Honeywell. (s.f.). *SM351LT Nanopower Series Magnetoresistive Sensor IC Datasheet*. https://www.alldatasheet.com/datasheet-pdf/pdf/644306/HONEYWELL-ACC/SM351LT.html

## 9. Sensor de Temperatura DS18B20 (sumergible)

Link: https://uelectronics.com/producto/ds18b20-sensor-de-temperatura-digital-de-acero-inoxidable-sumergible/

### ¿Qué es?

El sensor de temperatura DS18B20 en versión sumergible (SKU AR0135, UNIT Electronics) mide la temperatura y la entrega como un **dato digital** a través del protocolo **1-Wire**. Viene encapsulado en un tubo de acero inoxidable sellado con un cable de aproximadamente 1 metro, lo que permite sumergirlo en líquidos o enterrarlo en suelo.

El DS18B20 es un sensor **digital**, lo que significa que no entrega un voltaje analógico que haya que convertir con el ADC. El sensor hace la medición internamente, la convierte a un número digital y la envía al Arduino por un solo cable de datos usando el protocolo 1-Wire. El Arduino recibe directamente el valor de temperatura en grados Celsius, sin necesidad de fórmulas de conversión ni interpolación de Lagrange.

El sensor **necesita alimentación de 3.3 V o 5 V** y además necesita una **resistencia de pull-up de 4.7 kΩ** entre el cable de datos y VCC para que la comunicación 1-Wire funcione correctamente.

**Referencia:**
- UNIT Electronics. (s.f.). *DS18B20 Sensor de Temperatura Digital de Acero Inoxidable Sumergible*. https://uelectronics.com/producto/ds18b20-sensor-de-temperatura-digital-de-acero-inoxidable-sumergible/

### Características eléctricas

- **Rango de medición:** −55 °C a +125 °C.
- **Precisión:** ±0.5 °C (en el rango de −10 °C a +85 °C).
- **Resolución:** configurable entre 9 y 12 bits. Con 12 bits, la resolución es de 0.0625 °C.
- **Voltaje de alimentación:** 3.0 V a 5.5 V DC.
- **Protocolo de comunicación:** 1-Wire (un solo cable de datos).
- **Tiempo de conversión:** 750 ms (con resolución de 12 bits).
- **Encapsulado:** tubo de acero inoxidable sellado, cable de ~1 metro.

**¿De dónde sale la resolución de 0.0625 °C?** Con 12 bits, el sensor puede representar 2¹² = 4,096 valores diferentes. El rango total del sensor es de −55 °C a +125 °C, que son 180 °C. Si se divide el rango entre los valores: 180 / 4,096 = 0.0439 °C por paso. Sin embargo, el fabricante (Maxim/Dallas) especifica en la hoja de datos que la resolución de 12 bits es de 0.0625 °C, porque el sensor internamente representa la temperatura como un número de 12 bits donde el bit menos significativo equivale a 1/16 de grado: 1/16 = 0.0625 °C.

### ¿Cómo funciona?

El DS18B20 tiene un termómetro digital interno. Cuando el Arduino le pide una lectura, el sensor mide la temperatura, la convierte a un número de 12 bits y la envía por el cable de datos usando el protocolo 1-Wire.

El protocolo 1-Wire permite que varios dispositivos se conecten al mismo cable de datos (cada DS18B20 tiene una dirección única grabada de fábrica). En este proyecto se usa solo un sensor, pero si se necesitaran más, podrían compartir el mismo cable.

La resistencia de **4.7 kΩ** entre el cable de datos y VCC (pull-up) es necesaria porque el protocolo 1-Wire usa una línea de datos que los dispositivos “jalan” a LOW para comunicarse. Cuando nadie está transmitiendo, la resistencia mantiene la línea en HIGH. Sin esta resistencia, la comunicación no funciona.

### ¿Se aplica Lagrange?

No. El DS18B20 entrega la temperatura ya convertida a grados Celsius. No hay una lectura analógica que convertir con una fórmula. El Arduino solo necesita leer el dato digital que le envía el sensor.

### Conexión al Arduino Mega 2560

El sensor sumergible tiene **tres cables** que salen del tubo de acero inoxidable. Los colores pueden variar según el fabricante, pero los más comunes son:

- **Cable rojo** (VCC) → pin **5V** del Arduino Mega.
- **Cable negro** (GND) → pin **GND** del Arduino Mega.
- **Cable amarillo o blanco** (datos) → pin **digital 7** del Arduino Mega (o cualquier pin digital).
- **Resistencia de 4.7 kΩ** → conectada entre el cable de datos y el pin de 5V (pull-up).

```
    5V (Arduino)
     │
    [4.7 kΩ]  resistencia de pull-up
     │
     ├──── cable de datos (amarillo) → pin digital 7 del Arduino
     │
    DS18B20 (cable amarillo)
     │
    GND (cable negro)
```

### Código

El siguiente programa usa las librerías **OneWire** y **DallasTemperature** para comunicarse con el DS18B20. Se instalan desde PlatformIO. En cada ciclo, el Arduino le pide al sensor que mida la temperatura, espera a que termine la conversión y lee el resultado en grados Celsius. El valor se imprime por el monitor serial cada segundo.

Para instalar las librerías, se agregan al `platformio.ini`:

```
[env:megaatmega2560]
platform= atmelavr
board= megaatmega2560
framework= arduino
monitor_speed=9600
lib_deps=
  paulstoffregen/OneWire
  milesburton/DallasTemperature
```

```cpp
// Archivo: src/main.cpp
// Sensor de Temperatura DS18B20 (sumergible)
//
// Conexiones:
//   Cable rojo      →  5V del Arduino Mega
//   Cable negro     →  GND del Arduino Mega
//   Cable amarillo  →  Pin digital 7 del Arduino Mega
//   Resistencia 4.7 kΩ entre cable amarillo y 5V (pull-up)

#include<Arduino.h>
#include<OneWire.h>
#include<DallasTemperature.h>

// --- Constantes ---
const int PIN_DATOS = 7;           // Pin digital donde está el cable de datos

// --- Objetos ---
OneWire oneWire(PIN_DATOS);        // Comunicación 1-Wire en el pin 7
DallasTemperature sensor(&oneWire); // Librería del DS18B20

void setup() {
  Serial.begin(9600);              // Inicia comunicación serial
  sensor.begin();                  // Inicia el sensor DS18B20
}

void loop() {
  // Le pide al sensor que mida la temperatura
  sensor.requestTemperatures();

  // Lee la temperatura en grados Celsius del primer sensor encontrado
  float temperatura = sensor.getTempCByIndex(0);

  // Imprime el resultado
  Serial.print("Temperatura: ");
  Serial.print(temperatura, 2);    // 2 decimales
  Serial.println(" °C");

  delay(1000);                     // Espera 1 segundo
}
```

### Referencias

Maxim Integrated. (s.f.). *DS18B20 Programmable Resolution 1-Wire Digital Thermometer Datasheet*. https://www.analog.com/media/en/technical-documentation/data-sheets/DS18B20.pdf

Miles Burton. (s.f.). *DallasTemperature Arduino Library*. https://github.com/milesburton/Arduino-Temperature-Control-Library

Paul Stoffregen. (s.f.). *OneWire Arduino Library*. https://github.com/PaulStoffregen/OneWire

UNIT Electronics. (s.f.). *DS18B20 Sensor de Temperatura Digital de Acero Inoxidable Sumergible*. SKU: AR0135. https://uelectronics.com/producto/ds18b20-sensor-de-temperatura-digital-de-acero-inoxidable-sumergible/