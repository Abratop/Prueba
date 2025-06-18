# Teensy EVI MIDI Controller Firmware

## Overview
This firmware turns a PJRC Teensy 4.1 microcontroller into the brains of an Electronic Valve Instrument (EVI), sending MIDI messages based on sensor inputs. It supports various fingerings, breath control, pitch bend, and other expressive features.

## Hardware Requirements
- PJRC Teensy 4.1 development board
- 3x MPR121 Capacitive Touch Sensor Breakout Boards (ensure I2C addresses are set differently, e.g., 0x5A, 0x5B, 0x5C)
- NXP MP3V5004GP Piezoresistive Transducer (or similar analog pressure sensor) for breath sensing
- Analog sensors for pitch bend (2x, typically linear potentiometers or IR distance sensors), vibrato/portamento lever, bite sensor, and lip sensor.
- Appropriate circuitry for connecting sensors to the Teensy (e.g., voltage dividers, pull-up/down resistors as needed).

## Hardware Connection Guide (Pinout)

This guide outlines the connections between the Teensy 4.1 and the various sensors. For the most accurate and up-to-date pin assignments, **always refer to the definitions in the `config.h` file of this firmware.**

**Power Considerations:**
- Most sensors and breakout boards will require connections to 3.3V (or 5V, depending on the specific module) and GND on the Teensy. Ensure your sensors are powered according to their datasheets.
- **Breath Sensor (NXP MP3V5004GP):** This sensor is typically rated for a 5V supply. The Teensy 4.1's analog pins are generally **3.3V tolerant only**. If powering the MP3V5004GP with 5V, its output can exceed 3.3V. A voltage divider circuit is **required** on the sensor's output signal line to scale it down to a 0-3.3V range before connecting to the Teensy's analog input pin. Alternatively, if the sensor operates correctly and provides sufficient range when powered directly from 3.3V, that would simplify connections.
- **MPR121 Breakouts:** These usually operate at 3.3V.

**1. Built-in Teensy Capacitive Touch Pins:**
   - Key 1: Connect to Teensy pin defined by `TOUCH_PIN_0` in `config.h`.
   - Key 2: Connect to Teensy pin defined by `TOUCH_PIN_1` in `config.h`.
   *Important Note:* The default definitions for `TOUCH_PIN_0` and `TOUCH_PIN_1` in `config.h` might be `0` and `1`. These often correspond to Serial TX/RX pins on many Arduino boards (including some Teensy pins that might be default Serial1). You **must** reassign these constants in `config.h` to other available touch-capable digital pins on your Teensy 4.1 if you are using pins 0 and 1 for Serial communication. Refer to the Teensy 4.1 pinout diagram for touch-enabled pins (e.g., 0, 1, 15, 16, 17, 18, 19, 22, 23, etc., but check which are truly free in your setup).

**2. MPR121 Capacitive Touch Sensor Breakouts (x3):**
   - All three MPR121 boards connect to the Teensy's primary I2C bus.
   - **I2C Bus (Wire):**
      - SDA: Teensy Pin 18 (SDA0)
      - SCL: Teensy Pin 19 (SCL0)
      (These are the default I2C pins for Teensy 4.1, `Wire`)
   - **I2C Addresses:** Each MPR121 breakout must be configured to a unique I2C address. This is typically done by connecting the ADDR pin on the breakout to VCC, GND, SDA, or SCL.
      - MPR121 Chip 1: Use address defined by `MPR121_ADDR_1` in `config.h` (e.g., 0x5A).
      - MPR121 Chip 2: Use address defined by `MPR121_ADDR_2` in `config.h` (e.g., 0x5B).
      - MPR121 Chip 3: Use address defined by `MPR121_ADDR_3` in `config.h` (e.g., 0x5C).
   - **I2C Pull-up Resistors:** Ensure appropriate pull-up resistors (e.g., 2.2kOhms to 4.7kOhms) are present on both SDA and SCL lines. Some MPR121 breakout boards include these. If multiple devices are on the bus, usually one set of pull-ups is sufficient.

**3. Analog Sensors:**
   *(All analog sensors connect their signal output to a specific Teensy analog input pin. They also require power and GND connections as per their specifications.)*
   - **Breath Sensor Signal:** Connect to Teensy pin defined by `BREATH_PIN` in `config.h`. (Remember voltage divider if sensor output exceeds 3.3V).
   - **Pitch Bend Sensor 1 (e.g., Down):** Connect to Teensy pin defined by `PITCH_BEND_PIN_1` in `config.h`.
   - **Pitch Bend Sensor 2 (e.g., Up):** Connect to Teensy pin defined by `PITCH_BEND_PIN_2` in `config.h`.
   - **Vibrato/Portamento Lever:** Connect to Teensy pin defined by `VIBRATO_LEVER_PIN` in `config.h`.
   - **Bite Sensor:** Connect to Teensy pin defined by `BITE_SENSOR_PIN` in `config.h`.
   - **Lip Sensor:** Connect to Teensy pin defined by `LIP_SENSOR_PIN` in `config.h`.

## Features
- Multiple Fingering Modes (currently SAX, EWI, EWX, EVI, EVR, and a TRUMPET_DEV mode)
- Breath-controlled note dynamics:
    - Note On/Off triggered by breath pressure.
    - Continuous Breath Control (MIDI CC2).
    - Channel Aftertouch derived from breath pressure.
- Pitch Bend via dedicated analog sensors.
- Vibrato via Bite Sensor (MIDI CC1 - Modulation).
- Lip Sensor (MIDI CC16 - General Purpose Controller).
- Selectable Polyphony/Harmony Modes (Basic harmonies like Major Root, Major 3rd Down, Minor Harmony implemented).
- User-configurable Octave Shift (-2 to +2 octaves).
- User-configurable MIDI Channel (1-16).
- Throttled Breath CC updates (every 20ms) to optimize MIDI traffic.
- Persistent settings storage via EEPROM (MIDI channel, thresholds, modes, etc.)
- Serial command interface for real-time configuration and EEPROM management.

## Compilation and Upload
1.  **Install Arduino IDE:** Download from [arduino.cc](https://www.arduino.cc).
2.  **Install Teensyduino:** Download and install the Teensyduino add-on from [PJRC](https://www.pjrc.com/teensy/td_download.html). This will add Teensy board support to the Arduino IDE.
3.  **Required Libraries:**
    - `Wire.h`: Standard Arduino library for I2C communication (used by MPR121). Usually included with Arduino.
    - `Adafruit_MPR121.h`: Adafruit's library for the MPR121 sensor. Installable via the Arduino Library Manager (search for "Adafruit MPR121").
    - `usb_midi.h`: Part of the Teensyduino core libraries, provides USB MIDI functionality. No separate installation needed if Teensyduino is set up correctly.
4.  **Open Project:** Open `main.cpp` (or if you structure it as a sketch, your `.ino` file) in the Arduino IDE.
5.  **Board Selection:** In the Arduino IDE, go to `Tools > Board` and select your specific Teensy model (e.g., "Teensy 4.1").
6.  **USB Type:** Go to `Tools > USB Type` and select "MIDI" or "Serial + MIDI". "Serial + MIDI" is recommended during development for debugging output.
7.  **Compile and Upload:** Click the "Verify" (compile) button, then "Upload" to send the firmware to the Teensy.

## Usage

### Control Keys
Specific touch sensors are designated as control keys. These are mapped to channels on the third MPR121 sensor (`cap3`) as defined by `KEY_BIT_*` constants in `config.h`. A short press (touch and release) activates the function.
- **Octave Up/Down:** Changes the current octave (`KEY_BIT_OCTAVE_UP`, `KEY_BIT_OCTAVE_DOWN`).
- **Cycle Fingering Mode:** Cycles through available fingering modes (`KEY_BIT_CYCLE_FINGERING`).
- **Cycle Poly/Harmony Mode:** Cycles through `PolySelectMode` options (`KEY_BIT_CYCLE_POLYSELECT`).
- **Cycle MIDI Channel:** Cycles through MIDI channels 1-16 (`KEY_BIT_CYCLE_MIDI_CHANNEL`).
Changes are typically printed to the Serial Monitor for confirmation.

### Sensors
- **Keys:** Capacitive touch keys (Teensy built-in pins `TOUCH_PIN_0`, `TOUCH_PIN_1`, plus 3x MPR121s) determine the fingered note combination (`currentKeyStates`).
- **Breath Sensor (`BREATH_PIN`):** Controls note start/stop, MIDI CC2 (Breath Controller), and Channel Aftertouch. Notes only sound when breath pressure exceeds `BREATH_NOTE_ON_THRESHOLD`.
- **Pitch Bend Sensors (`PITCH_BEND_PIN_1`, `PITCH_BEND_PIN_2`):** Two proportional sensors control pitch bend up and down.
- **Bite Sensor (`BITE_SENSOR_PIN`):** Modulates vibrato depth (sends MIDI CC1 - Modulation).
- **Lip Sensor (`LIP_SENSOR_PIN`):** Sends MIDI CC16 (General Purpose Controller).
- **Vibrato/Portamento Lever (`VIBRATO_LEVER_PIN`):** Currently read by the firmware, and its value is printed to the Serial Monitor for debugging. It is not yet mapped to a specific MIDI message.

## Configuration

### Key Settings in `config.h`
Many operational parameters can be tuned by editing `config.h` before compiling and uploading the firmware:
- **Pin Definitions (`*_PIN`):** Ensure these match your physical wiring to the Teensy.
- **MPR121 Addresses (`MPR121_ADDR_*`):** Set the correct I2C addresses for your MPR121 chips. These are typically set by hardware jumpers or traces on the MPR121 boards.
- **Sensor Thresholds:**
    - `TOUCH_THRESHOLD`: For Teensy's built-in capacitive touch pins.
    - `BREATH_NOTE_ON_THRESHOLD`: Minimum breath pressure (0-1023 range from ADC) to trigger a note.
- **MIDI Parameters:**
    - `DEFAULT_NOTE_VELOCITY`: Velocity for new Note On messages.
    - `CC_BREATH_INTERVAL`: Interval in milliseconds for sending Breath CC messages.
    - `MIDI_CC_*` constants: Define the CC numbers used for breath, vibrato, lip sensor, etc.
- **Operational Limits:**
    - `MAX_OCTAVE_SHIFT`, `MIN_OCTAVE_SHIFT`: Sets the range for octave changes.
    - `MAX_HARMONY_NOTES`: Maximum number of additional harmony notes.
- **Control Key Bitmasks (`KEY_BIT_*`):** Define which bits in `currentKeyStates` (corresponding to specific sensor inputs, likely on MPR121 #3) trigger UI functions. `CONTROL_KEY_MASK` combines these.

### Fingering Charts in `main.cpp`
The core of the instrument's note production lies in the `NoteMapping` arrays (e.g., `saxFingerings[]`, `ewiFingerings[]`) within `main.cpp`.
- Each array defines the fingerings for a specific mode.
- An entry consists of a `keyState` (a `uint32_t` bitmask representing all active touch sensors) and the corresponding `midiNote` number (0-127).
- **These charts are currently placeholders.** You **MUST** populate them with comprehensive and accurate fingering data for your desired instrument behavior. The `keyState` bitmask needs to carefully match how `currentKeyStates` is assembled in `readBuiltInTouchSensors()` and `readMPR121Sensors()`. Refer to comments in `config.h` and `main.cpp` near `currentKeyStates` and sensor reading functions for bit allocation details.
- The last entry in each chart must be `{0xFFFFFFFF, -1}` to act as a sentinel.

### Serial Command Interface & EEPROM Settings
The EVI firmware includes a serial command interface allowing you to view and modify settings in real-time and save them permanently to EEPROM. This is useful for fine-tuning without recompiling.

**How to Connect:**
- Use the Arduino IDE's Serial Monitor (or any other serial terminal program).
- Baud Rate: 115200.
- Ensure line ending is set to Newline or Carriage Return in the Serial Monitor.

**Available Commands:**
*(Commands are generally case-insensitive. Setting names for GET/SET are case-insensitive. String values for SET (like mode names) should match the defined names, as processed by the firmware's parser.)*

- `HELP`
  - Displays a list of available commands and configurable setting names with their expected value types/ranges.
- `LIST_SETTINGS`
  - Shows the current values of all configurable settings active in the EVI's RAM.
- `GET <setting_name>`
  - Retrieves and displays the current value of a specific setting.
  - Example: `GET MIDI_CHANNEL`
- `SET <setting_name> <value>`
  - Sets a new value for a specific setting in RAM. These changes take effect immediately. To make them permanent across reboots, you must use `SAVE_CONFIG`.
  - Example: `SET MIDI_CHANNEL 5`, `SET OCTAVE_SHIFT -1`, `SET FINGERING_MODE SAX`
- `SAVE_CONFIG`
  - Saves all current RAM settings (reflecting any changes made by `SET` commands or physical UI controls) to the Teensy's EEPROM. These settings will then be loaded automatically on the next startup.
- `LOAD_CONFIG`
  - Loads all settings from EEPROM into RAM, overwriting any current non-saved changes. The EVI will immediately start using these loaded settings. It's useful to revert to the last saved state.

**Configurable Settings via Serial (`<setting_name>` for GET/SET):**
*(Refer to the output of the `HELP` command for the definitive list from the firmware.)*
- `MIDI_CHANNEL`: (1-16)
- `OCTAVE_SHIFT`: (e.g., -2 to 2, as defined by `MIN_OCTAVE_SHIFT`/`MAX_OCTAVE_SHIFT` in `config.h`)
- `TRANSPOSE_SHIFT`: (e.g., -12 to 12)
- `FINGERING_MODE`: (e.g., `EWI`, `SAX`, `EVR`, etc. - specific names as per firmware)
- `POLYSELECT_MODE`: (e.g., `OFF`, `MGR`, etc. - specific names as per firmware)
- `BREATH_THRESHOLD`: (0-1023, affects note-on sensitivity)
- `BREATH_INTERVAL`: (milliseconds, interval for sending breath CC messages)

**EEPROM Storage:**
- Key operational settings (listed above) are automatically loaded from EEPROM when the EVI starts up.
- If no valid settings are found in EEPROM (e.g., on first use after flashing, or if EEPROM data is corrupted based on `EVI_CONFIG_VERSION`), a set of default values is loaded.
- Changes made using physical UI controls (like octave buttons) or `SET` commands only affect the current session's settings in RAM. Use the `SAVE_CONFIG` serial command to make these changes persistent.
Note: The `config.h` file still defines hardware pinouts and fundamental constants not meant for EEPROM configuration. Settings like `BREATH_NOTE_ON_THRESHOLD` and `CC_BREATH_INTERVAL` now have their live values managed by variables loaded from EEPROM, though their initial default values are set in `getDefaultConfiguration()` in `main.cpp`.

## MIDI Implementation Details
- **MIDI Channel:** Default is 1, (changeable via UI control key or Serial `SET MIDI_CHANNEL` command, loaded from EEPROM).
- **Note On/Off:** Standard messages. Velocity for Note On is fixed by `DEFAULT_NOTE_VELOCITY`.
- **Channel Aftertouch:** Sent on the active MIDI channel, value derived from breath pressure.
- **Control Changes (CCs):**
    - CC2: Breath Controller (from breath sensor).
    - CC1: Modulation (used for vibrato depth, from bite sensor).
    - CC16: General Purpose Controller (from lip sensor).
- **Pitch Bend:** Standard 14-bit pitch bend messages.

## Debugging
Connect to the Teensy via the Arduino IDE's Serial Monitor (set baud rate to 115200). The firmware prints detailed information each loop cycle, including:
- Combined key states (`Keys:0x...`).
- Current octave, fingering mode, MIDI channel, selected polyphony mode.
- Calculated base MIDI note and any harmony notes.
- Note On/Off/Legato/Sustain/Idle state indication.
- Raw and processed values for breath, pitch bend, bite, and lip sensors.
- Aftertouch value when notes are playing.
This output is crucial for verifying sensor connections, calibrating thresholds, understanding the note logic, and debugging fingering charts.
