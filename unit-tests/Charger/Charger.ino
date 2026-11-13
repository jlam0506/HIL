#include "../../pins.h"

inline void turn_on_3v3_switch(void) {
  #if defined (KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, LOW);
  #elif defined (KOLIBRI_HARDWARE_REVISION_15)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, HIGH);
  #endif
}

inline void turn_off_3v3_switch(void) {
  #if defined (KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, HIGH);
  #elif defined (KOLIBRI_HARDWARE_REVISION_15)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, LOW);
  #endif
}

inline bool charging (void) {
  #if defined (KOLIBRI_HARDWARE_REVISION_13)
  return digitalRead(GPIO_INPUT_WIRELESS_CHARGER) == HIGH;
  #elif defined (KOLIBRI_HARDWARE_REVISION_15)
  return digitalRead(GPIO_INPUT_WIRELESS_CHARGER) == LOW;
  #endif
}

// The setup function runs once when you press reset or power the board
void setup() {
  // Initialize the Universal Asynchronous Receive-Transmit (UART) Communication Interface:
  Serial.begin(DEBUG_BAUD_RATE);
  delay(DELAY_ON_BOOT);
  Serial.printf("KOLIBRI HARDWARE REVISION: %s\n", KOLIBRI_HARDWARE_REVISION);

  // Disable Power Supply to Subsystems
  pinMode(GPIO_OUTPUT_V3V3P_ENABLE, OUTPUT);
  turn_off_3v3_switch();

  // Initialize digital pin GPIO_INPUT_WIRELESS_CHARGER as an input.
  pinMode(GPIO_INPUT_WIRELESS_CHARGER, INPUT);
}

// The loop function runs over and over again forever
void loop() {
  if (charging()) {
    Serial.println("Kolibri is charging");
  } else {
    Serial.println("Kolibri is NOT charging");
  }
  delay(1000);
}
