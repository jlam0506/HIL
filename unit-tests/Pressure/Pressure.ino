#define KOLIBRI_HARDWARE_REVISION_13
#include <Wire.h>
#include "SDP3X.h"
#include "../../pins.h"

// Uncomment the line below to enable pressure mapping to flow rate using simflow
// #define PRESSURE_MAPPING

#define I2C_ADDRESS   0x21
#define I2C_FREQUENCY 400000

using namespace SDP3X_NS;
SDP3X SDP31(I2C_ADDRESS);
constexpr bool SUCCESSFUL_MEASUREMENT = true;

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

void setup() {
  // put your setup code here, to run once:
  // Initialize the Universal Asynchronous Receive-Transmit (UART) Communication Interface:
  Serial.begin(DEBUG_BAUD_RATE);
  delay(DELAY_ON_BOOT);
  Serial.printf("KOLIBRI HARDWARE REVISION: %s\n", KOLIBRI_HARDWARE_REVISION);

  // Enable Power Supply to Subsystems
  pinMode(GPIO_OUTPUT_V3V3P_ENABLE, OUTPUT);
  turn_off_3v3_switch();
  delay(1000);
  turn_on_3v3_switch();

  Wire.begin(GPIO_I2C_SDA1, GPIO_I2C_SCL1, I2C_FREQUENCY);
  if (!SDP31.begin(&Wire)) {
    Serial.println("Could not initialize");
    while (1) ;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  #if defined (PRESSURE_MAPPING)
  if (SDP31.measure() == SUCCESSFUL_MEASUREMENT) Serial.println(SDP31.pressure());
  #else
  if (SDP31.measure() == SUCCESSFUL_MEASUREMENT) {
    Serial.printf("Pressure (Pa): %f \nTemperature (°C): %f\n\n", SDP31.pressure(), SDP31.temperature());
  }
  delay(1000);
  #endif
}
