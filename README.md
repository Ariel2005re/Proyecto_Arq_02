# Proyecto A2 — Monitoreo de Buses e Interrupciones con Arduino y ESP32

**Curso:** Arquitectura de Computadores — Escuela Politécnica Nacional

Proyecto que compara físicamente el rendimiento de dos estrategias de gestión de Entrada/Salida frente a un evento externo : **Polling (sondeo)** vs. **Interrupciones de Hardware**, midiendo el tiempo de respuesta real en milisegundos/microsegundos.

## Integrantes del equipo

| Rol | Responsabilidad |
|---|---|
| Aitana Gomez | Hardware y Electrónica — ensamblaje de protoboards y pulsadores |
| Ariel Recalde | Programación (Firmware) — código de ambos sistemas |
| Raquel Armijo | Pruebas y Documentación — informe técnico y repositorio |


## Descripción del experimento

Ambos sistemas ejecutan la misma "tarea pesada", una secuencia de 5 LEDs encendiéndose uno por uno con pausas bloqueantes `delay()`, mientras intentan detectar la pulsación de un botón:

- **Sistema A (Arduino Uno):** detecta el botón por **polling**, revisando su estado (`digitalRead()`) solo cuando el `delay()` en curso termina.
- **Sistema B (ESP32):** detecta el botón mediante una **interrupción de hardware (ISR)**, reaccionando de inmediato sin importar en qué parte de la secuencia de LEDs se encuentre la CPU.

## Hardware necesario

| Cantidad | Componente |
|---|---|
| 1 | Arduino Uno |
| 1 | ESP32 Dev Module |
| 10 | LEDs (5 por sistema) |
| 10 | Resistencias 220–330 Ω (para los LEDs) |
| 2 | Pulsadores táctiles (push button) |
| 1 | Protoboard |
| Cables | Dupont / alambre sólido para protoboard |

## Conexiones

### Sistema A — Arduino Uno (Polling)

| Componente | Pin |
|---|---|
| LED 1 (verde) | Digital 6 |
| LED 2 (azul) | Digital 5 |
| LED 3 (amarillo) | Digital 4 |
| LED 4 (verde) | Digital 3 |
| LED 5 (rojo) | Digital 2 |
| Botón | Digital 7 |

> Nota: el pin 7 no soporta `attachInterrupt` estándar en el Uno (solo los pines 2 y 3 lo soportan). El código usa **Pin Change Interrupt (PCINT)** exclusivamente como cronómetro de referencia para medir el instante real de la pulsación; la reacción del sistema sigue siendo 100% por polling.

### Sistema B — ESP32 (Interrupción)

| Componente | Pin (GPIO) |
|---|---|
| LED 1 (amarillo) | GPIO 23 |
| LED 2 (verde) | GPIO 22 |
| LED 3 (rojo) | GPIO 21 |
| LED 4 (verde) | GPIO 19 |
| LED 5 (verde) | GPIO 18 |
| Botón | GPIO 17 |

> El ESP32 soporta `attachInterrupt` en casi cualquier GPIO, por lo que no se necesita ningún truco especial (a diferencia del Arduino Uno).


## Cómo ejecutar el código

1. Instala el [Arduino IDE](https://www.arduino.cc/en/software) (versión 2.x recomendada).
2. Para programar el ESP32, agrega en **File → Preferences → Additional Boards Manager URLs**:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. **Sistema A:**
   - Abre `Sistema_A/Sistema_A_Polling.ino`.
   - Selecciona la placa **Arduino Uno** y el puerto correspondiente.
   - Sube el código.
4. **Sistema B:**
   - Abre `Sistema_B/Sistema_B_Interrupcion.ino`.
   - Selecciona la placa **ESP32 Dev Module** y el puerto correspondiente.
   - Sube el código (si no conecta, mantén presionado el botón **IO0/BOOT** mientras dice "Connecting...").
5. Abre el **Monitor Serial** a **115200 baudios** en ambos casos.
6. Presiona el botón físico en distintos momentos de la secuencia de LEDs y observa el retardo de respuesta impreso.

## Resultados obtenidos

| Métrica | Sistema A (Polling) | Sistema B (Interrupción) |
|---|---|---|
| Retardo mínimo | 0.236 ms | 0.003 ms |
| Retardo máximo | 143.592 ms | 0.004 ms |
| Retardo promedio | ≈ 63.44 ms | ≈ 0.0036 ms |
| Consistencia | Muy variable (depende del momento de la secuencia) | Prácticamente constante |
| Orden de magnitud | Milisegundos | Microsegundos |

El Sistema B respondió, en promedio, **más de 17,000 veces más rápido** que el Sistema A, demostrando cuantitativamente la ventaja de las interrupciones de hardware frente al polling cuando el tiempo de respuesta es crítico.

Ver el informe técnico completo en [`/Informe`](./Informe) para el análisis detallado, marco teórico y conclusiones.

## Estructura del repositorio

```
Proyecto-A2-Polling-vs-Interrupciones/
├── README.md
├── Sistema_A/
│   └── Sistema_A_Polling.ino
├── Sistema_B/
│   └── Sistema_B_Interrupcion.ino
├── Informe/
│   └── Informe_Tecnico_A2.pdf
└── Evidencias/
    ├── captura_sistema_A.png
    └── captura_sistema_B.png
```
