# Firmware Interface Tests
This repository contains the individual **component** interface test for the Kolibri Device. All the test uses the same pin definitions as in the [Demo](https://github.com/Pulmotree/fw-demo) and [RTOS](https://github.com/Pulmotree/fw-rtos).

To test for a given hardware revision of Kolibri (KB), enable the comment for that revision in the `pins.h` file in each Test. For Kolibri Revision 1.2 (KB R1.5), you can program it with either the USB or an external programmer. When using the external programmer to program KB R1.5, ensure that the programmer's `blue` tip is facing up when inserting it to the connector. The reverse is the case for KB R1.3. When using the USB to program KB R1.5, optionally enable `USB CDC On Boot` if you desire to print outputs to the serial monitor else ignore it.
<br>

![KB R1.5](./docs/images/KBR12.jpg) ![KB R1.3](./docs/images/KBR11.jpg)

Figure 1.1: (L-R) Kolibri R1.2 with the USB and the External Programmer Interface; Kolibri R1.1 with the External Programmer Interface.
For more details about the Hardware revisions see the schematics, [KB R1.5](./docs/schematics/KBR12.pdf) and [KB R1.3](./docs/schematics/KBR11.pdf).

## The Tests & Dependencies
The following describes the tests and the PASS conditions for each Test. Note that a battery must be connected to the device during the Tests. The Tests are dependent on these tools/Framework: [ESP-IDF V5.5^](https://github.com/espressif/esp-idf)/[Arduino-ESP32 Core V3.3.2^](https://github.com/espressif/arduino-esp32), Git and Arduino CLI; which can be installed by the instructions listed [here](https://github.com/Pulmotree/fw-demo/blob/main/docs/Setup.md). Also, [install](https://roboticsbackend.com/install-arduino-library-from-github/) the following external libraries with the commands listed below prior to testing the components.
- [SDP3X](https://github.com/Pulmotree/SDP3X)
- [MP2731](https://github.com/Pulmotree/MP2731)
- [MCP40DXX](https://github.com/Pulmotree/MCP40DXX)
- [AT21CSXX](https://github.com/Pulmotree/AT21CSXX)
- [TRF7970X](https://github.com/Pulmotree/TRF7970X)

Below are the commands to install/download and setup the libraries/dependencies and also test (compile, upload and monitor) two different versions simultaneously for Windows and Unix systems.
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

### ADC
The Analog-to-Digital (ADC) is used to measure the DC current, DC voltage and Phase (only in KB1.1) supplied to the Piezo Driver and the Piezo.
- **[TEST PASSED]** Serial Monitor displays an abitrary value in millivolts as measured by a muiltimeter. Current and Phase values are displayed only when the Piezo is energized.

### BLE
Bluetooth Low Energy (BLE) is a low-power communication medium used to connect Kolibri to either Pulmotree's Mobile App or a Third-party Mobile App for Lab/Test/Demo purposes. See [fw-demo setup](https://github.com/Pulmotree/fw-demo/blob/main/docs/Setup.md#testing-with-3rd-party-mobile-app) for a quick desricption of how to connect the Kolibri and a [Third-party Mobile App](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal&hl=en-US) for test.
- **[TEST PASSED]** Data transimitted to the BLE is received and also displayed in the serial monitor.


### LED
There are two LEDs on the Kolibri, an Orange LED and an RGB LED. The Orange LED is used for `charging` or `not-charging`/`fully-charged` indicatons whenever it is ON or OFF, respectively. It is controlled by the [wireless charger module](https://www.ti.com/lit/ds/symlink/bq51050b.pdf) and not the firmware. The RGB is fully controlled by the firmware and it is used for various status  indication. Please see the [specifiactions](https://pulmotree.sharepoint.com/:x:/r/sites/pm/_layouts/15/Doc.aspx?sourcedoc=%7BA43C0B4B-A18D-48B3-A9D7-D7B6E888C6E1%7D&file=PT001A%20Traceability%20Matrix_V3.0_draft.xlsx&action=default&mobileredirect=true) for more detailed uses.
- **[TEST PASSED]** Orange LED lights up when Kolibri (with not-fully charged battery connected) is placed on the wireless charger and turns off when removed. One of the RGB LED enabled in the test code blinks.


### Button
The Push Button of the Kolibri is used to get user's input for cleaning the Aerosol Generator; and for device turn-on and turn-off etc. 
- **[TEST PASSED]** Red LED of the RGB LED responds to each button press.


### Buzzer
The Buzzer is the primary source of acoustic feedback from the Kolibri to the user. It is expected to generate at least 2 different sounds/tone. More sounds can be obtained by modifying the PWM characteristics such as the Duty Cycle.
- **[TEST PASSED]** Buzzer beeps for 1s.

### Charger
The Wireless Charger Chip, [BQ51050B](https://www.ti.com/lit/ds/symlink/bq51050b.pdf), controls the Orange LED charge indidcator with its CHG# pin and this same pin is also used to sense when Kolibri is charging, not charging or fully charged.
- **[TEST PASSED]** Serial Monitor displays `Kolibri is charging` when the Kolibri is placed on the wireless charger and `Kolibri is NOT charging` when it is removed

### Motor
The Motor is the primary source of haptic feedback from the Kolibri to the user. It is expected to generate at least 3 different vibrations.  The vibrations can be obtained by modifying the PWM characteristics such as the Duty Cycle. This test only checks that the motor is functional. The PWM-based haptic feedback is described below.
- **[TEST PASSED]** On button press, the Motor turns ON and OFF at 1s interval.


### Piezo
The Head Aerosol Generator (HAG) is energized by this Test. The ESP32's built-in Frequency Generator (PWM Peripheral) and the Boost Converter feeds the Piezo Driver which produces an approximate resonant AC signal together with the RLC Tank Circuit. This signal resonates with the Piezo Mesh in the HAG to convert the Liquid Drug into droplets. This mechanism is sometimes called Nebulization; hence the name of this Test. Remember to use NaCl as your test liquid.
- **[TEST PASSED]** Nebulize for 3s at button press.


### Potentiometer
The Potentiometer in the Kolibri is used to continuously adjust the feedback voltage to the Boost Converter that supplies the voltage used by the Piezo Driver and the Piezo
- **[TEST PASSED]** Serial Monitor displays the approximate default Power-On-Reset resistance, 5000Ω and the set resistance, 3000Ω. Note that the resolution is ±50Ω.


### Power
The Power Management Chip, MP2731 provides Wired Charging, System Voltage, USB OTG Voltage (that feed the Booster) and Battery monitoring features to the Kolibri Device.
- **[TEST PASSED]** Serial Monitor displays the expected Power-On-Reset or default value, 0x06 of the `INPUT VOLTAGE REGULATION REGISTER` and the set value, 0x08


### Power and Potentiometer
The Power Management Chip, MP2731 and the Potentiometer, MCP40D18 share the same Inter-Integrated Circuit (I2C) Bus. This Test is to evaluate I2C Bus contention avoidance.
- **[TEST PASSED]** The same PASS conditions of Potentiometer and Power above.


### Pressure
The Differential Pressure Sensor, SDP31 in the Kolibri is used to measure the differential pressure caused by the user's inhalation and then converted to a flow rate by a characteristic polynomial obtained empirically. For more details about the conversion of the Differential Pressure to Flow Rate, see this [example](https://github.com/Pulmotree/SDP3X/blob/main/examples/flow_rate/flow_rate.ino).
- **[TEST PASSED]** Serial Monitor displays the Differential Pressure and Temperature measured at 1s interval.


### PWM Devices Combined
This Test evaluates the coexistance of all the components whose drivers are dependent on PWM. This is to test for crosstalk between the different PWM channels. The components under tests are Piezo, Buzzer, Motor and RGB LED
- **[TEST PASSED]** On button press, the same PASS conditions of Piezo, Buzzer, Motor and RGB LED above.


### Reset
This Test tests a handy feature - the ability of Kolibri to self-reset.
- **[TEST PASSED]** Resets the device if button is pressed down for 3s. This can be observe via the Serial Monitor.


### RGB
The Red, Green, Blue (RGB) LED is used for various status indication as described in the [Traceability Matrix](https://pulmotree.sharepoint.com/:x:/r/sites/pm/_layouts/15/Doc.aspx?sourcedoc=%7BA43C0B4B-A18D-48B3-A9D7-D7B6E888C6E1%7D&file=PT001A%20Traceability%20Matrix_V3.0_draft.xlsx&action=default&mobileredirect=true)
- **[TEST PASSED]** On button press, display the LED colours at 2s interval in this following order: RED, GREEN, BLUE, YELLOW, WHITE


### Sleep
This Test tests the device's deep-sleep capability. In this state, the Kolibri is expected to draw the least power in the order of µW.
- **[TEST PASSED]** Turns off all devices and periherials then goes to sleep. Only wake-up when the input button is pressed.


### Vibration
The Motor is the primary source of haptic feedback from the Kolibri to the user. It is expected to generate at least 3 different vibrations. The vibrations can be obtained by modifying the PWM characteristics such as the Duty cycle. This test only checks that the motor generates different vibrations.
- **[TEST PASSED]** On button press, three different vibrations are observed


### WiFi
The Wireless Fidelity (WiFi) is the wireless interface that provides Kolibri access to both WLAN and Internet access for numerous purposes. The WiFi Access Point can be tested by connecting your device (PC or Mobile Phone) to the Kolibri's Hotspot/Access Point whose SSID is `Kolibri` with no passphare. To complete the test, make these requests `GET \H`, `GET \L` to the address `192.168.1.4:80` from a [Third-party Mobile App](https://play.google.com/store/apps/details?id=de.kai_morich.serial_wifi_terminal&hl=en-US) or any Browser.
- **[TEST PASSED]** Turns ON and OFF the Green LED in the RGB when a `GET \H` and `GET \L` requests are made from a Third-party Mobile App or Browser, respectively.