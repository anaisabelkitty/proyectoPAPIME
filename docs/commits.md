# Convención de commits

## Formato

```
<tipo>(<alcance>): <descripción>
```

- Una sola línea, sin punto al final.
- Máximo 72 caracteres.
- En español, minúsculas.

---

## Tipos

| Tipo | Cuándo usarlo |
|------|---------------|
| `feat` | Se agrega algo que no existía |
| `fix` | Se corrige algo que no funcionaba bien |
| `refactor` | Se reorganiza o limpia código sin cambiar su comportamiento |
| `docs` | Solo cambia documentación |
| `test` | Se agrega o ajusta código de prueba |
| `chore` | Cambios de configuración, dependencias o archivos auxiliares |

---

## Alcance (opcional)

Indica qué parte del proyecto cambió. Usa el nombre del sensor, módulo o componente:

- `ph`, `voltaje`, `corriente`, `hall`, `temperatura`
- `lcd`, `uart`, `esp32`, `mega`
- `android`, `web`

---

## Ejemplos

```
feat(ph): agregar cálculo de pH a partir de voltaje

fix(ph): corregir offset en calibración con buffer pH 7

refactor(ph): separar lectura de voltaje y cálculo de pH

docs: agregar convención de commits

chore: actualizar platformio.ini con entorno esp32
```

---

## Reglas

- **Un commit = un cambio concreto.** Si tocaste tres cosas distintas, haz tres commits.
- No pongas "varios cambios", "wip" ni "arreglos varios".
- Si el cambio afecta varios módulos sin un alcance claro, omite el paréntesis: `refactor: reorganizar estructura de carpetas`.
- No uses el cuerpo del commit a menos que el título no sea suficiente para entender el por qué.
