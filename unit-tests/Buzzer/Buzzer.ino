// KB R1.3 uses Power Amplifier that is controlled by I2S. This requires an external libraries
// KB R1.5 uses GPIO and DAC to achieve the same functionally as KB R1.3

#include "../../pins.h"

inline bool button_pressed(void) {
  return digitalRead(GPIO_INPUT_BUTTON) == LOW;
}

inline void turn_on_3v3_switch(void) {
#if defined(KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, LOW);
#elif defined(KOLIBRI_HARDWARE_REVISION_15)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, HIGH);
#endif
}

#if defined(KOLIBRI_HARDWARE_REVISION_13)
#include <ESP_I2S.h> // serial communication protocol usually used for transmitting audio

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

void setup() {
  // Initialize the Universal Asynchronous Receive-Transmit (UART) Communication Interface:
  Serial.begin(DEBUG_BAUD_RATE);
  delay(DELAY_ON_BOOT);
  Serial.printf("KOLIBRI HARDWARE REVISION: %s\n", KOLIBRI_HARDWARE_REVISION);

  // Enable Power Supply to Subsystems
  pinMode(GPIO_OUTPUT_V3V3P_ENABLE, OUTPUT);
  turn_on_3v3_switch();

  // Initialize the pushbutton pin as an input:
  pinMode(GPIO_INPUT_BUTTON, INPUT);

  // Initialize the I2S Interface:
  i2s.setPins(GPIO_I2S_BCLK, GPIO_I2S_LRCLK, GPIO_I2S_DIN);

  // start I2S at the sample rate with 16-bits per sample
  if (!i2s.begin(mode, sampleRate, bps, slot)) {
    Serial.println("Failed to initialize I2S!");
    while (1)
      ;  // do nothing
  }
}

void loop() {
  if (button_pressed()) {
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
    delay(1000);
  }
}

#elif defined(KOLIBRI_HARDWARE_REVISION_15)

// Buzzer Pulse Width Modulation (PWM) Analog Output Definitions
#define BUZZER_PWM_TIMER_RESOLUTION 8   // 8 bit (ie. 0 to 255)
#define BUZZER_PWM_BASE_FREQUENCY 3100  // In Hz
#define BUZZER_PWM_CHANNEL 7
#define BUZZER_PWM_DUTY_CYCLE 128
#define NO_PWM_DUTY_CYCLE 0

// Inline definintion of activation or deactivation of peripherals
inline void turn_on_buzzer(void) {
  ledcWriteChannel(BUZZER_PWM_CHANNEL, BUZZER_PWM_DUTY_CYCLE);
  Serial.println("Buzzer On");
}

inline void turn_off_buzzer(void) {
  ledcWriteChannel(BUZZER_PWM_CHANNEL, NO_PWM_DUTY_CYCLE);
  Serial.println("Buzzer Off");
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

  // Initialize the Input Button
  pinMode(GPIO_INPUT_BUTTON, INPUT_PULLUP);

  // Initialize the PWM controller for the Buzzer
  ledcAttachChannel(GPIO_OUTPUT_BUZZER, BUZZER_PWM_BASE_FREQUENCY, BUZZER_PWM_TIMER_RESOLUTION, BUZZER_PWM_CHANNEL);
}

// the loop function runs over and over again forever
void loop() {
  if (button_pressed()) {
    turn_on_buzzer();   // Turn the Buzzer on (HIGH is the voltage level)
    delay(1000);        // Wait for a second
    turn_off_buzzer();  // Turn the Buzzer off by making the voltage LOW
    delay(1000);        // Wait for a second
  }
}

#endif