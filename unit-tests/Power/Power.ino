#include <Wire.h>
#include <MP2731.h>
#include "../../pins.h"


#define I2C_ADDRESS   0x4B
#define I2C_FREQUENCY 400000

using namespace MP2731_NS;
MP2731 power(I2C_ADDRESS);

inline void turn_on_3v3_switch(void) {
  #if defined (KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, LOW);
  #elif defined (KOLIBRI_HARDWARE_REVISION_15)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, HIGH);
  #endif
}

uint8_t calculate_battery_percentage (const float voltage) {
  struct BatteryStep {
    float voltage;
    uint8_t percentage;
  };

  // Standard Li-ion discharge curve points
  const BatteryStep battery_table[] = {
    {4.20, 100}, {4.10, 90}, {3.97, 80}, {3.91, 70}, {3.86, 60},
    {3.82, 50},  {3.77, 40}, {3.73, 30}, {3.70, 20}, {3.60, 10},
    {3.30, 5},   {3.00, 0}
  };

  constexpr const uint8_t table_size = sizeof(battery_table) / sizeof(battery_table[0]);
  if (voltage >= battery_table[0].voltage) return 100;
  if (voltage <= battery_table[table_size - 1].voltage) return 0;

  for (uint8_t i = 0; i < table_size - 1; i++) {
    if (voltage <= battery_table[i].voltage && voltage >= battery_table[i+1].voltage) {
      // Linear Interpolation Equation
      float voltage_upper_bound = battery_table[i].voltage;
      float voltage_lower_bound = battery_table[i+1].voltage;
      uint8_t percentage_upper_bound = battery_table[i].percentage;
      uint8_t percentage_lower_bound = battery_table[i+1].percentage;
     
      return percentage_lower_bound + (voltage - voltage_lower_bound) * (percentage_upper_bound - percentage_lower_bound) / (voltage_upper_bound - voltage_lower_bound);
    }
  }
  return 0;
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
  if (!power.begin(&Wire)) {
    Serial.println("Could not initialize");
    while (1) ;
  }

  power.reset();
  power.write(ADC_CONTROL_OTG_CONFIGURATION_REGISTER, 0xDC);
  Serial.print("REG 01H: 0x"); Serial.println(power.read(INPUT_VOLTAGE_REGULATION_REGISTER), HEX);  // Expect: 0x06
  power.write(INPUT_VOLTAGE_REGULATION_REGISTER, 0x08);
  Serial.print("REG 01H: 0x"); Serial.println(power.read(INPUT_VOLTAGE_REGULATION_REGISTER), HEX);  // Expect: 0x08
  
  power.otg(true);                            // PMU ADC only works in Charging and OTG Modes. So, turning OTG On is essential

  #if defined(KOLIBRI_HARDWARE_REVISION_154)
  pinMode(GPIO_OUTPUT_PREBOOST, OUTPUT);
  digitalWrite(GPIO_OUTPUT_PREBOOST, HIGH);
  #endif
}

void loop() {
  // put your main code here, to run repeatedly:
  // Get Battery Voltage and calculate the Battery Percentage
  constexpr const float epsilon = 0.001;      // Ensure a non-zero voltage value to prevent divide by zero if used as denominator in any equation
  float battery_voltage = (power.battery_voltage() + epsilon) / 1000.0;
  uint8_t battery_percentage = calculate_battery_percentage(battery_voltage);
  Serial.printf("Kolibri's Battery is at %.4fV (%d%%).\n", battery_voltage, battery_percentage);
  delay(2000);
}