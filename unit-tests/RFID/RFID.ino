// RADIO FREQUENCY IDENTIFICATION (RFID) & NEAR-FIELD COMMUNICATION (NFC) READER EXAMPLE

#include <SPI.h>
#include <TRF7970A.h>

// #define KOLIBRI_HARDWARE_REVISION_13
#define KOLIBRI_HARDWARE_REVISION_15

#if defined(KOLIBRI_HARDWARE_REVISION_13)
#define GPIO_OUTPUT_V3V3P_ENABLE    GPIO_NUM_8
#define GPIO_OUTPUT_RFID_ENABLE     GPIO_NUM_37
#define GPIO_INPUT_RFID_IRQ         GPIO_NUM_6
#define GPIO_SPI_SCLK               GPIO_NUM_39
#define GPIO_SPI_MISO               GPIO_NUM_41
#define GPIO_SPI_MOSI               GPIO_NUM_40
#define GPIO_SPI_SS                 GPIO_NUM_42
#elif defined(KOLIBRI_HARDWARE_REVISION_15)
#define GPIO_OUTPUT_V3V3P_ENABLE    GPIO_NUM_7
#define GPIO_OUTPUT_RFID_ENABLE     GPIO_NUM_46
#define GPIO_INPUT_RFID_IRQ         GPIO_NUM_5
#define GPIO_SPI_SCLK               GPIO_NUM_9
#define GPIO_SPI_MISO               GPIO_NUM_11
#define GPIO_SPI_MOSI               GPIO_NUM_10
#define GPIO_SPI_SS                 GPIO_NUM_12
#endif

using namespace TRF7970A_NS;
TRF7970A NFC(GPIO_SPI_SS, GPIO_INPUT_RFID_IRQ);

uint8_t keyA[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t uid[4] = { 0xDE, 0xAD, 0xBE, 0xEF };  // Replace with actual tag UID

// Inline definintion of activation or deactivation of peripherals
inline void turn_on_3v3_switch(void) {
#if defined(KOLIBRI_HARDWARE_REVISION_13)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, LOW);
#elif defined(KOLIBRI_HARDWARE_REVISION_15)
  digitalWrite(GPIO_OUTPUT_V3V3P_ENABLE, HIGH);
#endif
}

void setup() {
  Serial.begin(115200);

  // Turn on the 3V3 Peripheral Subsystem. It is Active low (It is called 3V3Enable in the schematics)
  pinMode(GPIO_OUTPUT_V3V3P_ENABLE, OUTPUT);
  turn_on_3v3_switch();

  // Initialize the SPI Interface
  SPI.begin(GPIO_SPI_SCLK, GPIO_SPI_MISO, GPIO_SPI_MOSI, GPIO_SPI_SS);

  // Initialize the RFID/NFC Chip
  pinMode(GPIO_OUTPUT_RFID_ENABLE, OUTPUT);
  digitalWrite(GPIO_OUTPUT_RFID_ENABLE, HIGH);
  if (!NFC.begin(MODE_ISO14443A, &SPI)) {
    Serial.println("Could not initialize the RFID Device");
    while (1)
      delay(1000);
  }
  else {
    Serial.println("The RFID Device is initialized");
  }
}

void loop() {
  if (NFC.detectTag()) {
    Serial.println("Tag detected!");

    if (NFC.mifareAuthenticate(4, keyA, uid)) {
      Serial.println("Authenticated!");

      uint8_t data[16];
      if (NFC.mifareReadBlock(4, data)) {
        Serial.print("Block 4: ");
        for (int i = 0; i < 16; i++) {
          Serial.print(data[i], HEX);
          Serial.print(" ");
        }
        Serial.println();
      }
    }

    delay(2000);
  }
  delay(2000);
}
