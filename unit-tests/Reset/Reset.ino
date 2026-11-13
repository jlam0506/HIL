/*
  Description: Reset the device if button is pressed down for 3s
  Date: November, 2024
*/

#include "../../pins.h"

// Inline definintion of activation or deactivation of peripherals
inline bool button_pressed(void) {
  constexpr uint32_t BUTTON_PRESS_RESET_DURATION = 3000;
  if (digitalRead(GPIO_INPUT_BUTTON) == LOW) {
    uint32_t button_pressed_timestamp = millis();
    while (digitalRead(GPIO_INPUT_BUTTON) == LOW) {
      if (millis() - button_pressed_timestamp > BUTTON_PRESS_RESET_DURATION) {
        Serial.println("The device will reset now\n\n");
        delay(100);
        ESP.restart();
      }
    }
    return true;
  }
  return false;
}


void setup() {
  // Initialize the Universal Asynchronous Receive-Transmit (UART) Communication Interface:
  Serial.begin(DEBUG_BAUD_RATE);
  delay(DELAY_ON_BOOT);
  Serial.printf("KOLIBRI HARDWARE REVISION: %s\n", KOLIBRI_HARDWARE_REVISION);
  Serial.println("Testing the reset feature of the device\nReset the system if button is pressed down longer than 3s");

  // Initialize the Input Button
  pinMode(GPIO_INPUT_BUTTON, INPUT_PULLUP);
}

void loop() {
  // Reset the system if button is pressed down longer than 3s
  if (button_pressed());
}
