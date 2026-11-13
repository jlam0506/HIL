/*
  Description: Combines all the devices that use PWM for Nebulization, Vibration and LED Control
  Note: The LEDC Peripheral has 4 timers and 8 channel outputs. So we can have 1 timer to control 8 channels (1:8), 2 timer to control 2x 4 channels (2:4) ...etc
  For this example the PWM Channels are chosen to match the timer (i.e. timer = (channel/2) % 4) since we want to drive them with different frequencies.
  Therefore we use timer 1 for channel 0 for Piezo, skip/unused channel 1,
                   timer 2 for channel 3 for Red LED, skip/unused channel 2,
                   timer 3 for channel 4 for Green  LED,
                   timer 3 for channel 5 for Blue LED,
                   timer 4 for channel 6 for Motor,
                   timer 4 for channel 7 for Buzzer.
                   Because RGB LEDs are running on the same frequency therefore they can share timers.
  In summary you can only specify a maximum of four different frequency at a time
  Date: November, 2024
*/

#include <Wire.h>
#include <MP2731.h>
#include <MCP40DXX.h>
#include "../../pins.h"

// I2C Addresses and Clock Frequency Definitions
#define MP2731_I2C_ADDRESS          0x4B
#define MCP40D18_I2C_ADDRESS        0x2E
#define I2C_CLOCK_FREQUENCY         400000

// All PWM Definitions: Piezo PWM, LED PWM, Motor PWM
#define NO_PWM_DUTY_CYCLE           0

// Piezo Pulse Width Modulation (PWM) Analog Output Definitions
#define PIEZO_PWM_TIMER_RESOLUTION  4       // 4 bit
#define PIEZO_PWM_BASE_FREQUENCY    143000  // In Hz
#define PIEZO_PWM_CHANNEL           0
#define PIEZO_PWM_DUTY_CYCLE        8

// LED Pulse Width Modulation (PWM) Analog Output Definitions
#define LED_PWM_TIMER_RESOLUTION    8                   // 8 bit (ie. 0 to 255)
#define LED_PWM_BASE_FREQUENCY      5000                // In Hz
#define LED_R_PWM_CHANNEL           3
#define LED_G_PWM_CHANNEL           4
#define LED_B_PWM_CHANNEL           5
#define LED_PWM_CHANNEL             LED_R_PWM_CHANNEL   // Just for completeness
#define LED_PWM_DUTY_CYCLE          128

// Motor Pulse Width Modulation (PWM) Analog Output Definitions
#define MOTOR_PWM_TIMER_RESOLUTION  8                   // 8 bit (ie. 0 to 255)
#define MOTOR_PWM_BASE_FREQUENCY    200                 // In Hz
#define MOTOR_PWM_CHANNEL           6
#define MOTOR_PWM_DUTY_CYCLE        60

// Buzzer Pulse Width Modulation (PWM) Analog Output Definitions
#define BUZZER_PWM_TIMER_RESOLUTION  8                   // 8 bit (ie. 0 to 255)
#define BUZZER_PWM_BASE_FREQUENCY    3100                // 3100Hz ± 500Hz is the resonant frequency of the buzzer
#define BUZZER_PWM_CHANNEL           7
#define BUZZER_PWM_DUTY_CYCLE        128
#define NO_PWM_DUTY_CYCLE            0

// Empty, Weak, Strong and No Vibration Definitions
#define VIBRATION_A                 (MOTOR_PWM_DUTY_CYCLE)
#define VIBRATION_B                 (MOTOR_PWM_DUTY_CYCLE + 40)
#define EMPTY_VIBRATION             (MOTOR_PWM_DUTY_CYCLE + 80)
#define NO_VIBRATION                NO_PWM_DUTY_CYCLE
#define WEAK_VIBRATION              VIBRATION_A
#define STRONG_VIBRATION            VIBRATION_B

using namespace MP2731_NS;
using namespace MCP40DXX_NS;

MP2731 power(MP2731_I2C_ADDRESS);
MCP40DXX potentiometer(MCP40D18, RESISTANCE_10KΩ);  // RESISTANCE_10KΩ = 10KΩ

// RGB (Red Green Blue) LED Data Structure
typedef struct RGB_LABEL
{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} RGB;

// Instantiate Objects for the different colours we use
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

#if defined(KOLIBRI_HARDWARE_REVISION_13)
#include <ESP_I2S.h>

const int frequency = 440;    // frequency of square wave in Hz
const int amplitude = 2000;   // amplitude of square wave
const int sampleRate = 8000;  // sample rate in Hz

i2s_data_bit_width_t bps = I2S_DATA_BIT_WIDTH_16BIT;
i2s_mode_t mode = I2S_MODE_STD;
i2s_slot_mode_t slot = I2S_SLOT_MODE_STEREO;

const uint32_t halfWavelength = sampleRate / frequency / 2;   // half wavelength of square wave
int32_t sample = amplitude;                                   // current sample value
uint32_t count = 0;

I2SClass i2s;
#endif

int brightness = 0;  // how bright the LED is
int fadeAmount = 5;  // how many points to fade the LED by

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

inline void turn_on_boost_converter(void) {
  power.otg(true);
  #if defined (KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(GPIO_OUTPUT_POWER_ENABLE, HIGH);
  #endif
}

inline void turn_off_boost_converter(void) {
  power.otg(false);
  #if defined (KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(GPIO_OUTPUT_POWER_ENABLE, LOW);
  #endif
}

inline void nebulization_on(void) {
#if defined(KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(GPIO_OUTPUT_POWER_ENABLE, HIGH);
#endif
#if defined(KOLIBRI_HARDWARE_REVISION_154)
  digitalWrite(GPIO_OUTPUT_PREBOOST, LOW);
#endif
  ledcWriteChannel(PIEZO_PWM_CHANNEL, PIEZO_PWM_DUTY_CYCLE);
  Serial.println("Nebulization On");
}

inline void nebulization_off(void) {
#if defined(KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(GPIO_OUTPUT_POWER_ENABLE, LOW);
#endif
#if defined(KOLIBRI_HARDWARE_REVISION_154)
  digitalWrite(GPIO_OUTPUT_PREBOOST, HIGH);
#endif
  ledcWriteChannel(PIEZO_PWM_CHANNEL, NO_PWM_DUTY_CYCLE);
  Serial.println("Nebulization Off");
}

inline void change_colour(RGB& colour) {
  ledcWriteChannel(LED_R_PWM_CHANNEL, colour.red);
  ledcWriteChannel(LED_G_PWM_CHANNEL, colour.green);
  ledcWriteChannel(LED_B_PWM_CHANNEL, colour.blue);
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

inline void turn_on_buzzer(void) {
  #if defined(KOLIBRI_HARDWARE_REVISION_13)
  uint32_t timestamp = millis();
  while (millis() - timestamp < 1000) {
    if (count % halfWavelength == 0) {
      // invert the sample every half wavelength count multiple to generate square wave
      sample = -1 * sample;
    }

    // Left channel, the low 8 bits then high 8 bits
    i2s.write(sample);
    i2s.write(sample >> 8);

    // Right channel, the low 8 bits then high 8 bits
    i2s.write(sample);
    i2s.write(sample >> 8);

    // increment the counter for the next sample
    count++;
  }
  #else
  ledcWriteChannel(BUZZER_PWM_CHANNEL, BUZZER_PWM_DUTY_CYCLE);
  #endif
  Serial.println("Buzzer On");
}

inline void turn_off_buzzer(void) {
  #if defined(KOLIBRI_HARDWARE_REVISION_13)
  ;
  #else
  ledcWriteChannel(BUZZER_PWM_CHANNEL, NO_PWM_DUTY_CYCLE);
  #endif
  Serial.println("Buzzer Off");
}


void setup() {
  // Initialize the Universal Asynchronous Receive-Transmit (UART) Communication Interface:
  Serial.begin(DEBUG_BAUD_RATE);
  delay(DELAY_ON_BOOT);
  Serial.printf("KOLIBRI HARDWARE REVISION: %s\n", KOLIBRI_HARDWARE_REVISION);

  // Turn on the 3V3 Peripheral Subsystem. It is Active low (It is called 3V3Enable in the schematics)
  pinMode(GPIO_OUTPUT_V3V3P_ENABLE, OUTPUT);
  turn_on_3v3_switch();

  // Turn on the power to the boost subsystem. It is Active High (It is called PowerEnable in the schematics)
  #if defined(KOLIBRI_HARDWARE_REVISION_13)
  pinMode(GPIO_OUTPUT_POWER_ENABLE, OUTPUT);
  digitalWrite(GPIO_OUTPUT_POWER_ENABLE, HIGH);
  #elif defined(KOLIBRI_HARDWARE_REVISION_15)
  #if defined(KOLIBRI_HARDWARE_REVISION_154)
  pinMode(GPIO_OUTPUT_PREBOOST, OUTPUT);
  digitalWrite(GPIO_OUTPUT_PREBOOST, LOW);
  #endif
  #endif

  // Initialize the Input Button
  #if defined (KOLIBRI_HARDWARE_REVISION_13)
  pinMode(GPIO_INPUT_BUTTON, INPUT);
  #elif defined (KOLIBRI_HARDWARE_REVISION_15)
  pinMode(GPIO_INPUT_BUTTON, INPUT_PULLUP);
  #endif

  // Initialize the I2C interfaces
  Wire.begin(GPIO_I2C_SDA0, GPIO_I2C_SCL0, I2C_CLOCK_FREQUENCY);

  // Initialize the Potentiometer
  if (!potentiometer.begin(&Wire)) {
    Serial.println("Could not initialize the potentiometer");
    while (1)
      ;
  }
  potentiometer.limit(3500, 50, 10000);  // 3500Ω, 50Ω, 10000Ω
  Serial.printf("Default resistance: %dΩ \n", potentiometer.get());
  potentiometer.set(4000);  // 4000 Ω
  Serial.printf("New resistance: %dΩ \n", potentiometer.get());

  // Initialize the Power Supply (i.e. The switch-mode charger management IC)
  if (!power.begin(&Wire)) {
    Serial.println("Could not initialize the power");
    while (1)
      ;
  }

  // Set the power supply that the boost converter generates
  potentiometer.set(2500);
  turn_on_boost_converter();

  // Initialize the PWM controller for the Piezo
  ledcAttachChannel(GPIO_OUTPUT_PIEZO, PIEZO_PWM_BASE_FREQUENCY, PIEZO_PWM_TIMER_RESOLUTION, PIEZO_PWM_CHANNEL);

  // Initialize the PWM controller for all the LEDs
  ledcAttachChannel(GPIO_OUTPUT_LED_RED, LED_PWM_BASE_FREQUENCY, LED_PWM_TIMER_RESOLUTION, LED_R_PWM_CHANNEL);
  ledcAttachChannel(GPIO_OUTPUT_LED_GREEN, LED_PWM_BASE_FREQUENCY, LED_PWM_TIMER_RESOLUTION, LED_G_PWM_CHANNEL);
  ledcAttachChannel(GPIO_OUTPUT_LED_BLUE, LED_PWM_BASE_FREQUENCY, LED_PWM_TIMER_RESOLUTION, LED_B_PWM_CHANNEL);

  // Initialize the PWM controller for the Motor (Haptics/Vibration)
  ledcAttachChannel(GPIO_OUTPUT_MOTOR, MOTOR_PWM_BASE_FREQUENCY, MOTOR_PWM_TIMER_RESOLUTION, MOTOR_PWM_CHANNEL);

  // Initialize the PWM controller for the Buzzer
  #if defined (KOLIBRI_HARDWARE_REVISION_13)
  // Initialize the I2S Interface:
  i2s.setPins(GPIO_I2S_BCLK, GPIO_I2S_LRCLK, GPIO_I2S_DIN);

  // start I2S at the sample rate with 16-bits per sample
  if (!i2s.begin(mode, sampleRate, bps, slot)) {
    Serial.println("Failed to initialize I2S!");
    while (1)
      ;  // do nothing
  }
  #elif defined (KOLIBRI_HARDWARE_REVISION_15)
  ledcAttachChannel(GPIO_OUTPUT_BUZZER, BUZZER_PWM_BASE_FREQUENCY, BUZZER_PWM_TIMER_RESOLUTION, BUZZER_PWM_CHANNEL);
  #endif

  change_colour(green);
  
}

void loop() {
  // Nebulize for 3s whenever the button is pressed
  if (button_pressed()) {
    nebulization_on();
    delay(2000);
    nebulization_off();

    // Vibrate for 6s (3s weak vibration, 3s strong vibration) whenever the button is pressed
    weak_vibration();
    delay(2000);
    stop_vibration();
    delay(2000);
    strong_vibration();
    delay(2000);
    stop_vibration();
    delay(2000);
    empty_vibration();
    delay(2000);
    stop_vibration();
    delay(2000);

    // Change LED colours whenever the button is pressed
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

    // Turn the Buzzer on at 1s interval
    turn_on_buzzer();
    delay(1000);
    turn_off_buzzer();
    delay(1000);
  }

/*
  // Nebulize for 6s whenever the button is pressed
  if (button_pressed()) {
    nebulization_on();
    strong_vibration();
    change_colour(green);
    delay(6000);
    nebulization_off();
    stop_vibration();
    change_colour(black);
  }
*/

  /*
  // Set the brightness on LEDC channel 0
  ledcWriteChannel(LED_PWM_CHANNEL, brightness);

  // Change the brightness for next time through the loop:
  brightness = brightness + fadeAmount;

  // Reverse the direction of the fading at the ends of the fade:
  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = -fadeAmount;
  }

  // Wait for 30 milliseconds to see the dimming effect
  delay(30);
  */
}
