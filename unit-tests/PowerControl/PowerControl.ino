#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLE2901.h>
#include <Wire.h>
#include <MP2731.h>
#include <MCP40DXX.h>
#include "../../pins.h"

// I2C Addresses and Clock Frequency Definitions
#define MP2731_I2C_ADDRESS          0x4B
#define MCP40D18_I2C_ADDRESS        0x2E
#define I2C_CLOCK_FREQUENCY         400000

// Piezo Pulse Width Modulation (PWM) Analog Output Definitions
#define PIEZO_PWM_TIMER_RESOLUTION  8       // 8 bit
#define PIEZO_PWM_BASE_FREQUENCY    143000  // In Hz
#define PIEZO_PWM_CHANNEL           0
#define PIEZO_PWM_DUTY_CYCLE        128
#define NO_PWM_DUTY_CYCLE           0

// Define how many conversion per pin will happen and reading the data will be and average of all conversions
#define NUMBER_OF_SAMPLES           5
#define NEBULIZATION_POWER          1.0
#define NEBULIZATION_MINIMUM_POWER  0.9
#define NEBULIZATION_MAXIMUM_POWER  1.8

using namespace MP2731_NS;
using namespace MCP40DXX_NS;

MP2731 power(MP2731_I2C_ADDRESS);
MCP40DXX potentiometer(MCP40D18_I2C_ADDRESS, RESISTANCE_10KΩ);  // RESISTANCE_10KΩ = 10KΩ

// Track sleep time
uint32_t inactivity_timestamp;

// Empty Detection Variables
bool nebulizer_active = false;
bool empty_detection_active = true;
constexpr const uint32_t EMPTY_DETECTION_IMPEDANCE = 300; // In Ohms (Ω)

// Define Queue handle
QueueHandle_t QueueHandle;
const uint8_t QueueElementSize = 1;

// Bluetooth Low Energy (BLE) UUID Definitions
// See the following for generating UUIDs: https://www.uuidgenerator.net/
#define BLE_DEVICE_NAME           "Kolibri"
#define SERVICE_UUID              "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID       "beb5483e-36e1-4688-b7f5-ea07361b26a8"
String rx_mode{};
String tx_mode{};
const String MODE_1_RECEIVED = "A1";
const String MODE_2_RECEIVED = "A2";
const String MODE_3_RECEIVED = "A3";
const String MODE_4_RECEIVED = "A4";
const String MODE_1_TRANSMIT = "D1";
const String MODE_2_TRANSMIT = "D2";
const String MODE_3_TRANSMIT = "D3";
const String MODE_4_TRANSMIT = "D4";
const String CLEAR_MODE      = "";

// BLE Variables
BLEServer *server = NULL;
BLECharacteristic *characteristic = NULL;
BLE2901 *descriptor_2901 = NULL;
bool device_connected = false;
bool old_device_connected = false;

// BLE Callbacks
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *server) {
    device_connected = true;
    inactivity_timestamp = millis();
  };

  void onDisconnect(BLEServer *server) {
    device_connected = false;
    inactivity_timestamp = millis();
  };
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    rx_mode = characteristic->getValue();

    if (rx_mode.length() > 0) {
      Serial.print("New value: ");
      Serial.println(rx_mode);
    }
  }
};

// Inline definintion of activation or deactivation of peripherals
inline void button_release(void) {
  while (digitalRead(GPIO_INPUT_BUTTON) == LOW);
}

inline bool button_pressed(void) {
  #define BUTTON_PRESS_RESET_DURATION 3000
  if (digitalRead(GPIO_INPUT_BUTTON) == LOW) {
    Serial.println("\n\nbutton pressed\n");
    uint32_t button_pressed_timestamp = millis();
    while (digitalRead(GPIO_INPUT_BUTTON) == LOW) {
      if (millis() - button_pressed_timestamp > BUTTON_PRESS_RESET_DURATION) {
        Serial.println("The device will reset now\n\n");
        delay(100);
        ESP.restart();
      }
      delay(20);
    }
    return true;
  }
  return false;
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

inline void nebulization_on (void) {
  ledcWriteChannel(PIEZO_PWM_CHANNEL, PIEZO_PWM_DUTY_CYCLE);
  Serial.println("Nebulization On");
}

inline void nebulization_off (void) {
  ledcWriteChannel(PIEZO_PWM_CHANNEL, NO_PWM_DUTY_CYCLE);
  Serial.println("Nebulization Off");
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

void power_control_task (void* pvParamters) {
  uint8_t nebulizer;
  #if defined (KOLIBRI_HARDWARE_REVISION_153)
  Serial.println("Power Control Task not suppport for Kolibri Hardware Revision 1.5.3");
  for(;;) {
    delay(1000);
  }
  #else
  for (;;) {
    if (QueueHandle != NULL) {  // Sanity check just to make sure the queue actually exists
      int16_t ret = xQueueReceive(QueueHandle, &nebulizer, portMAX_DELAY);
      if (ret == pdPASS) {      // Check if nebulizer is active then try to read data
        // Measure the voltage, current and phase shift all in mV. epsilon (ε) is an offset added to avoid divided by zero which is an invalid operation
        float epsilon = 1.0, feedback_voltage = epsilon, feedback_current = epsilon, feedback_phase_shift = epsilon;
        for (uint8_t count = 0; count < NUMBER_OF_SAMPLES; count++) {
          feedback_voltage += analogReadMilliVolts(GPIO_ADC_BOOST_VFB);                   // In mV
          feedback_current += analogReadMilliVolts(GPIO_ADC_BOOST_IFB);                   // In mV
          #if defined (KOLIBRI_HARDWARE_REVISION_13)
          feedback_phase_shift += analogReadMilliVolts(GPIO_ADC_PHASE_SHIFT);             // In mV
          #endif
          delay(10); //yield();
        }
        
        feedback_voltage /= NUMBER_OF_SAMPLES;
        feedback_current /= NUMBER_OF_SAMPLES;
        feedback_phase_shift /= NUMBER_OF_SAMPLES;
        Serial.printf(" Feedback Voltage: %.2fmV \n Feedback Current: %.2fmA/V \n Feedback Phase: %.2f° \n",  feedback_voltage, feedback_current / 10.0, feedback_phase_shift);
        
        // Calculate the Real Power (W). Note that the algorithm is meant to maintain the power between 1650mW to 1750mW
        #if defined (KOLIBRI_HARDWARE_REVISION_13)
        float boost_voltage = (feedback_voltage / 1000.0) * (120.0 / 10.0);               // "/1000" converts mV to V. "120/10" is the inverted Voltage Divider Ratio. Please see the schematics for more details
        #elif defined (KOLIBRI_HARDWARE_REVISION_15)
        float boost_voltage = (feedback_voltage / 1000.0) * (259.0 / 10.0);               // "/1000" converts mV to V. "259/10" is the inverted Voltage Divider Ratio. Please see the schematics for more details
        #else
        #error "You did not define any kolibri version"
        #endif
        float boost_current = (feedback_current / 1000.0) / 10.0;                         // "/1000" converts mV to A. "/10" converts V to A. Equation: I = V / ( R X Gain). R = 100mΩ, Gain of INA293B3 = 100V/V, Vmv = V/1000
        float boost_power = boost_voltage * boost_current;                                // In Watts (W)
        float average_power = boost_power * 0.72;                                         // "* 0.72" Probably the conversion from Apparent Power (VA) to Real Power (W)
        float impedance = boost_voltage / boost_current;                                  // In Ohms (Ω)
        Serial.printf(" Boost Voltage: %.2fV \n Boost Current: %.2fA \n Phase: %.2f° \n Boost Power: %.2fW \n Average Power: %.2fW \n Impedance: %.2fΩ \n",  boost_voltage, boost_current, feedback_phase_shift, boost_power, average_power, impedance);
        
        // Empty detection of liquid
        if(empty_detection_active  && (impedance >= EMPTY_DETECTION_IMPEDANCE)) {
          static uint8_t empty_detection_triggered_count = 0;
          empty_detection_triggered_count++;
          if (empty_detection_triggered_count == 3) {
            empty_detection_triggered_count = 0;
            if (average_power < 0.2 && boost_voltage > 5) {
              Serial.println("HAG is disconnected");
              // TODO: Turn off nebulizer
              // TODO: Turn off boost Power
              // TODO: ERROR, HAG_DISCONNECTED
            }
            else {
              // TODO: Save state the state of Kolibri e.g empty_detection_triggered
              // TODO: Here, call a function to disable the nebulizer because the Head Unit has exhausted the liquid
              power_boost_active(false);
              Serial.printf("[EMPTY DETECTION] Kolibri is empty with impedance (Ω): %.2f \n", impedance);
            }
          }
        }

        // Check the output power, then increase or decrease it to maintain nebuilizatlion quality by varying the feedback resistance to the boost converter
        /*
        if (average_power > 0.2) { // Ensures that HAG is connected
          if (average_power < NEBULIZATION_MINIMUM_POWER) {
              potentiometer.decrease();
              Serial.printf("New resistance: %.2fΩ \n", potentiometer.get());
          }
          else if (average_power > NEBULIZATION_MAXIMUM_POWER) {
              potentiometer.increase();
              Serial.printf("New resistance: %.2fΩ \n", potentiometer.get());
          }
        }
        */

        
        if(average_power < (NEBULIZATION_POWER - 0.04)) {
          if(average_power < (NEBULIZATION_POWER - 0.15)) {
            potentiometer.decrease(4);
          }
          else {
            potentiometer.decrease(2);
          }
        }
        else if(average_power > (NEBULIZATION_POWER + 0.04)) {
          if(average_power > (NEBULIZATION_POWER + 0.15)) {
            potentiometer.increase(4);
          }
          else{
            potentiometer.increase(2);
          }
        }
        
      }
      else if (ret == pdFALSE) {
        Serial.println("The `power control task` was unable to receive data from the Queue");
      }
    }
  }
  #endif
} 

void nebulization_task (void* pvParamters) {
  uint8_t nebulizer = 1;
  for(;;) {
    if (button_pressed()) {
      // Check if the queue exists AND if there is any free space in the queue
      if (QueueHandle != NULL && uxQueueSpacesAvailable(QueueHandle) > 0) {
        if ( xQueueSend(QueueHandle, (void *)&nebulizer, 0) == errQUEUE_FULL ) {
          // Message is unsuccessful
          Serial.println("The `nebulization task` was unable to send data into the Queue");
        }
        nebulization_on();
        delay(5000);
        nebulization_off();
      }
    }
    delay(200);
  }
}

void bluetooth_task(void* pvParamters) {
  // Create the BLE Device
  BLEDevice::init(BLE_DEVICE_NAME);

  // Create the BLE Server
  server = BLEDevice::createServer();
  server->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *service = server->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  characteristic = service->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
  characteristic->setCallbacks(new MyCallbacks());

  // Start the service
  service->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");

  for (;;) {
    // Process the mode received
    if (rx_mode == MODE_1_RECEIVED) {
      tx_mode = MODE_1_TRANSMIT;
      rx_mode = CLEAR_MODE;
      Serial.println("Mode 1");
      //nebulize_by(SIMULATION, mode1, sizeof(mode1)/sizeof(float));
    }
    else if (rx_mode == MODE_2_RECEIVED) {
      tx_mode = MODE_2_TRANSMIT;
      rx_mode = CLEAR_MODE;
      Serial.println("Mode 2");
      //nebulize_by(SIMULATION, mode2, sizeof(mode2)/sizeof(float));
    }
    else if (rx_mode == MODE_3_RECEIVED) {
      tx_mode = MODE_3_TRANSMIT;
      rx_mode = CLEAR_MODE;
      Serial.println("Mode 3");
      //nebulize_by(SIMULATION, mode3, sizeof(mode3)/sizeof(float));
    }
    else if (rx_mode == MODE_4_RECEIVED) {
      tx_mode = MODE_4_TRANSMIT;
      rx_mode = CLEAR_MODE;
      Serial.println("Mode 4");
      //nebulize_by(SIMULATION, mode4, sizeof(mode4)/sizeof(float));
    }

    // Notify the Mobile App that the requested mode has successfully executed
    if (tx_mode != rx_mode) {
      if (device_connected) {
        characteristic->setValue(tx_mode);
        characteristic->notify();
        delay(500);
      }
      tx_mode = CLEAR_MODE;
    }

    // Disconnection
    if (!device_connected && old_device_connected) {
      delay(1000);                    // Give the bluetooth stack the chance to get things ready
      server->startAdvertising();     // Restart advertising
      Serial.println("start advertising");
      old_device_connected = device_connected;
    }

    // Connection
    if (device_connected && !old_device_connected) {
      // Do stuff here on connecting
      old_device_connected = device_connected;
    }

    // Yeild for other tasks to access CPU time
    delay(50);
  }
}

void setup() {
  // Initialize the Universal Asynchronous Receive-Transmit (UART) Communication Interface:
  Serial.begin(DEBUG_BAUD_RATE);
  delay(DELAY_ON_BOOT);
  Serial.printf("KOLIBRI HARDWARE REVISION: %s\n", KOLIBRI_HARDWARE_REVISION);

  Serial.print("\n\nPower Control Test\n\n");
  
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
  potentiometer.limit(3500, 50, 10000);  // 3500Ω, 50Ω, 10000Ω
  Serial.printf("Default resistance: %fΩ \n", potentiometer.get());
  potentiometer.set(4000);  // 4000 Ω
  Serial.printf("New resistance: %fΩ \n", potentiometer.get());

  // Initialize the Power Supply (i.e. The switch-mode charger management IC)
  if (!power.begin(&Wire)) {
    Serial.println("Could not initialize the power");
    while (1)
      ;
  }

  // Set the power supply that the boost converter generates
  power_boost_active(true);

  // Optional for ESP32: Set the resolution to 9-12 bits (default is 12 bits)
  analogReadResolution(12);

  // Initialize the PWM controller for the Piezo
  ledcAttachChannel(GPIO_OUTPUT_PIEZO, PIEZO_PWM_BASE_FREQUENCY, PIEZO_PWM_TIMER_RESOLUTION, PIEZO_PWM_CHANNEL);
  // nebulization_off();

  // Create the queue which will have <QueueElementSize> number of elements, each of size `uint8_t` and pass the address to <QueueHandle>.
  QueueHandle = xQueueCreate(QueueElementSize, sizeof(uint8_t));
  // Check if the queue was successfully created
  if (QueueHandle == NULL) {
    Serial.println("Queue could not be created. Halt.");
    while (1) {
      delay(1000);  // Halt at this point as is not possible to continue
    }
  }

  // Create Nebulization Task
  xTaskCreate(nebulization_task, "Nebulization", 8 * 1024, NULL, 1, NULL);

  // Create Power Control Task
  xTaskCreate(power_control_task, "Power Control", 8 * 1024, NULL, 2, NULL);

  // Create Bluetooth Task
  xTaskCreate(bluetooth_task, "Bluetooth", 8 * 1024, NULL, 2, NULL);
}

void loop () {
  delay(1000);
}
