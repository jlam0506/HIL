/*
  Description: Test all the three LED by blinking
  Date: November, 2024
*/

#include "../../pins.h"

// Select the LED you want to test by uncommenting one of the lines below 
#define RED_LED                 GPIO_OUTPUT_LED_RED
#define GREEN_LED               GPIO_OUTPUT_LED_GREEN
#define BLUE_LED                GPIO_OUTPUT_LED_BLUE

/*
#if defined (RED_LED)
#define LED_BUILTIN_RED                 GPIO_OUTPUT_LED_RED
#elif defined (GREEN_LED)
#define LED_BUILTIN1                 GPIO_OUTPUT_LED_GREEN
#elif defined (BLUE_LED)
#define LED_BUILTIN2                 GPIO_OUTPUT_LED_BLUE
#endif 
*/

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


// the setup function runs once when you press reset or power the board
void setup() {
  // Initialize the Universal Asynchronous Receive-Transmit (UART) Communication Interface:
  Serial.begin(DEBUG_BAUD_RATE);
  delay(DELAY_ON_BOOT);
  Serial.printf("KOLIBRI HARDWARE REVISION: %s\n", KOLIBRI_HARDWARE_REVISION);
  
  // Enable Power Supply to Subsystems
  pinMode(GPIO_OUTPUT_V3V3P_ENABLE, OUTPUT);
  turn_on_3v3_switch();

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  turn_on_led(RED_LED);             // turn the LED on (HIGH is the voltage level)
  delay(1000);                      // wait for a second
  turn_off_led(RED_LED);            // turn the LED off by making the voltage LOW
  delay(1000);                      // wait for a second
  turn_on_led(GREEN_LED);     
  delay(1000);                  
  turn_off_led(GREEN_LED);       
  delay(1000);                 
  turn_on_led(BLUE_LED);     
  delay(1000);        
  turn_off_led(BLUE_LED);    
  delay(1000);  
}
