#include <Wire.h>
#include <MP2731.h>
#include <MCP40DXX.h>
#include "../../pins.h"

// I2C Addresses and Clock Frequency Definitions
#define MP2731_I2C_ADDRESS 0x4B
#define MCP40D18_I2C_ADDRESS 0x2E
#define I2C_CLOCK_FREQUENCY 400000

using namespace MP2731_NS;
using namespace MCP40DXX_NS;

MP2731 power(MP2731_I2C_ADDRESS);
MCP40DXX potentiometer(MCP40D18_I2C_ADDRESS, RESISTANCE_10KΩ);  // RESISTANCE_10KΩ = 10KΩ


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


inline void power_boost_active(bool enable) {
  if (enable) {
    potentiometer.write(20);  // 20 in Decimal as in 0x14 in Hex. TODO: Later, put this in its resistance form as in potentiometer.set(Ω);
    turn_on_boost_converter();
  } else {
    potentiometer.write(20);
    turn_off_boost_converter();
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

  // Turn on the power to the boost subsystem. It is Active High (It is called PowerEnable in the schematics)
  #if defined (KOLIBRI_HARDWARE_REVISION_13)
  pinMode(GPIO_OUTPUT_POWER_ENABLE, OUTPUT);
  digitalWrite(GPIO_OUTPUT_POWER_ENABLE, HIGH);
  #elif defined (KOLIBRI_HARDWARE_REVISION_15)
  ;
  #endif

  // Initialize the Input Button
  pinMode(GPIO_INPUT_BUTTON, INPUT_PULLUP);

  // Initialize the I2C interfaces
  Wire.begin(GPIO_I2C_SDA0, GPIO_I2C_SCL0, I2C_CLOCK_FREQUENCY);

  // Initialize the Potentiometer
  if (!potentiometer.begin(&Wire)) {
    Serial.println("Could not initialize the potentiometer");
    while (1)
      ;
  }

  // Initialize the Power Supply (i.e. The switch-mode charger management IC)
  if (!power.begin(&Wire)) {
    Serial.println("Could not initialize the power");
    while (1)
      ;
  }

  // Set the power supply that the boost converter generates
  power_boost_active(true);

  // Set the resolution to 12 bits (0-4095)
  analogReadResolution(12);
}

void loop() {
  // Read the analog / millivolts value for ADC pins:
 
#if defined(KOLIBRI_HARDWARE_REVISION_13)
  Serial.printf("ADC analog value of GPIO_ADC_PHASE_SHIFT = \t\t%d\n", (int) analogRead(GPIO_ADC_PHASE_SHIFT));
  Serial.printf("ADC millivolts value of GPIO_ADC_PHASE_SHIFT = \t\t%d\n\n", (int) analogReadMilliVolts(GPIO_ADC_PHASE_SHIFT));
#endif
#if defined(KOLIBRI_HARDWARE_REVISION_151) || defined(KOLIBRI_HARDWARE_REVISION_152)
  Serial.printf("ADC analog value of GPIO_ADC_BOOST_VFB = \t\t%d\n", (int) analogRead(GPIO_ADC_BOOST_VFB));
  Serial.printf("ADC millivolts value of GPIO_ADC_BOOST_VFB = \t\t%d\n\n", (int) analogReadMilliVolts(GPIO_ADC_BOOST_VFB));
 
  Serial.printf("ADC analog value of GPIO_ADC_BOOST_IFB = \t\t%d\n", (int) analogRead(GPIO_ADC_BOOST_IFB));
  Serial.printf("ADC millivolts value of GPIO_ADC_BOOST_IFB = \t\t%d\n\n", (int) analogReadMilliVolts(GPIO_ADC_BOOST_IFB));
#endif
 
#if defined(KOLIBRI_HARDWARE_REVISION_153)
Serial.print("Test not supported\n");
#endif
 
  delay(500);  // delay in between reads for clear read from serial
}
