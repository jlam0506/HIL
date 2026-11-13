/*
  Description: Red, Green, Blue (RGB) LED Configuration and Selection using PWM
  Date: November, 2024
*/

#include "../../pins.h"

// LED Pulse Width Modulation (PWM) Analog Output Definitions
#define LED_PWM_TIMER_RESOLUTION    8                   // 8 bit (ie. 0 to 255)
#define LED_PWM_BASE_FREQUENCY      5000                // In Hz
#define LED_R_PWM_CHANNEL           3
#define LED_G_PWM_CHANNEL           4
#define LED_B_PWM_CHANNEL           5
#define LED_PWM_CHANNEL             LED_G_PWM_CHANNEL   // Just for completeness
#define LED_PWM_DUTY_CYCLE          128
#define NO_PWM_DUTY_CYCLE           0

// RGB (Red Green Blue) LED Data Structure
typedef struct RGB_LABEL
{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} RGB;

// Instantiate Objects for the different colours we use
#if defined(KOLIBRI_HARDWARE_REVISION_13)
RGB red {128, 0, 0};
RGB green {0, 128, 0};
RGB blue {0, 0, 128};
RGB yellow {64, 128, 0};
RGB white {128, 128, 128};
RGB black {0, 0, 0}; // Alias: No Colour
#elif defined(KOLIBRI_HARDWARE_REVISION_15)
RGB red {128, 255, 255};
RGB green {255, 128, 255};
RGB blue {255, 255, 128};
RGB yellow {64, 128, 255};
RGB white {128, 128, 128};
RGB black {255, 255, 255}; // Alias: No Colour
#endif

// Inline definintion of activation or deactivation of peripherals
inline bool button_pressed(void) {
  return digitalRead(GPIO_INPUT_BUTTON) == LOW;
}

inline void turn_on_3v3_switch(void) {
  #if defined (KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, LOW);
  #elif defined (KOLIBRI_HARDWARE_REVISION_15)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, HIGH);
  #endif
}

void change_colour(RGB& colour) {
  ledcWriteChannel(LED_R_PWM_CHANNEL, colour.red);
  ledcWriteChannel(LED_G_PWM_CHANNEL, colour.green);
  ledcWriteChannel(LED_B_PWM_CHANNEL, colour.blue);
}


void boot_completion_indication(void) {
  int16_t brightness = 0;  // how bright the LED is
  int16_t fadeAmount = 5;  // how many points to fade the LED by
  for (int16_t i = 0; i < 255; i++) {
    // Set the brightness on LEDC channel 0
    ledcWriteChannel(LED_PWM_CHANNEL, brightness);

    // Change the brightness for next time through the loop:
    brightness = brightness + fadeAmount;

    // Reverse the direction of the fading at the ends of the fade:
    if (brightness <= 0 || brightness >= 255) {
      fadeAmount = -fadeAmount;
    }

    // Wait for 30 milliseconds to see the dimming effect
    delay(20);
  }
}

void setup() {
  // Initialize the Universal Asynchronous Receive-Transmit (UART) Communication Interface:
  Serial.begin(DEBUG_BAUD_RATE);
  delay(DELAY_ON_BOOT);
  Serial.printf("KOLIBRI HARDWARE REVISION: %s\n", KOLIBRI_HARDWARE_REVISION);

  // Turn on the 3V3 Peripheral Subsystem. It is Active low (It is called 3V3Enable in the schematics)
  pinMode(GPIO_OUTPUT_V3V3P_ENABLE, OUTPUT);
  turn_on_3v3_switch();

  // Initialize the Input Button
  pinMode(GPIO_INPUT_BUTTON, INPUT_PULLUP);

  // Initialize the PWM controller for all the LEDs
  ledcAttachChannel(GPIO_OUTPUT_LED_RED, LED_PWM_BASE_FREQUENCY, LED_PWM_TIMER_RESOLUTION, LED_R_PWM_CHANNEL);
  ledcAttachChannel(GPIO_OUTPUT_LED_GREEN, LED_PWM_BASE_FREQUENCY, LED_PWM_TIMER_RESOLUTION, LED_G_PWM_CHANNEL);
  ledcAttachChannel(GPIO_OUTPUT_LED_BLUE, LED_PWM_BASE_FREQUENCY, LED_PWM_TIMER_RESOLUTION, LED_B_PWM_CHANNEL);
  
  change_colour(green);
  boot_completion_indication();
  change_colour(green);
}

void loop() {
  // Change LED colours whenever the button is pressed
  if (button_pressed()) {
    change_colour(red);
    delay(2000);
    change_colour(black);
    delay(1000);
    change_colour(green);
    delay(2000);
    change_colour(black);
    delay(1000);
    change_colour(blue);
    delay(2000);
    change_colour(black);
    delay(1000);
    change_colour(yellow);
    delay(2000);
    change_colour(black);
    delay(1000);
    change_colour(white);
    delay(2000);
    change_colour(black);
    delay(1000);
  }
}