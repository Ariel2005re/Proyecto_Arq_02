/*
  =========================================================
  SISTEMA B - DETECCION POR INTERRUPCION (ISR)
  Proyecto A2 - Arquitectura de Computadores
  Placa: ESP32
  =========================================================

  IDEA DEL EXPERIMENTO:
  La CPU ejecuta la misma "tarea pesada" que el Sistema A
  (secuencia de LEDs con delay() bloqueante). Pero ahora el
  boton esta conectado a una interrupcion de hardware (ISR).

  Cuando el boton se presiona, el microcontrolador DETIENE
  momentaneamente lo que esta haciendo (sin importar en que
  parte de la secuencia de LEDs este) y ejecuta la ISR de
  inmediato. Ahi mismo se apagan los LEDs y se registra el
  instante exacto de reaccion.

  Esto demuestra que las interrupciones no dependen de que
  el codigo "llegue" a revisar el boton: el hardware avisa
  a la CPU en cuanto ocurre el evento.

  NOTA: en el ESP32 (a diferencia del Arduino Uno) casi
  cualquier GPIO soporta attachInterrupt, por eso podemos
  usar el GPIO 17 sin ningun truco especial.
*/

// ---------- CONFIGURACION DE PINES (segun cableado real) ----------
const int LED_PINS[] = {23, 22, 21, 19, 18}; // amarillo, verde, rojo, verde, verde
const int NUM_LEDS = 5;
const int BUTTON_PIN = 17;     // Boton (señal), soporta interrupcion sin problema
const int LED_DELAY_MS = 150;  // Duracion de la "tarea pesada" por paso

// ---------- VARIABLES DE LA ISR ----------
volatile unsigned long retardoMedido_us = 0;  // tiempo YA calculado DENTRO de la ISR
volatile bool interrupcionPendiente = false;  // bandera para reportar por Serial
volatile unsigned long ultimaInterrupcion = 0; // para antirrebote

// ---------- VARIABLES DE LA SECUENCIA DE LEDS ----------
int ledActual = 0;

// Declaracion explicita de la ISR (necesaria en ESP32 por el atributo IRAM_ATTR)
void IRAM_ATTR ISR_boton();

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(LED_PINS[i], OUTPUT);
  }
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), ISR_boton, FALLING);

  Serial.println("=== SISTEMA B (INTERRUPCION) LISTO ===");
  Serial.println("Presiona el boton para medir el retardo de deteccion.");
}

void loop() {
  // ---------- REPORTE DE LA MEDICION (solo Serial.print, NO la reaccion) ----------
  // La reaccion real (apagar LEDs) ya ocurrio dentro de la ISR.
  // Aqui solo informamos el resultado por el Monitor Serial.
  if (interrupcionPendiente) {
    // El valor ya fue calculado DENTRO de la ISR (ver mas abajo).
    // Aqui solo lo mostramos, no volvemos a medir nada.
    Serial.print("INTERRUPCION -> Retardo de respuesta: ");
    Serial.print(retardoMedido_us / 1000.0, 3);
    Serial.println(" ms");
    interrupcionPendiente = false;
    delay(500); // pausa visual antes de reanudar la secuencia
  }

  // ---------- TAREA PESADA: secuencia de LEDs (BLOQUEANTE) ----------
  apagarTodosLosLeds();
  digitalWrite(LED_PINS[ledActual], HIGH);
  ledActual = (ledActual + 1) % NUM_LEDS;
  delay(LED_DELAY_MS);
}

// ---------- ISR: reacciona INMEDIATAMENTE al presionar el boton ----------
// IRAM_ATTR asegura que la funcion se cargue en RAM (requisito del ESP32
// para rutinas de interrupcion, no existe en Arduino Uno).
void IRAM_ATTR ISR_boton() {
  unsigned long ahora = millis();
  if (ahora - ultimaInterrupcion > 200) { // antirrebote simple
    unsigned long inicio = micros(); // instante en que el hardware avisa a la CPU

    // Reaccion inmediata: apagar los LEDs sin esperar al loop()
    for (int i = 0; i < NUM_LEDS; i++) {
      digitalWrite(LED_PINS[i], LOW);
    }

    unsigned long fin = micros(); // instante en que la reaccion ya se completo

    // Este SI es el retardo real: lo que tarda la CPU en reaccionar
    // desde que el hardware la interrumpe hasta que termina de apagar los LEDs.
    retardoMedido_us = fin - inicio;

    interrupcionPendiente = true; // solo para que el loop() reporte el dato
    ultimaInterrupcion = ahora;
  }
}

void apagarTodosLosLeds() {
  for (int i = 0; i < NUM_LEDS; i++) {
    digitalWrite(LED_PINS[i], LOW);
  }
}
