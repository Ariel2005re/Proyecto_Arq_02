/*
  =========================================================
  SISTEMA A - DETECCION POR POLLING (SONDEO)
  Proyecto A2 - Arquitectura de Computadores
  Placa: Arduino Uno
  =========================================================

  IDEA DEL EXPERIMENTO:
  La CPU ejecuta una "tarea pesada" (secuencia de LEDs con
  delay() bloqueante). Mientras esa tarea corre, el boton
  SOLO se revisa cuando el codigo llega a la linea del
  digitalRead(), es decir, DESPUES de que termina el delay().

  Esto demuestra que el polling puede "perderse" el instante
  exacto en el que ocurre el evento externo, generando un
  retardo de respuesta variable e impredecible.

  MEDICION:
  El boton esta en el pin 7, que NO soporta la interrupcion
  estandar de Arduino (attachInterrupt solo funciona en pines
  2 y 3). Por eso usamos "Pin Change Interrupt" (PCINT), un
  segundo tipo de interrupcion de hardware que SI funciona en
  cualquier pin digital (0 al 7 en este caso).

  Esta interrupcion se usa UNICAMENTE como "cronometro de
  referencia" (no como parte de la logica de reaccion del
  sistema) para saber el instante REAL en que se presiono el
  boton. Luego, cuando el polling finalmente lo detecta con
  digitalRead(), calculamos cuanto tiempo paso entre ambos
  eventos.
*/

// ---------- CONFIGURACION DE PINES (segun cableado real) ----------
const int LED_PINS[] = {6, 5, 4, 3, 2}; // verde, azul, amarillo, verde, rojo
const int NUM_LEDS = 5;
const int BUTTON_PIN = 7;      // Pulsador (no soporta attachInterrupt, usamos PCINT)
const int LED_DELAY_MS = 150;  // Duracion de la "tarea pesada" por paso

// ---------- VARIABLES DE MEDICION (cronometro de referencia) ----------
volatile unsigned long tiempoPulsacion = 0;
volatile bool botonPresionadoFlag = false;
volatile unsigned long ultimaInterrupcion = 0; // para antirrebote

// ---------- VARIABLES DE LA SECUENCIA DE LEDS ----------
int ledActual = 0;

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(LED_PINS[i], OUTPUT);
  }
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // ---- Habilitar Pin Change Interrupt en el pin 7 ----
  // El pin 7 pertenece al grupo PCINT2 (pines 0 a 7 del puerto D)
  PCICR |= (1 << PCIE2);     // Activa el grupo de interrupciones PCINT2
  PCMSK2 |= (1 << PCINT23);  // Activa especificamente el pin 7 (PCINT23 = D7)

  Serial.println("=== SISTEMA A (POLLING) LISTO ===");
  Serial.println("Presiona el boton para medir el retardo de deteccion.");
}

void loop() {
  // ---------- TAREA PESADA: secuencia de LEDs (BLOQUEANTE) ----------
  apagarTodosLosLeds();
  digitalWrite(LED_PINS[ledActual], HIGH);
  ledActual = (ledActual + 1) % NUM_LEDS;
  delay(LED_DELAY_MS); // <-- Aqui la CPU queda "ciega" al boton

  // ---------- DETECCION POR POLLING ----------
  // Esto solo se ejecuta DESPUES de que el delay() anterior termino
  if (digitalRead(BUTTON_PIN) == LOW && botonPresionadoFlag) {
    unsigned long tiempoDeteccion = micros();
    unsigned long retardo_us = tiempoDeteccion - tiempoPulsacion;

    apagarTodosLosLeds(); // reaccion (tardia) del sistema

    Serial.print("POLLING -> Retardo de respuesta: ");
    Serial.print(retardo_us / 1000.0, 3);
    Serial.println(" ms");

    botonPresionadoFlag = false; // listo para la siguiente prueba
    delay(500); // pausa visual antes de reanudar la secuencia
  }
}

// ---------- ISR de referencia (SOLO mide, no reacciona) ----------
// Esta funcion se llama automaticamente cuando CUALQUIER pin
// del grupo PCINT2 cambia de estado. Por eso verificamos que
// sea justamente nuestro boton (pin 7) el que causo el cambio.
ISR(PCINT2_vect) {
  if (digitalRead(BUTTON_PIN) == LOW) {
    unsigned long ahora = millis();
    if (ahora - ultimaInterrupcion > 200) { // antirrebote simple
      tiempoPulsacion = micros();
      botonPresionadoFlag = true;
      ultimaInterrupcion = ahora;
    }
  }
}

void apagarTodosLosLeds() {
  for (int i = 0; i < NUM_LEDS; i++) {
    digitalWrite(LED_PINS[i], LOW);
  }
}
