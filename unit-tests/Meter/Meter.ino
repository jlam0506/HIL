// An example showing the basic (Voltage, Current and Power measurement) functions of the INA2XX chip in Kolibri
#include <Wire.h>
#include "INA2XX.h"

// I2C Bus Details
#define I2C_ADDRESS                     0x40
#define I2C_FREQUENCY                   400000
#define EXAMPLE_I2C_SDA_PIN             38
#define EXAMPLE_I2C_SCL_PIN             39

// Parameters specific to INA260
#define SHUNT_RESISTANCE                0.002
#define CURRENT_MEASUREMENT_RESOLUTION  0.00125
#define MAXIMUM_MEASURABLE_CURRENT      15.0
#define VOLTAGE_MEASUREMENT_RESOLUTION  0.00125
#define MAXIMUM_MEASURABLE_VOLTAGE      36.0
#define POWER_MEASUREMENT_RESOLUTION    0.01

using namespace INA2XX_NS;

INA2XX meter(INA260, I2C_ADDRESS, SHUNT_RESISTANCE, CURRENT_MEASUREMENT_RESOLUTION, MAXIMUM_MEASURABLE_CURRENT, VOLTAGE_MEASUREMENT_RESOLUTION, MAXIMUM_MEASURABLE_VOLTAGE, POWER_MEASUREMENT_RESOLUTION);

constexpr int8_t GPIO_OUTPUT_V3V3P_ENABLE = GPIO_NUM_7;
constexpr int8_t GPIO_INPUT_METER_IRQ = GPIO_NUM_6;
constexpr bool SUCCESSFUL_MEASUREMENT = true;

void setup() {
  // Initialize the UART Serial Interface
  Serial.begin(115200);
  delay(2000);

  // Power on the Peripheral Circuits
  pinMode(GPIO_OUTPUT_V3V3P_ENABLE, OUTPUT);
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, HIGH);

  // Initialize the I2C Interface
  #if defined(ESP32)
  Wire.begin(EXAMPLE_I2C_SDA_PIN, EXAMPLE_I2C_SCL_PIN, I2C_FREQUENCY);
  #else
  Wire.begin();
  #endif

  // Initialize the Metering ASIC
  if (!meter.begin(&Wire)) {
    Serial.println("Could not initialize the metering chip");
    while (1);
  }
}


void loop() {
  if (meter.ready() == SUCCESSFUL_MEASUREMENT && !meter.error()) {
    Serial.printf("\nShunt Voltage: %.4fV\tVoltage: %.4fV\tCurrent: %.4fA\tPower: %.4fW", meter.shunt(), meter.voltage(), meter.current(), meter.power());
  }
  delay(1000);
}
