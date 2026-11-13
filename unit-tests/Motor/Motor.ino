#include "../../pins.h"

inline void turn_on_3v3_switch(void) {
  #if defined (KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, LOW);
  #elif defined (KOLIBRI_HARDWARE_REVISION_15)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, HIGH);
  #endif
}

// Inline definintion of activation or deactivation of peripherals
inline bool button_pressed(void) {
  return digitalRead(GPIO_INPUT_BUTTON) == LOW;
}

// The setup function runs once when you press reset or power the board
void setup() {
  // Initialize the Universal Asynchronous Receive-Transmit (UART) Communication Interface:
  Serial.begin(DEBUG_BAUD_RATE);
  delay(DELAY_ON_BOOT);
  Serial.printf("KOLIBRI HARDWARE REVISION: %s\n", KOLIBRI_HARDWARE_REVISION);
  
  // Enable Power Supply to Subsystems
  pinMode(GPIO_OUTPUT_V3V3P_ENABLE, OUTPUT);
  turn_on_3v3_switch();

  // Initialize the Input Button
  pinMode(GPIO_INPUT_BUTTON, INPUT_PULLUP);

  // Initialize digital pin LED_BUILTIN as an output.
  pinMode(GPIO_OUTPUT_MOTOR, OUTPUT);
}

// The loop function runs over and over again forever
void loop() {
  if (button_pressed()) {
    digitalWrite(GPIO_OUTPUT_MOTOR, HIGH);  // Turn the LED on (HIGH is the voltage level)
    delay(1000);                            // wait for a second
    digitalWrite(GPIO_OUTPUT_MOTOR, LOW);   // turn the LED off by making the voltage LOW
    delay(1000);                            // wait for a second
  }
  delay(50);
}
