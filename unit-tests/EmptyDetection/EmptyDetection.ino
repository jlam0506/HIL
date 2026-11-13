/*
  Description: Until empty detection, nebulize for 3s each for positive and negative feedback whenever a button is pressed
  Date: April, 2026
*/
// #define KOLIBRI_HARDWARE_REVISION_13
#define KOLIBRI_HARDWARE_REVISION_154
#include <Wire.h>
#include <MP2731.h>
#include <MCP40DXX.h>
#include "driver/rtc_io.h"
#include "../../pins.h"
#if defined(KOLIBRI_HARDWARE_REVISION_153) || defined(KOLIBRI_HARDWARE_REVISION_154)
#include "INA2XX.h"
#define METER_I2C_ADDRESS 0x40
#endif

// The Nebulization Parameters
#define SMALL_AG
#define DEFAULT_POTENTIOMETER_RESISTANCE 2500         // In Ohms
#define MINIMUM_NEBULIZATION_FREQUENCY 130000         // In Hertz
#define MAXIMUM_NEBULIZATION_FREQUENCY 150000         // In Hertz
#define MAXIMUM_TIME_OF_CONSTANT_NEBULIZATION 300000  // In milliseconds
#define MINIMUM_NEBULIZATION_POWER_SMALL_AG 1.75      // In Watt (M06252)
#define MAXIMUM_NEBULIZATION_POWER_SMALL_AG 1.65      // In Watt
#define MINIMUM_NEBULIZATION_POWER_LARGE_AG 1.5       // In Watt (M01252)
#define MAXIMUM_NEBULIZATION_POWER_LARGE_AG 1.8       // In Watt
#define POWER_COEFFICIENT 0.72                        // % Apparent Power of BoostPower (1 = 100%) | 0.75@1nF or 0.71@1.5nF for C500+C501
#define EMPTY_TRIGGER_COUNT 3                         // Number of times empty detection needs to be triggered within 10s till active
#if defined(SMALL_AG)
#define MINIMUM_NEBULIZATION_POWER MINIMUM_NEBULIZATION_POWER_SMALL_AG
#define MAXIMUM_NEBULIZATION_POWER MAXIMUM_NEBULIZATION_POWER_SMALL_AG
#else
#define MINIMUM_NEBULIZATION_POWER MINIMUM_NEBULIZATION_POWER_LARGE_AG
#define MAXIMUM_NEBULIZATION_POWER MAXIMUM_NEBULIZATION_POWER_LARGE_AG
#endif

// Parameters specific to Metering ASIC
#define SHUNT_RESISTANCE 0.002
#define CURRENT_MEASUREMENT_RESOLUTION 0.00125
#define MAXIMUM_MEASURABLE_CURRENT 15.0
#define VOLTAGE_MEASUREMENT_RESOLUTION 0.00125
#define MAXIMUM_MEASURABLE_VOLTAGE 36.0
#define POWER_MEASUREMENT_RESOLUTION 0.01

// LED Pulse Width Modulation (PWM) Analog Output Definitions
#define LED_PWM_TIMER_RESOLUTION 8   // 8 bit (ie. 0 to 255)
#define LED_PWM_BASE_FREQUENCY 5000  // In Hz
#define LED_R_PWM_CHANNEL 3
#define LED_G_PWM_CHANNEL 4
#define LED_B_PWM_CHANNEL 5
#define LED_PWM_CHANNEL LED_G_PWM_CHANNEL  // Just for completeness
#define LED_PWM_DUTY_CYCLE 128
#define NO_PWM_DUTY_CYCLE 0

// Motor Pulse Width Modulation (PWM) Analog Output Definitions
#define MOTOR_PWM_TIMER_RESOLUTION 8   // 8 bit (ie. 0 to 255)
#define MOTOR_PWM_BASE_FREQUENCY 5000  // In Hz
#define MOTOR_PWM_CHANNEL 6
#define MOTOR_PWM_DUTY_CYCLE 60
#define NO_PWM_DUTY_CYCLE 0

// Empty, Weak, Strong and No Vibration Definitions
// #define VIBRATION_A                 (MOTOR_PWM_DUTY_CYCLE)
// #define VIBRATION_B                 (MOTOR_PWM_DUTY_CYCLE + 40)
#define VIBRATION_A 110
#define VIBRATION_B 200
#define EMPTY_VIBRATION VIBRATION_B
#define NO_VIBRATION NO_PWM_DUTY_CYCLE
#define WEAK_VIBRATION VIBRATION_A
#define STRONG_VIBRATION VIBRATION_B

// I2C Addresses and Clock Frequency Definitions
#define MP2731_I2C_ADDRESS 0x4B
#define MCP40D18_I2C_ADDRESS 0x2E
#define I2C_CLOCK_FREQUENCY 400000

// Piezo Pulse Width Modulation (PWM) Analog Output Definitions
#define PIEZO_PWM_TIMER_RESOLUTION 4     // 4 bit
#define PIEZO_PWM_BASE_FREQUENCY 140000  // In Hz
#define PIEZO_PWM_CHANNEL 0
#define PIEZO_PWM_DUTY_CYCLE 8
#define NO_PWM_DUTY_CYCLE 0

// Maximum Idle or Inactivity Time Definition
#define INACTIVITY_DURATION_IN_MS 300000  // 60000 for 60 seconds, 300000 for 5 minutes

// Track sleep time
uint32_t inactivity_timestamp;

using namespace MP2731_NS;
using namespace MCP40DXX_NS;

MP2731 power(MP2731_I2C_ADDRESS);
MCP40DXX potentiometer(MCP40D18, RESISTANCE_10KΩ);  // RESISTANCE_10KΩ = 10KΩ
#if defined(KOLIBRI_HARDWARE_REVISION_153) || defined(KOLIBRI_HARDWARE_REVISION_154)
INA2XX_NS::INA2XX Meter(INA2XX_NS::INA260, METER_I2C_ADDRESS, SHUNT_RESISTANCE, CURRENT_MEASUREMENT_RESOLUTION, MAXIMUM_MEASURABLE_CURRENT, VOLTAGE_MEASUREMENT_RESOLUTION, MAXIMUM_MEASURABLE_VOLTAGE, POWER_MEASUREMENT_RESOLUTION);
#endif

// Inline definintion of activation or deactivation of peripherals
inline bool button_pressed(void) {
  return digitalRead(GPIO_INPUT_BUTTON) == LOW;
}

// Inline definintion of activation or deactivation of peripherals
inline void button_release(void) {
  while (digitalRead(GPIO_INPUT_BUTTON) == LOW)
    ;
}

inline void turn_on_3v3_switch(void) {
#if defined(KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, LOW);
#elif defined(KOLIBRI_HARDWARE_REVISION_15)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, HIGH);
#endif
}

inline void turn_off_3v3_switch(void) {
#if defined(KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, HIGH);
#elif defined(KOLIBRI_HARDWARE_REVISION_15)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, LOW);
#endif
}

inline void turn_on_boost_converter(void) {
  power.otg(true);
#if defined(KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(GPIO_OUTPUT_POWER_ENABLE, HIGH);
#endif
}

inline void turn_off_boost_converter(void) {
  power.otg(false);
#if defined(KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(GPIO_OUTPUT_POWER_ENABLE, LOW);
#endif
}

inline void turn_on_led(int pin) {
  pinMode(pin, OUTPUT);
#if defined(KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(pin, HIGH);
#elif defined(KOLIBRI_HARDWARE_REVISION_15)
  digitalWrite(pin, LOW);
#endif
}

inline void turn_off_led(int pin) {
  pinMode(pin, OUTPUT);
#if defined(KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(pin, LOW);
#elif defined(KOLIBRI_HARDWARE_REVISION_15)
  digitalWrite(pin, HIGH);
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

inline void weak_vibration(void) {
  Serial.println("Vibration On: Weak");
  ledcWriteChannel(MOTOR_PWM_CHANNEL, WEAK_VIBRATION);
}

inline void strong_vibration(void) {
  Serial.println("Vibration On: Strong");
  ledcWriteChannel(MOTOR_PWM_CHANNEL, STRONG_VIBRATION);
}

inline void empty_vibration(void) {
  Serial.println("Vibration On: Empty");
  ledcWriteChannel(MOTOR_PWM_CHANNEL, EMPTY_VIBRATION);
  delay(250);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, NO_VIBRATION);
  delay(250);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, EMPTY_VIBRATION);
  delay(250);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, NO_VIBRATION);
  delay(250);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, EMPTY_VIBRATION);
  delay(250);
  ledcWriteChannel(MOTOR_PWM_CHANNEL, NO_VIBRATION);
  delay(250);
  // ledcWriteChannel(MOTOR_PWM_CHANNEL, EMPTY_VIBRATION);
  // delay(250);
  // ledcWriteChannel(MOTOR_PWM_CHANNEL, NO_VIBRATION);
  // delay(250);
}

inline void stop_vibration(void) {
  Serial.println("Vibration Off");
  ledcWriteChannel(MOTOR_PWM_CHANNEL, NO_VIBRATION);
}

void boot_completion_indication(void) {
  int16_t brightness = 0;  // how bright the LED is
  int16_t fadeAmount = 5;  // how many points to fade the LED by
  for (int16_t i = 0; i < 256; i++) {
    // Set the brightness on LEDC channel 0
    ledcWriteChannel(LED_G_PWM_CHANNEL, brightness);

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


bool empty(void) {
  constexpr uint32_t NUMBER_OF_VOLTAGE_AND_CURRENT_SAMPLES_MEASURED = 5;
  constexpr bool EMPTY = true;
  constexpr bool NOT_EMPTY = false;
  static bool status = NOT_EMPTY;
  static uint32_t empty_detection_triggered_count_timeout = millis();

  if (status == EMPTY) return status;

  // Measure the voltage, current and phase shift all in mV. epsilon (ε) is an offset added to avoid divided by zero which is an invalid operation
  float epsilon = 1.0, feedback_voltage = epsilon, feedback_current = epsilon, feedback_phase_shift = epsilon;
  float boost_voltage, boost_current, boost_power, average_power, impedance;

  for (uint8_t count = 0; count < NUMBER_OF_VOLTAGE_AND_CURRENT_SAMPLES_MEASURED; count++) {
#if defined(KOLIBRI_HARDWARE_REVISION_13) || defined(KOLIBRI_HARDWARE_REVISION_151) || defined(KOLIBRI_HARDWARE_REVISION_152)
    feedback_voltage += analogReadMilliVolts(GPIO_ADC_BOOST_VFB);  // In mV
    feedback_current += analogReadMilliVolts(GPIO_ADC_BOOST_IFB);  // In mV
#elif defined(KOLIBRI_HARDWARE_REVISION_153) || defined(KOLIBRI_HARDWARE_REVISION_154)
    if (Meter.ready() && !Meter.error()) {
      boost_voltage += Meter.voltage();
      boost_current += Meter.current();
      boost_power += Meter.power();
      //Serial.printf("\nShunt Voltage: %.4fV\tVoltage: %.4fV\tCurrent: %.4fA\tPower: %.4fW", Meter.shunt(), Meter.voltage(), Meter.current(), Meter.power());
    }
#endif
#if defined(KOLIBRI_HARDWARE_REVISION_13)
    feedback_phase_shift += analogReadMilliVolts(GPIO_ADC_PHASE_SHIFT);  // In mV
#endif
    //delay(SAMPLE_RATE_IN_MILLISECONDS/NUMBER_OF_VOLTAGE_AND_CURRENT_SAMPLES_MEASURED);
  }

#if defined(KOLIBRI_HARDWARE_REVISION_13) || defined(KOLIBRI_HARDWARE_REVISION_151) || defined(KOLIBRI_HARDWARE_REVISION_152)
  feedback_voltage /= NUMBER_OF_VOLTAGE_AND_CURRENT_SAMPLES_MEASURED;
  feedback_current /= NUMBER_OF_VOLTAGE_AND_CURRENT_SAMPLES_MEASURED;
  feedback_phase_shift /= NUMBER_OF_VOLTAGE_AND_CURRENT_SAMPLES_MEASURED;
//Serial.printf(" Feedback Voltage: %.2fmV \n Feedback Current: %.2fmA/V \n Feedback Phase: %.2f° \n",  feedback_voltage, feedback_current / 10.0, feedback_phase_shift);
#elif defined(KOLIBRI_HARDWARE_REVISION_153) || defined(KOLIBRI_HARDWARE_REVISION_154)
  boost_voltage /= NUMBER_OF_VOLTAGE_AND_CURRENT_SAMPLES_MEASURED;
  boost_current /= NUMBER_OF_VOLTAGE_AND_CURRENT_SAMPLES_MEASURED;
  boost_power /= NUMBER_OF_VOLTAGE_AND_CURRENT_SAMPLES_MEASURED;
  boost_current = boost_current > 0.2 ? 0.12 : boost_current;
#endif

// Calculate the Real Power (W). Note that the algorithm is meant to maintain the power between 1650mW to 1750mW
#if defined(KOLIBRI_HARDWARE_REVISION_13)
  boost_voltage = (feedback_voltage / 1000.0) * (120.0 / 10.0);  // "/1000" converts mV to V. "120/10" is the inverted Voltage Divider Ratio. Please see the schematics for more details
#elif defined(KOLIBRI_HARDWARE_REVISION_151) || defined(KOLIBRI_HARDWARE_REVISION_152)
  boost_voltage = (feedback_voltage / 1000.0) * (259.0 / 10.0);  // "/1000" converts mV to V. "259/10" is the inverted Voltage Divider Ratio. Please see the schematics for more details
#endif

#if defined(KOLIBRI_HARDWARE_REVISION_13) || defined(KOLIBRI_HARDWARE_REVISION_151) || defined(KOLIBRI_HARDWARE_REVISION_152)
  boost_current = (feedback_current / 1000.0) / 10.0;  // "/1000" converts mV to A. "/10" converts V to A. Equation: I = V / ( R X Gain). R = 100mΩ, Gain of INA293B3 = 100V/V, Vmv = V/1000
  boost_power = boost_voltage * boost_current;         // In Watts (W)
#endif
  average_power = boost_power * POWER_COEFFICIENT;  // "* 0.72" Probably the conversion from Apparent Power (VA) to Real Power (W)
  impedance = boost_voltage / boost_current;        // In Ohms (Ω)
  //Serial.printf(" Boost Voltage: %.2fV \t Boost Current: %.2fA \t Phase: %.2f° \t Boost Power: %.2fW \t Average Power: %.2fW \t Impedance: %.2fΩ \n",  boost_voltage, boost_current, feedback_phase_shift, boost_power, average_power, impedance);

  // Check the output power, then increase or decrease it to maintain nebuilizatlion quality by varying the feedback resistance to the boost converter
  if (average_power < (MINIMUM_NEBULIZATION_POWER - 0.04)) {
    if (average_power < (MINIMUM_NEBULIZATION_POWER - 0.15)) {
      potentiometer.decrease(300);
    } else {
      potentiometer.decrease(150);
    }
    Serial.printf("[decrease] New resistance: %dΩ \n", potentiometer.get());
  } else if (average_power > (MAXIMUM_NEBULIZATION_POWER + 0.04)) {
    if (average_power > (MAXIMUM_NEBULIZATION_POWER + 0.15)) {
      potentiometer.increase(300);
    } else {
      potentiometer.increase(150);
    }
    Serial.printf("[increase] New resistance: %dΩ \n", potentiometer.get());
  }

#if defined(KOLIBRI_EMPTY_DETECTION_ENABLED)
// Empty detection of liquid
#if defined(KOLIBRI_EMPTY_DETECTION_WITH_IMPEDANCE_LOWER_BOUND_AND_UPPER_BOUND)
  if (impedance <= EMPTY_DETECTION_IMPEDANCE_IN_OHMS_LOWER_BOUND || impedance >= EMPTY_DETECTION_IMPEDANCE_IN_OHMS_UPPER_BOUND) {
#else
  if (impedance >= EMPTY_DETECTION_IMPEDANCE_IN_OHMS) {
#endif
    static uint8_t empty_detection_triggered_count = 0;
    empty_detection_triggered_count++;
    if (empty_detection_triggered_count == 1) {
      empty_detection_triggered_count_timeout = millis();
    }
    Serial.printf(" Boost Voltage: %.2fV \t Boost Current: %.2fA \t Phase: %.2f° \t Boost Power: %.2fW \t Average Power: %.2fW \t Impedance: %.2fΩ \n", boost_voltage, boost_current, feedback_phase_shift, boost_power, average_power, impedance);
    Serial.printf("[EMPTY_DETECTION_IMPEDANCE_IN_OHMS] Kolibri suspects empty HAG: %d\n", empty_detection_triggered_count);
    if ((empty_detection_triggered_count >= 3) && ((millis() - empty_detection_triggered_count_timeout) < 200)) {
      empty_detection_triggered_count = 0;
      //if (hag_removed()) {
      if (average_power < 0.2 && boost_voltage > 5) {
        Serial.printf("[HAG DISCONNECTED] HAG is disconnected with impedance (Ω): %.2f \n", impedance);
        status = EMPTY;
      } else {
        Serial.printf("[EMPTY DETECTION] Kolibri is empty with impedance (Ω): %.2f \n", impedance);
        status = EMPTY;
      }
    }
    if ((empty_detection_triggered_count > 1) && ((millis() - empty_detection_triggered_count_timeout) > 200)) {
      empty_detection_triggered_count = 0;
    }
  }
#elif defined(KOLIBRI_SIMULATE_EMPTY_DETECTION_ENABLED)
  if (metadata.breath_count >= KOLIBRI_SIMULATE_EMPTY_DETECTION_ENABLED) {
    Serial.printf("[SIMULATED EMPTY DETECTION] Kolibri is empty with impedance (Ω): %d \n", KOLIBRI_SIMULATE_EMPTY_DETECTION_ENABLED * 1000);
    status = EMPTY;
  }
#endif
  return status;
}


void sleep(void) {
  nebulization_off();

  // Indicate that the device is going to sleep
  turn_off_led(GPIO_OUTPUT_LED_GREEN);
  turn_off_led(GPIO_OUTPUT_LED_RED);
  turn_on_led(GPIO_OUTPUT_LED_BLUE);
  delay(2000);
  turn_off_led(GPIO_OUTPUT_LED_BLUE);

  // Ensure that the button pressed to turn the device off is released
  button_release();

  // High = 1 = Wake-up on High Signal, Low = 0 = Wake-up on Low Signal. Remember to set and reset pull-ups and pull-downs accordingly
  esp_sleep_enable_ext0_wakeup(GPIO_INPUT_WAKEUP_IRQ, LOW);
  rtc_gpio_pullup_en(GPIO_INPUT_WAKEUP_IRQ);
  rtc_gpio_pulldown_dis(GPIO_INPUT_WAKEUP_IRQ);

  // TODO: Turn off or disable the RFID chip
  // TODO: Turn off or disable the Power Amplifier to the Buzzer

  //turn_off_nebulizer();
  //turn_off_vibration();
  //turn_off_red_led();
  //turn_off_green_led();
  //BLEDevice::deinit();

  turn_off_3v3_switch();
  turn_off_boost_converter();

  Serial.println("Going to sleep now in 3s");
  delay(50);
  esp_deep_sleep_start();
}


void setup() {
  // Initialize the Universal Asynchronous Receive-Transmit (UART) Communication Interface:
  Serial.begin(DEBUG_BAUD_RATE);
  delay(DELAY_ON_BOOT);
  Serial.printf("KOLIBRI HARDWARE REVISION: %s\n", KOLIBRI_HARDWARE_REVISION);
  Serial.printf("Default Nebulization Frequency: %dHz\n", PIEZO_PWM_BASE_FREQUENCY);
  Serial.println("To change the Nebulization Frequency, simply type the value in Hz into the terminal and press enter to change it");

  // Turn on the 3V3 Peripheral Subsystem. It is Active low (It is called 3V3Enable in the schematics)
  pinMode(GPIO_OUTPUT_V3V3P_ENABLE, OUTPUT);
  turn_on_3v3_switch();

  // Initialize the LED
  pinMode(GPIO_OUTPUT_LED_GREEN, OUTPUT);

  // Turn on the power to the boost subsystem. It is Active High (It is called PowerEnable in the schematics)
#if defined(KOLIBRI_HARDWARE_REVISION_13)
  pinMode(GPIO_OUTPUT_POWER_ENABLE, OUTPUT);
  digitalWrite(GPIO_OUTPUT_POWER_ENABLE, HIGH);
#elif defined(KOLIBRI_HARDWARE_REVISION_15)
#if defined(KOLIBRI_HARDWARE_REVISION_154)
  pinMode(GPIO_OUTPUT_PREBOOST, OUTPUT);
  digitalWrite(GPIO_OUTPUT_PREBOOST, HIGH);
#endif
#endif

// Initialize the pushbutton pin as an input:
#if defined(KOLIBRI_HARDWARE_REVISION_13)
  pinMode(GPIO_INPUT_BUTTON, INPUT);
#elif defined(KOLIBRI_HARDWARE_REVISION_15)
  pinMode(GPIO_INPUT_BUTTON, INPUT_PULLUP);
#endif

  // Initialize the I2C interfaces
  Wire.begin(GPIO_I2C_SDA0, GPIO_I2C_SCL0, I2C_CLOCK_FREQUENCY);

  // Initialize the potentiometer
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

#if defined(KOLIBRI_HARDWARE_REVISION_153) || defined(KOLIBRI_HARDWARE_REVISION_154)
  // Initialize the Metering Application Specific Integrated Circuit
  if (!Meter.begin(&Wire)) {
    Serial.println("[Metering Sensor] Initialization Failed!");
    while (1) 10;
  } else {
    Serial.println("[Metering Sensor] Initialization Successful!");
  }
#endif

  // Optional for ESP32: Set the resolution to 9-12 bits (default is 12 bits)
  analogReadResolution(12);

  // Initialize the PWM controller for the Piezo
  ledcAttachChannel(GPIO_OUTPUT_PIEZO, PIEZO_PWM_BASE_FREQUENCY, PIEZO_PWM_TIMER_RESOLUTION, PIEZO_PWM_CHANNEL);

  // Initialize the PWM controller for the Motor (Haptics/Vibration)
  ledcAttachChannel(GPIO_OUTPUT_MOTOR, MOTOR_PWM_BASE_FREQUENCY, MOTOR_PWM_TIMER_RESOLUTION, MOTOR_PWM_CHANNEL);

  // Initialize the PWM for the Red LED
  ledcAttachChannel(GPIO_OUTPUT_LED_RED, LED_PWM_BASE_FREQUENCY, LED_PWM_TIMER_RESOLUTION, LED_R_PWM_CHANNEL);

  // Initialize the PWM for the Green LED
  ledcAttachChannel(GPIO_OUTPUT_LED_GREEN, LED_PWM_BASE_FREQUENCY, LED_PWM_TIMER_RESOLUTION, LED_G_PWM_CHANNEL);

  // Initialize the PWM for the Blue LED
  ledcAttachChannel(GPIO_OUTPUT_LED_BLUE, LED_PWM_BASE_FREQUENCY, LED_PWM_TIMER_RESOLUTION, LED_B_PWM_CHANNEL);

  // Boot Delay: Essential for VOTG Stabilization
  boot_completion_indication();
  turn_on_led(GPIO_OUTPUT_LED_GREEN);
}

void loop() {
  if (button_pressed()) {
    for (uint16_t i = 0; i < 100; i++) {
      weak_vibration();
      nebulization_on();
      uint32_t timestamp = millis();
      while (millis() - timestamp < 3000) {
        if (empty()) {
          empty_vibration();
          sleep();
        }
      }
      strong_vibration();
      timestamp = millis();
      while (millis() - timestamp < 3000) {
        if (empty()) {
          empty_vibration();
          sleep();
        }
      }
      stop_vibration();
      nebulization_off();
      delay(4000);
    }
    inactivity_timestamp = millis();
  }

  // Go to sleep after "INACTIVITY_DURATION_IN_MS" seconds of inactivity
  if (millis() - inactivity_timestamp > INACTIVITY_DURATION_IN_MS) {
    sleep();
  }
}

void serialEventRun(void) {
  if (Serial.available() > 0) {
    uint32_t frequency = atoi(Serial.readStringUntil('\r').c_str());
    while (Serial.available() > 0) Serial.read();
    ledcChangeFrequency(GPIO_OUTPUT_PIEZO, frequency, PIEZO_PWM_TIMER_RESOLUTION);
    Serial.printf("Frequency has changed to: %luHz\n", frequency);
    inactivity_timestamp = millis();
  }
}