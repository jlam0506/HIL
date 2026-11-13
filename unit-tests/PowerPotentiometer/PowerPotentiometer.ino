// Note: The potentiometer accuracy is limited to ±1% of the maximum resistance value

#include <Wire.h>
#include <MP2731.h>
#include <MCP40DXX.h>
#include "../../pins.h"

#define MP2731_I2C_ADDRESS    0x4B
#define I2C_FREQUENCY         400000

using namespace MP2731_NS;
using namespace MCP40DXX_NS;

MP2731 power(MP2731_I2C_ADDRESS);
MCP40DXX potentiometer(MCP40D18, RESISTANCE_10KΩ);  // RESISTANCE_10KΩ = 10KΩ

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

  // Turn on the 3V3 Peripheral Subsystem. It is Active low/high in KB R1.3/1.2 (It is called 3V3Enable in the schematics)
  pinMode(GPIO_OUTPUT_V3V3P_ENABLE, OUTPUT);
  turn_on_3v3_switch();

  Wire.begin(GPIO_I2C_SDA0, GPIO_I2C_SCL0, I2C_FREQUENCY);

  if (!power.begin(&Wire)) {
    Serial.println("Could not initialize");
    while (1)
      ;
  }

  if (!potentiometer.begin(&Wire)) {
    Serial.println("Could not initialize");
    while (1)
      ;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  power.reset();
  Serial.print("REG 01H: 0x");
  Serial.println(power.read(INPUT_VOLTAGE_REGULATION_REGISTER), HEX);  // Expect: 0x06
  delay(1000);
  power.write(INPUT_VOLTAGE_REGULATION_REGISTER, 0x08);
  Serial.print("REG 01H: 0x");
  Serial.println(power.read(INPUT_VOLTAGE_REGULATION_REGISTER), HEX);  // Expect: 0x08
  delay(1000);

  potentiometer.reset();
  Serial.print("Default Resistance in Ω: "); Serial.println(potentiometer.get());  // Expect: RESISTANCE_10KΩ / 2 (i.e. The default register value after reset, where RESISTANCE_10KΩ = 10KΩ)
  delay(1000);
  potentiometer.set(3000); // 3000 Ω
  Serial.print("New Resistance in Ω: "); Serial.println(potentiometer.get());      // Expect: 3000 Ω (The value that was just written)
  delay(1000);
}
