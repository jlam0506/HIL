#include "../../pins.h"

inline void turn_on_3v3_switch(void) {
  #if defined (KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, LOW);
  #elif defined (KOLIBRI_HARDWARE_REVISION_15)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, HIGH);
  #endif
}

void turn_on_led(int pin) {
  #if defined (KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(pin, HIGH);
  #elif defined (KOLIBRI_HARDWARE_REVISION_15)
  digitalWrite(pin, LOW);
  #endif
}

void turn_off_led(int pin) {
  #if defined (KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(pin, LOW);
  #elif defined (KOLIBRI_HARDWARE_REVISION_15)
  digitalWrite(pin, HIGH);
  #endif
}

inline bool button_pressed(void) {
  return digitalRead(GPIO_INPUT_BUTTON) == LOW;
}

void setup() {
  // Initialize the Universal Asynchronous Receive-Transmit (UART) Communication Interface:
  Serial.begin(DEBUG_BAUD_RATE);
  delay(DELAY_ON_BOOT);
  Serial.printf("KOLIBRI HARDWARE REVISION: %s\n", KOLIBRI_HARDWARE_REVISION);
  // Enable Power Supply to Subsystems
  pinMode(GPIO_OUTPUT_V3V3P_ENABLE, OUTPUT);
  turn_on_3v3_switch();
  
  // Initialize the LED pin as an output:
  pinMode(GPIO_OUTPUT_LED_RED, OUTPUT);
  
  // Initialize the pushbutton pin as an input:
  #if defined (KOLIBRI_HARDWARE_REVISION_13)
  pinMode(GPIO_INPUT_BUTTON, INPUT);
  #elif defined (KOLIBRI_HARDWARE_REVISION_15)
  pinMode(GPIO_INPUT_BUTTON, INPUT_PULLUP);
  #endif
}

void loop() {
  // Check if the pushbutton is pressed. If so, turn on the LED
  if (button_pressed()) {
    turn_on_led(GPIO_OUTPUT_LED_RED);
  } else {
    turn_off_led(GPIO_OUTPUT_LED_RED);
  }
}
