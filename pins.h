#ifndef PINS_H_DEFINED
#define PINS_H_DEFINED

// Select the Kolibri Version you want to test by uncommenting one of the lines below
// KOLIBRI_HARDWARE_REVISION_13 is selected by default if nothing is defined
// #define KOLIBRI_HARDWARE_REVISION_13
// #define KOLIBRI_HARDWARE_REVISION_15

/* Below are the commands to install/download and setup the libraries/dependencies and also test (compile, upload and monitor) two different versions simultaneously for Windows and Unix systems.
```bash
# Windows:
# Change directory to the Arduino libraries folder
cd %USERPROFILE%\Documents\Arduino\libraries\

# Install/Download the Dependencies/Libraries
arduino-cli lib install ArduinoJson@7.4.2
git clone https://github.com/Pulmotree/SDP3X.git
git clone https://github.com/Pulmotree/MP2731.git
git clone https://github.com/Pulmotree/MCP40DXX.git
git clone https://github.com/Pulmotree/AT21CSXX.git
git clone https://github.com/Pulmotree/TRF7970A.git

# Change directory to the Arduino Project folder
cd %USERPROFILE%\Documents\Arduino\

# Download the Tests:
git clone https://github.com/Pulmotree/fw-tests.git

# Change directory to the root folder of any interface unit test e.g. LED
cd fw-tests/unit-tests/LED

# Compile and Upload an interface unit test. Remove ":CDCOnBoot=cdc,USBMode=hwcdc" if you're using UART-USB interface and not USB
# Powershell 5.X.X
arduino-cli compile -v -j12 `
  --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc,USBMode=hwcdc `
  --libraries "~\Documents\Arduino\libraries" `
  --build-property "compiler.cpp.extra_flags=-DDEVICE_NUMBER=01 -DKOLIBRI_FIRMWARE_VERSION=0 -DKOLIBRI_HARDWARE_REVISION_13" `
  -u -p COM4
if ($?) {
arduino-cli compile -v -j12 `
  --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc,USBMode=hwcdc `
  --libraries "~\Documents\Arduino\libraries" `
  --build-property "compiler.cpp.extra_flags=-DDEVICE_NUMBER=02 -DKOLIBRI_FIRMWARE_VERSION=0 -DKOLIBRI_HARDWARE_REVISION_15" `
  -u -p COM5 if ($?) { arduino-cli monitor -c 115200 -p COM5 }
}

# Powershell 7.X.X (pwsh.exe, Check Version: $PSVersionTable.PSVersion)
arduino-cli compile -v -j12 `
  --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc,USBMode=hwcdc `
  --libraries "~\Documents\Arduino\libraries" `
  --build-property "compiler.cpp.extra_flags=-DDEVICE_NUMBER=01 -DKOLIBRI_FIRMWARE_VERSION=0 -DKOLIBRI_HARDWARE_REVISION_13" `
  -u -p COM4 && `
arduino-cli compile -v -j12 `
  --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc,USBMode=hwcdc `
  --libraries "~\Documents\Arduino\libraries" `
  --build-property "compiler.cpp.extra_flags=-DDEVICE_NUMBER=02 -DKOLIBRI_FIRMWARE_VERSION=0 -DKOLIBRI_HARDWARE_REVISION_15" `
  -u -p COM5 && arduino-cli monitor -c 115200 -p COM5

# Serial Monitor
arduino-cli monitor -c 115200 -p COM4
arduino-cli monitor -c 115200 -p COM5
```

```bash
# UNIX
# Change directory to the Arduino libraries folder. For Mac OS use: cd ~/Documents/Arduino/libraries/
cd ~/Arduino/libraries/

# Install/Download the Dependencies/Libraries
arduino-cli lib install ArduinoJson@7.4.2
git clone https://github.com/Pulmotree/SDP3X.git
git clone https://github.com/Pulmotree/MP2731.git
git clone https://github.com/Pulmotree/MCP40DXX.git
git clone https://github.com/Pulmotree/AT21CSXX.git
git clone https://github.com/Pulmotree/TRF7970A.git

# Change directory to the Arduino Project folder. For Mac OS use: cd ~/Documents/Arduino/
cd ~/Arduino/

# Download the Tests:
git clone https://github.com/Pulmotree/fw-tests.git

# Change directory to the root folder of any interface unit test e.g. LED
cd fw-tests/unit-tests/LED

# Compile and Upload an interface unit test. For Mac OS use: /dev/tty.usbserial-14110. Remove ":CDCOnBoot=cdc,USBMode=hwcdc" if you're using UART-USB interface and not USB
arduino-cli compile -v -j12 \
  --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc,USBMode=hwcdc \
  --libraries "~/Arduino/libraries/" \
  --build-property "compiler.cpp.extra_flags=-DDEVICE_NUMBER=01 -DKOLIBRI_FIRMWARE_VERSION=0 -DKOLIBRI_HARDWARE_REVISION_13" \
  -u -p /dev/ttyUSB0 \
  && \
  arduino-cli compile -v -j12 \
    --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc,USBMode=hwcdc \
    --libraries "~/Arduino/libraries/" \
    --build-property "compiler.cpp.extra_flags=-DDEVICE_NUMBER=02 -DKOLIBRI_FIRMWARE_VERSION=0 -DKOLIBRI_HARDWARE_REVISION_15" \
    -u -p /dev/ttyUSB1

# Serial Monitor. For Mac OS use: /dev/tty.usbserial-14110
arduino-cli monitor -c 115200 -p /dev/ttyUSB0
arduino-cli monitor -c 115200 -p /dev/ttyUSB1
```
*/

#if !defined(KOLIBRI_HARDWARE_REVISION_13) && !defined(KOLIBRI_HARDWARE_REVISION_15) && !defined(KOLIBRI_HARDWARE_REVISION_151) && !defined(KOLIBRI_HARDWARE_REVISION_152) && !defined(KOLIBRI_HARDWARE_REVISION_153) && !defined(KOLIBRI_HARDWARE_REVISION_154)
#define KOLIBRI_HARDWARE_REVISION_15   4
#define KOLIBRI_HARDWARE_REVISION_154  4
#elif defined(KOLIBRI_HARDWARE_REVISION_13) && !defined(KOLIBRI_HARDWARE_REVISION_15)
#undef KOLIBRI_HARDWARE_REVISION_13
#define KOLIBRI_HARDWARE_REVISION_13  130
#elif !defined(KOLIBRI_HARDWARE_REVISION_13) && defined(KOLIBRI_HARDWARE_REVISION_15)
#if KOLIBRI_HARDWARE_REVISION_15   == 1
#define KOLIBRI_HARDWARE_REVISION_151 1
#elif KOLIBRI_HARDWARE_REVISION_15 == 2
#define KOLIBRI_HARDWARE_REVISION_152 2
#elif KOLIBRI_HARDWARE_REVISION_15 == 3
#define KOLIBRI_HARDWARE_REVISION_153 3
#elif KOLIBRI_HARDWARE_REVISION_15 == 4
#define KOLIBRI_HARDWARE_REVISION_154 4
#else
#define KOLIBRI_HARDWARE_REVISION_154 4
#warning "Kolibri Hardware Revision not defined! Using the latest revision 1.5.4 of KB HW R1.5.4"
#endif
#elif !defined(KOLIBRI_HARDWARE_REVISION_13) && !defined(KOLIBRI_HARDWARE_REVISION_15) && defined(KOLIBRI_HARDWARE_REVISION_151) && !defined(KOLIBRI_HARDWARE_REVISION_152) && !defined(KOLIBRI_HARDWARE_REVISION_153) && !defined(KOLIBRI_HARDWARE_REVISION_154)
#define KOLIBRI_HARDWARE_REVISION_15
#undef KOLIBRI_HARDWARE_REVISION_151
#define KOLIBRI_HARDWARE_REVISION_151 1
#elif !defined(KOLIBRI_HARDWARE_REVISION_13) && !defined(KOLIBRI_HARDWARE_REVISION_15) && !defined(KOLIBRI_HARDWARE_REVISION_151) && defined(KOLIBRI_HARDWARE_REVISION_152) && !defined(KOLIBRI_HARDWARE_REVISION_153) && !defined(KOLIBRI_HARDWARE_REVISION_154)
#define KOLIBRI_HARDWARE_REVISION_15
#undef KOLIBRI_HARDWARE_REVISION_152
#define KOLIBRI_HARDWARE_REVISION_152 2
#elif !defined(KOLIBRI_HARDWARE_REVISION_13) && !defined(KOLIBRI_HARDWARE_REVISION_15) && !defined(KOLIBRI_HARDWARE_REVISION_151) && !defined(KOLIBRI_HARDWARE_REVISION_152) && defined(KOLIBRI_HARDWARE_REVISION_153) && !defined(KOLIBRI_HARDWARE_REVISION_154)
#define KOLIBRI_HARDWARE_REVISION_15
#undef KOLIBRI_HARDWARE_REVISION_153
#define KOLIBRI_HARDWARE_REVISION_153 3
#elif !defined(KOLIBRI_HARDWARE_REVISION_13) && !defined(KOLIBRI_HARDWARE_REVISION_15) && !defined(KOLIBRI_HARDWARE_REVISION_151) && !defined(KOLIBRI_HARDWARE_REVISION_152) && !defined(KOLIBRI_HARDWARE_REVISION_153) && defined(KOLIBRI_HARDWARE_REVISION_154)
#define KOLIBRI_HARDWARE_REVISION_15
#undef KOLIBRI_HARDWARE_REVISION_154
#define KOLIBRI_HARDWARE_REVISION_154 4
#else
#error You defined more than one Hardware Revision
#endif

// General-Purpose Input and Output (GPIO) Pin Definitions for Analog IO, Digital IO, Inter-integrated Circuit (I2C), Inter-integrated Circuit Sound (I2S) and Serial Peripheral Interface (SPI)
// Pin naming convention:
// #define GPIO_[CLASS]_[FUNCTION]  GPIO_NUM_[x]

#if defined(KOLIBRI_HARDWARE_REVISION_15)
#define GPIO_UNUSED_NONE1           GPIO_NUM_35
#define GPIO_UNUSED_NONE2           GPIO_NUM_36
#define GPIO_UNUSED_NONE3           GPIO_NUM_37
#define GPIO_UNUSED_NONE4           GPIO_NUM_45
#define GPIO_UNUSED_NONE5           GPIO_NUM_47
#define GPIO_UNUSED_NONE6           GPIO_NUM_48
#define GPIO_IGNORE                 GPIO_NUM_NC

#define GPIO_I2C_SCL0               GPIO_NUM_39
#define GPIO_I2C_SDA0               GPIO_NUM_38
#define GPIO_I2C_SDA1               GPIO_NUM_2
#define GPIO_I2C_SCL1               GPIO_NUM_41
#define GPIO_I2C_EEPROM             GPIO_NUM_1 

#define GPIO_SPI_SCLK               GPIO_NUM_9
#define GPIO_SPI_MISO               GPIO_NUM_11
#define GPIO_SPI_MOSI               GPIO_NUM_10
#define GPIO_SPI_SS                 GPIO_NUM_12

#if KOLIBRI_HARDWARE_REVISION_151
#define KOLIBRI_HARDWARE_REVISION   "KOLIBRI_HARDWARE_REVISION_151"
#define GPIO_ADC_BOOST_VFB          GPIO_NUM_4
#define GPIO_ADC_BOOST_IFB          GPIO_NUM_6
#define GPIO_INPUT_BUTTON           GPIO_NUM_0
#elif KOLIBRI_HARDWARE_REVISION_152
#define KOLIBRI_HARDWARE_REVISION   "KOLIBRI_HARDWARE_REVISION_152"
#define GPIO_ADC_BOOST_VFB          GPIO_NUM_4
#define GPIO_ADC_BOOST_IFB          GPIO_NUM_6
#define GPIO_INPUT_RTC_IRQ          GPIO_NUM_42
#define GPIO_INPUT_BUTTON           GPIO_NUM_14
#elif KOLIBRI_HARDWARE_REVISION_153
#define KOLIBRI_HARDWARE_REVISION   "KOLIBRI_HARDWARE_REVISION_153"
#define GPIO_INPUT_RTC_IRQ          GPIO_NUM_4
#define GPIO_INPUT_METER_IRQ        GPIO_NUM_6
#define GPIO_INPUT_BUTTON           GPIO_NUM_14
#elif KOLIBRI_HARDWARE_REVISION_154
#define KOLIBRI_HARDWARE_REVISION   "KOLIBRI_HARDWARE_REVISION_154"
#define GPIO_INPUT_RTC_IRQ          GPIO_NUM_4
#define GPIO_INPUT_METER_IRQ        GPIO_NUM_6
#define GPIO_INPUT_BUTTON           GPIO_NUM_14
#define GPIO_OUTPUT_PREBOOST        GPIO_NUM_21
#else
#define GPIO_ADC_BOOST_VFB          GPIO_NUM_4
#define GPIO_ADC_BOOST_IFB          GPIO_NUM_6
#define GPIO_INPUT_BUTTON           GPIO_NUM_0
#endif
 
#define GPIO_INPUT_WAKEUP_IRQ       GPIO_INPUT_BUTTON
#define GPIO_INPUT_POWER_IRQ        GPIO_NUM_13
#define GPIO_INPUT_RFID_IRQ         GPIO_NUM_5
#define GPIO_INPUT_PRESSURE_IRQ     GPIO_NUM_42
#define GPIO_INPUT_WIRELESS_CHARGER GPIO_NUM_15

#define GPIO_OUTPUT_V3V3P_ENABLE    GPIO_NUM_7 
#define GPIO_OUTPUT_RFID_ENABLE     GPIO_NUM_46
#define GPIO_OUTPUT_MOTOR           GPIO_NUM_8
#define GPIO_OUTPUT_BUZZER          GPIO_NUM_3
#define GPIO_OUTPUT_LED_RED         GPIO_NUM_17
#define GPIO_OUTPUT_LED_GREEN       GPIO_NUM_16
#define GPIO_OUTPUT_LED_BLUE        GPIO_NUM_18
#define GPIO_OUTPUT_PIEZO           GPIO_NUM_40

#define GPIO_UART_RXD0              GPIO_NUM_44
#define GPIO_UART_TXD0              GPIO_NUM_43

#define GPIO_USB_DN1                GPIO_NUM_19
#define GPIO_USB_DP1                GPIO_NUM_20

#elif defined(KOLIBRI_HARDWARE_REVISION_13)
#define KOLIBRI_HARDWARE_REVISION   "KOLIBRI_HARDWARE_REVISION_13"
#define GPIO_OUTPUT_V3V3P_ENABLE    GPIO_NUM_8
#define GPIO_OUTPUT_POWER_ENABLE    GPIO_NUM_10

#define GPIO_I2C_SCL0               GPIO_NUM_20
#define GPIO_I2C_SDA0               GPIO_NUM_21
#define GPIO_I2C_SDA1               GPIO_NUM_18
#define GPIO_I2C_SCL1               GPIO_NUM_17
#define GPIO_I2C_EEPROM             GPIO_NUM_48

#define GPIO_I2S_DIN                GPIO_NUM_47
#define GPIO_I2S_BCLK               GPIO_NUM_36
#define GPIO_I2S_LRCLK              GPIO_NUM_38

#define GPIO_SPI_SCLK               GPIO_NUM_39
#define GPIO_SPI_MISO               GPIO_NUM_41
#define GPIO_SPI_MOSI               GPIO_NUM_40
#define GPIO_SPI_SS                 GPIO_NUM_42

#define GPIO_ADC_PHASE_SHIFT        GPIO_NUM_1
#define GPIO_ADC_BOOST_VFB          GPIO_NUM_2
#define GPIO_ADC_BOOST_IFB          GPIO_NUM_5

#define GPIO_INPUT_BUTTON           GPIO_NUM_4
#define GPIO_INPUT_WAKEUP_IRQ       GPIO_INPUT_BUTTON
#define GPIO_INPUT_POWER_IRQ        GPIO_NUM_19
#define GPIO_INPUT_RFID_IRQ         GPIO_NUM_6
#define GPIO_INPUT_PRESSURE_IRQ     GPIO_NUM_16
#define GPIO_INPUT_WIRELESS_CHARGER GPIO_NUM_15

#define GPIO_OUTPUT_RFID_ENABLE     GPIO_NUM_37
#define GPIO_OUTPUT_MOTOR           GPIO_NUM_11
#define GPIO_OUTPUT_LED_RED         GPIO_NUM_13
#define GPIO_OUTPUT_LED_GREEN       GPIO_NUM_12
#define GPIO_OUTPUT_LED_BLUE        GPIO_NUM_14
#define GPIO_OUTPUT_PIEZO           GPIO_NUM_9

#endif

#define STRINGIFY(x)                #x
#define EXPAND_AND_STRINGIFY(x)     STRINGIFY(x)

#ifndef DEVICE_NUMBER
#define DEVICE_NUMBER               "01"
#warning "DEVICE_NUMBER not defined! Using the default"
#endif

#ifndef DEBUG_BAUD_RATE
#define DEBUG_BAUD_RATE             115200
#endif

#ifndef DELAY_ON_BOOT
#define DELAY_ON_BOOT               2000
#endif

#endif // PINS_H_DEFINED

/*
Powershell 5.X.X
arduino-cli compile -v -j12 `
  --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc,USBMode=hwcdc `
  --libraries "~\Documents\Arduino\libraries" `
  --build-property "compiler.cpp.extra_flags=-DDEVICE_NUMBER=2 -DKOLIBRI_FIRMWARE_VERSION=0 -DKOLIBRI_HARDWARE_REVISION_153=3" `
  -u -p COM13
  if ($?) { arduino-cli monitor -c 115200 -p COM13 }

  Powershell 7.X.X
  arduino-cli compile -v -j12 `
  --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc,USBMode=hwcdc `
  --libraries "~\Documents\Arduino\libraries" `
  --build-property "compiler.cpp.extra_flags=-DDEVICE_NUMBER=2 -DKOLIBRI_FIRMWARE_VERSION=0 -DKOLIBRI_HARDWARE_REVISION_153=3" `
  -u -p COM13 && arduino-cli monitor -c 115200 -p COM13
*/