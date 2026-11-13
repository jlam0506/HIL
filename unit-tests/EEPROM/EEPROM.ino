#include "AT21CSXX.h"
using namespace AT21CSXX_NS;
// AT21CSXX EEPROM(AT21CS01, A);
AT21CSXX EEPROM(AT21CS11, A);
// AT21CSXX EEPROM(AT21CSX1, A);

// #define KOLIBRI_HARDWARE_REVISION_13
#define KOLIBRI_HARDWARE_REVISION_15

#if defined (KOLIBRI_HARDWARE_REVISION_13)
#define GPIO_SINGLE_WIRE_INTERFACE        48
#define GPIO_OUTPUT_V3V3P_ENABLE          8
#elif defined (KOLIBRI_HARDWARE_REVISION_15)
#define GPIO_SINGLE_WIRE_INTERFACE        1
#define GPIO_OUTPUT_V3V3P_ENABLE          7
#endif

void print_data(const uint8_t data[], uint16_t size) {
  Serial.print("Data in HEX:");
  for (uint8_t i = 0; i < size; i++) {
    Serial.print(" 0x");
    Serial.print(data[i], HEX);
  }
  Serial.println();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(2000);
  Serial.println("EEPROM Test\n");

  pinMode(GPIO_OUTPUT_V3V3P_ENABLE, OUTPUT);
  #if defined (KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, LOW);
  #elif defined (KOLIBRI_HARDWARE_REVISION_15)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, HIGH);
  #endif

  if (!EEPROM.begin(GPIO_SINGLE_WIRE_INTERFACE)) {
    Serial.println("Could not initialize the EEPROM");
    while (1) delay(1000);
  }
  else {
    Serial.println("EEPROM initialized");
    Serial.printf("EEPROM Serial Number: %llu \n\n", EEPROM.serial_number());
  }

  uint8_t byte;
  uint8_t address = 0x42;
  EEPROM.read(address, &byte);
  Serial.printf("Read Byte: 0x%X \n", byte);

  byte = 0xAA;
  EEPROM.write(address, &byte);
  Serial.printf("Wrote Byte: 0x%X \n", byte);

  EEPROM.read(address, &byte);
  Serial.printf("Read Byte: 0x%X \n\n", byte);

  // Default Data
  constexpr const uint8_t START_ADDRESS = 0x40;
  uint8_t empty[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t full[8]  = {0, 1, 2, 3, 4, 5, 6, 7};
  uint8_t data[8];
  memcpy(data, empty, sizeof(empty));
  Serial.print("Empty "); print_data(data, sizeof(data));

  // Write Example
  memcpy(data, full, sizeof(full));
  if (EEPROM.write(START_ADDRESS, data, sizeof(data))) {
    Serial.println("EEPROM Write Successful");
  }
  else {
    Serial.println("EEPROM Write Unsuccessful");
    while (1) delay(1000);
  }
  Serial.print("Wrote "); print_data(data, sizeof(data));

  // Read Example
  memcpy(data, empty, sizeof(empty));
  if (EEPROM.read(START_ADDRESS, data, sizeof(data))) {
    Serial.println("EEPROM Read Successful");
  }
  else {
    Serial.println("EEPROM Read Unsuccessful");
    while (1) delay(1000);
  }
  Serial.print("Read "); print_data(data, sizeof(data));
}

void loop() {
  // put your main code here, to run repeatedly:
}
