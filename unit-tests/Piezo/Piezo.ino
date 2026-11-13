/*
  Description: Nebulize for T seconds whenever a button is pressed
  Date: November, 2024
*/

// #define USE_BLE_INTERFACE  // Remove/Comment this line to use only the UART interface to control the basic nebulization parameter
// Command to change nebulization parameter via the BLE or UART Interface: {"Hz":140000, "W":1.7 , "s": 5}

#if defined(USE_BLE_INTERFACE)
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLE2901.h>
#endif
#include <Wire.h>
#include <MP2731.h>
#include <MCP40DXX.h>
#include "ArduinoJson.h"
#include "../../pins.h"

// I2C Addresses and Clock Frequency Definitions
#define MP2731_I2C_ADDRESS 0x4B
#define MCP40D18_I2C_ADDRESS 0x2E
#define I2C_CLOCK_FREQUENCY 400000

// Piezo Pulse Width Modulation (PWM) Analog Output Definitions
#define PIEZO_PWM_TIMER_RESOLUTION 4     // 8 bit
#define PIEZO_PWM_BASE_FREQUENCY 140000  // In Hz
#define PIEZO_PWM_CHANNEL 0
#define PIEZO_PWM_DUTY_CYCLE 8
#define NO_PWM_DUTY_CYCLE 0

// LED Pulse Width Modulation (PWM) Analog Output Definitions
#define LED_PWM_TIMER_RESOLUTION 8   // 8 bit (ie. 0 to 255)
#define LED_PWM_BASE_FREQUENCY 5000  // In Hz
#define LED_R_PWM_CHANNEL 3
#define LED_G_PWM_CHANNEL 4
#define LED_B_PWM_CHANNEL 5
#define LED_PWM_CHANNEL LED_G_PWM_CHANNEL  // Just for completeness
#define LED_PWM_DUTY_CYCLE 128

#define MINIMUM_FREQUENCY 135000
#define MAXIMUM_FREQUENCY 145000
#define MINIMUM_NEBULIZATION_POWER 1.0
#define MAXIMUM_NEBULIZATION_POWER 2.0
#define MINIMUM_TIME_OF_CONSTANT_NEBULIZATION 3000
#define MAXIMUM_TIME_OF_CONSTANT_NEBULIZATION 300000

#if defined(USE_BLE_INTERFACE)
#define BLE_DEVICE_NAME "Kolibri 01"
#define BATTERY_SERVICE_UUID "180F"
#define BATTERY_CHARACTERISTIC_UUID "2A19"
#define COMMUNICATION_SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define COMMUNICATION_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#endif

using namespace MP2731_NS;
using namespace MCP40DXX_NS;

MP2731 power(MP2731_I2C_ADDRESS);
MCP40DXX potentiometer(MCP40D18, RESISTANCE_10KΩ);  // RESISTANCE_10KΩ = 10KΩ

#if defined(USE_BLE_INTERFACE)
// BLE Variables
BLEServer *server = NULL;
BLECharacteristic *communication_characteristic = NULL;
BLECharacteristic *battery_characteristic = NULL;
BLE2901 *descriptor_2901 = NULL;
#endif

// The Nebulization Parameters
// {"Hz":PIEZO_PWM_BASE_FREQUENCY, "W":1.7 , "s": 5}
uint32_t frequency = PIEZO_PWM_BASE_FREQUENCY;
float nebulization_power = 1.7;
uint32_t time_of_constant_nebulization = 5000;

// Inline definintion of activation or deactivation of peripherals
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

inline void nebulization_on(void) {
  power.otg(true);
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
  power.otg(false);
  #if defined(KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(GPIO_OUTPUT_POWER_ENABLE, LOW);
  #endif
  #if defined(KOLIBRI_HARDWARE_REVISION_154)
  digitalWrite(GPIO_OUTPUT_PREBOOST, HIGH);
  #endif
  ledcWriteChannel(PIEZO_PWM_CHANNEL, NO_PWM_DUTY_CYCLE);
  Serial.println("Nebulization Off");
}

// This function is essential for the OTG to function properly
void boot_completion_indication(void) {
  int16_t brightness = 0;  // how bright the LED is
  int16_t fadeAmount = 5;  // how many points to fade the LED by
  for (int16_t i = 0; i < 150; i++) {
    // Set the brightness on LEDC channel 0
    ledcWriteChannel(LED_PWM_CHANNEL, brightness);

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

void process(String &input) {
  // Allocate the JSON document
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, input.c_str());

  // Test if parsing succeeds
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
  } else {
    frequency = doc["Hz"];
    nebulization_power = doc["W"];
    time_of_constant_nebulization = doc["s"];

    delay(50);
    Serial.print("Frequency (Hz): ");
    Serial.println(frequency);
    Serial.print("Nebulization Power (W): ");
    Serial.println(nebulization_power);
    Serial.print("Time of Constant Nebulization (s): ");
    Serial.println(time_of_constant_nebulization);

    frequency = frequency > MAXIMUM_FREQUENCY ? MAXIMUM_FREQUENCY : frequency;
    frequency = frequency < MINIMUM_FREQUENCY ? MINIMUM_FREQUENCY : frequency;

    nebulization_power *= 1000;  // x1000 converts to mW
    nebulization_power = nebulization_power > MAXIMUM_NEBULIZATION_POWER ? MAXIMUM_NEBULIZATION_POWER : nebulization_power;
    nebulization_power = nebulization_power < MINIMUM_NEBULIZATION_POWER ? MINIMUM_NEBULIZATION_POWER : nebulization_power;

    time_of_constant_nebulization *= 1000;  // x1000 converts to ms
    time_of_constant_nebulization = time_of_constant_nebulization > MAXIMUM_TIME_OF_CONSTANT_NEBULIZATION ? MAXIMUM_TIME_OF_CONSTANT_NEBULIZATION : time_of_constant_nebulization;
    time_of_constant_nebulization = time_of_constant_nebulization < MINIMUM_TIME_OF_CONSTANT_NEBULIZATION ? MINIMUM_TIME_OF_CONSTANT_NEBULIZATION : time_of_constant_nebulization;

    ledcChangeFrequency(GPIO_OUTPUT_PIEZO, frequency, PIEZO_PWM_TIMER_RESOLUTION);
    Serial.printf("Frequency has changed to: %dHz\n", frequency);
    //potentiometer.write(map((int)nebulization_power, MAXIMUM_NEBULIZATION_POWER, MINIMUM_NEBULIZATION_POWER, 5, 35)); // TODO: Needs to be converted from float to int between 0 and 128. Also, resultant value needs to be clamp between 1000 to 10000 ohms
  }
}

void serialEventRun(void) {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\r');
    while (Serial.available() > 0) Serial.read();
    process(input);
  }
}

#if defined(USE_BLE_INTERFACE)
// BLE Callbacks
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *server) {
    Serial.println("Device Connected");
  };

  void onDisconnect(BLEServer *server) {
    Serial.println("Device Disconnected");
  };
};

class CommunicationCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String input = communication_characteristic->getValue();
    process(input);
  }
};

class BatteryCallbacks : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *pCharacteristic) {
    uint8_t battery_percentage = map(power.battery_voltage(), 3.2, 4200, 0, 100);  // Convert from (3300mV, 4200mV) to (0% to 100%)
    battery_characteristic->setValue(&battery_percentage, sizeof(battery_percentage));
    battery_characteristic->notify();
    Serial.printf("\n\nBattery Percentage: %d\n\n", battery_percentage);
  }
};
#endif

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
  digitalWrite(GPIO_OUTPUT_PREBOOST, LOW);
  #endif
  #endif

  pinMode(GPIO_OUTPUT_MOTOR, OUTPUT);
  digitalWrite(GPIO_OUTPUT_MOTOR, LOW);

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
  potentiometer.limit(1000, 50, 10000);  // 3500Ω, 50Ω, 10000Ω
  Serial.printf("Default resistance: %dΩ \n", potentiometer.get());
  potentiometer.set(2000);  // 4000 Ω
  Serial.printf("New resistance: %dΩ \n", potentiometer.get());

  // Initialize the Power Supply (i.e. The switch-mode charger management IC)
  if (!power.begin(&Wire)) {
    Serial.println("Could not initialize the power");
    while (1)
      ;
  }

  // Set the power supply that the boost converter generates
  turn_on_boost_converter();

  // Initialize the PWM controller for the Piezo
  ledcAttachChannel(GPIO_OUTPUT_PIEZO, PIEZO_PWM_BASE_FREQUENCY, PIEZO_PWM_TIMER_RESOLUTION, PIEZO_PWM_CHANNEL);

  // Boot Delay: Essential for VOTG Stabilization
  ledcAttachChannel(GPIO_OUTPUT_LED_GREEN, LED_PWM_BASE_FREQUENCY, LED_PWM_TIMER_RESOLUTION, LED_G_PWM_CHANNEL);
  boot_completion_indication();

#if defined(USE_BLE_INTERFACE)
  // Create the BLE Device and a BLE Server
  BLEDevice::init(BLE_DEVICE_NAME);
  server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  // Create the Service and Characteristic for Battery
  BLEService *battery_service = server->createService(BATTERY_SERVICE_UUID);
  battery_characteristic = battery_service->createCharacteristic(BATTERY_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  battery_characteristic->setCallbacks(new BatteryCallbacks());

  // Create the Service and Characteristic for Communication
  BLEService *communication_service = server->createService(COMMUNICATION_SERVICE_UUID);
  communication_characteristic = communication_service->createCharacteristic(COMMUNICATION_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
  communication_characteristic->setCallbacks(new CommunicationCallbacks());
  // Start the service
  battery_service->start();
  communication_service->start();
  // Start advertising
  BLEDevice::startAdvertising();
#endif
}

void loop() {
  // Nebulize for 3s whenever the button is pressed
  if (button_pressed()) {
    nebulization_on();
    delay(time_of_constant_nebulization);
    nebulization_off();
    delay(1000);
  }
}