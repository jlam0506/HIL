/*
  Description: Vibrate for 6s (3s weak vibration, 3s strong vibration) whenever a button is pressed
  Date: November, 2024
*/

#include "../../pins.h"

// Motor Pulse Width Modulation (PWM) Analog Output Definitions
#define MOTOR_PWM_TIMER_RESOLUTION  8                   // 8 bit (ie. 0 to 255)
#define MOTOR_PWM_BASE_FREQUENCY    200                 // In Hz
#define MOTOR_PWM_CHANNEL           6
#define MOTOR_PWM_DUTY_CYCLE        60
#define NO_PWM_DUTY_CYCLE           0

// Empty, Weak, Strong and No Vibration Definitions
#define VIBRATION_A                 (MOTOR_PWM_DUTY_CYCLE)
#define VIBRATION_B                 (MOTOR_PWM_DUTY_CYCLE + 40)
#define EMPTY_VIBRATION             (MOTOR_PWM_DUTY_CYCLE + 80)
#define NO_VIBRATION                NO_PWM_DUTY_CYCLE
#define WEAK_VIBRATION              VIBRATION_A
#define STRONG_VIBRATION            VIBRATION_B

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

inline void weak_vibration(void) {
  Serial.println("Vibration On: Weak");
  ledcWriteChannel(MOTOR_PWM_CHANNEL, WEAK_VIBRATION);
}

inline void strong_vibration(void) {
  Serial.println("Vibration On: Strong");
  ledcWriteChannel(MOTOR_PWM_CHANNEL, STRONG_VIBRATION);
}

/*
inline void strong_vibration(void) {
  Serial.println("Vibration On: Strong");
  ledcWriteChannel(MOTOR_PWM_CHANNEL, STRONG_VIBRATION);
  delay(500);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, NO_VIBRATION);
  delay(50);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, STRONG_VIBRATION);
  delay(500);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, NO_VIBRATION);
  delay(50);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, STRONG_VIBRATION);
  delay(500);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, NO_VIBRATION);
  delay(50);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, STRONG_VIBRATION);
  delay(500);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, NO_VIBRATION);
  delay(50);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, STRONG_VIBRATION);
  delay(500);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, NO_VIBRATION);
  delay(50);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, STRONG_VIBRATION);
  delay(500);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, NO_VIBRATION);
  delay(50);
}
*/

inline void empty_vibration(void) {
  Serial.println("Vibration On: Empty");
  ledcWriteChannel(MOTOR_PWM_CHANNEL, EMPTY_VIBRATION);
  delay(200);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, NO_VIBRATION);
  delay(100);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, EMPTY_VIBRATION);
  delay(200);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, NO_VIBRATION);
  delay(100);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, EMPTY_VIBRATION);
  delay(200);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, NO_VIBRATION);
  delay(100);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, EMPTY_VIBRATION);
  delay(200);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, NO_VIBRATION);
  delay(100);
}

inline void stop_vibration(void) {
  Serial.println("Vibration Off");
  ledcWriteChannel(MOTOR_PWM_CHANNEL, NO_VIBRATION);
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

  // Initialize the PWM controller for the Motor (Haptics/Vibration)
  ledcAttachChannel(GPIO_OUTPUT_MOTOR, MOTOR_PWM_BASE_FREQUENCY, MOTOR_PWM_TIMER_RESOLUTION, MOTOR_PWM_CHANNEL);
}

void loop() {
  // Vibrate for 6s (3s weak vibration, 3s strong vibration) whenever a button is pressed
  if (button_pressed()) {
    weak_vibration();
    delay(3000);
    stop_vibration();
    delay(3000);
    strong_vibration();
    delay(3000);
    stop_vibration();
    delay(3000);
    empty_vibration();
    delay(3000);
    stop_vibration();
    delay(3000);
  }
}