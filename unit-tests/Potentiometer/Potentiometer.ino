// Note: The potentiometer accuracy is limited to ±1% of the maximum resistance value

#include <Wire.h>
#include <MCP40DXX.h>
#include "../../pins.h"

#define I2C_FREQUENCY         400000

using namespace MCP40DXX_NS;
MCP40DXX potentiometer(MCP40D18, RESISTANCE_10KΩ); // MCP40D18 = 0x2E (i.e. The I2C address of MCP40D18) and RESISTANCE_10KΩ = 10KΩ

inline void turn_on_3v3_switch(void) {
  #if defined (KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, LOW);
  #elif defined (KOLIBRI_HARDWARE_REVISION_15)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, HIGH);
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
  turn_on_3v3_switch();

  Wire.begin(GPIO_I2C_SDA0, GPIO_I2C_SCL0, I2C_FREQUENCY);
  if (!potentiometer.begin(&Wire)) {
    Serial.println("Could not initialize");
    while (1) delay(10);
  }

  potentiometer.reset();
  potentiometer.limit(1000, 50, 10000);  // 1000Ω, 50Ω, 10000Ω
  Serial.println("[Potentiometer] Initialization Successful!");
  Serial.printf("Potentiometer Resolution in Ω: %d\n", potentiometer.resolution());
  Serial.printf("Default Resistance in Ω: %d\n", potentiometer.get());
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  while (Serial.available() > 0) {
    potentiometer.set(atoi(Serial.readStringUntil('\r').c_str()));
    Serial.printf("New Resistance in Ω: %d\n", potentiometer.get());      // Expect: Any value in Ω that was just written via the Serial Monitor
    while (Serial.available() > 0) Serial.read();
  }
  delay(10);
}
