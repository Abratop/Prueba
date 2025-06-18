// main.cpp - Firmware for Teensy EVI MIDI Controller
// This firmware reads various analog and digital sensors (including capacitive touch keys via MPR121)
// to generate MIDI messages, simulating an Electronic Valve/Wind Instrument.
// It supports multiple fingering modes, octave control, harmony, and various MIDI CC outputs.

// main.cpp - Firmware for Teensy EVI MIDI Controller
// This firmware reads various analog and digital sensors (including capacitive touch keys via MPR121)
// to generate MIDI messages, simulating an Electronic Valve/Wind Instrument.
// It supports multiple fingering modes, octave control, harmony, and various MIDI CC outputs.

#include <Arduino.h>
#include "config.h"          // Main configuration parameters, pin definitions, enums
#include <Wire.h>            // For I2C communication with MPR121 sensors
#include <usb_midi.h>        // Standard Teensy MIDI library for USB MIDI functionality
#include <Adafruit_MPR121.h> // Library for MPR121 Capacitive Touch Sensors
#include <EEPROM.h>          // For persistent storage of configuration settings (Teensy EEPROM emulation)

// --- Global Variables ---

// Stores the combined state of all touch sensors (built-in and MPR121s) as a bitmask.
// Each bit corresponds to a specific key/sensor. This is used to look up fingerings.
uint32_t currentKeyStates = 0;

// --- Configuration Data Structures ---
EVIConfig activeConfig; // Holds the currently active configuration, loaded from EEPROM or defaults.

// --- Global Configuration Variables (Live Operational Values) ---
// These variables store the current operational state of the instrument, loaded from `activeConfig`.
// Their extern declarations are in config.h.
FingeringMode currentFingeringMode;
PolySelectMode currentPolySelectMode;
int currentHarmonyNotes[MAX_HARMONY_NOTES]; // Array to store calculated harmony notes.
int numHarmonyNotes = 0;                    // Number of harmony notes currently active.
uint8_t currentMidiChannel;             // Current MIDI channel for sending messages (1-16).
uint16_t currentBreathNoteOnThreshold;   // Active breath threshold for Note On.
uint16_t currentCCBreathIntervalMs;      // Active interval for Breath CC.

// --- MIDI Timing Variables ---
unsigned long lastBreathCCSendTime = 0; // Timestamp of the last Breath CC message sent. Used for interval control.

// --- Serial Command Processing Variables & Functions ---
const int SERIAL_CMD_BUFFER_SIZE = 64; // Maximum length for incoming serial commands.
char serialCmdBuffer[SERIAL_CMD_BUFFER_SIZE]; // Buffer to store characters from serial input.
uint8_t serialCmdBufferPos = 0;          // Current position in serialCmdBuffer.

// Helper to convert FingeringMode enum to a user-readable string.
// Used for serial output (LIST_SETTINGS, GET FINGERING_MODE).
String fingeringModeToString(FingeringMode mode) {
    switch (mode) {
        case FingeringMode::EWI: return "EWI";
        case FingeringMode::EWX: return "EWX";
        case FingeringMode::SAX: return "SAX";
        case FingeringMode::EVI: return "EVI";
        case FingeringMode::EVR: return "EVR";
        case FingeringMode::TRUMPET_DEV: return "TRUMPET_DEV";
        default: return "UNKNOWN";
    }
}

// Helper to parse a string (from serial input) to a FingeringMode enum value.
// Used by SET FINGERING_MODE command. Case-insensitive.
// Returns true if parsing is successful, false otherwise.
bool stringToFingeringMode(String s, FingeringMode& mode) {
    s.toUpperCase();
    if (s == "EWI") { mode = FingeringMode::EWI; return true; }
    if (s == "EWX") { mode = FingeringMode::EWX; return true; }
    if (s == "SAX") { mode = FingeringMode::SAX; return true; }
    if (s == "EVI") { mode = FingeringMode::EVI; return true; }
    if (s == "EVR") { mode = FingeringMode::EVR; return true; }
    if (s == "TRUMPET_DEV") { mode = FingeringMode::TRUMPET_DEV; return true; }
    return false;
}

// Helper to convert PolySelectMode enum to a user-readable string.
// Used for serial output (LIST_SETTINGS, GET POLYSELECT_MODE).
String polySelectModeToString(PolySelectMode mode) {
    switch (mode) {
        case PolySelectMode::OFF: return "OFF";
        case PolySelectMode::MGR: return "MGR";
        case PolySelectMode::MGD: return "MGD";
        case PolySelectMode::MND: return "MND";
        case PolySelectMode::MNH: return "MNH";
        case PolySelectMode::FWC: return "FWC";
        case PolySelectMode::RTA: return "RTA";
        case PolySelectMode::RTB: return "RTB";
        case PolySelectMode::RTC: return "RTC";
        default: return "UNKNOWN";
    }
}

// Helper to parse a string (from serial input) to a PolySelectMode enum value.
// Used by SET POLYSELECT_MODE command. Case-insensitive.
// Returns true if parsing is successful, false otherwise.
bool stringToPolySelectMode(String s, PolySelectMode& mode) {
    s.toUpperCase();
    if (s == "OFF") { mode = PolySelectMode::OFF; return true; }
    if (s == "MGR") { mode = PolySelectMode::MGR; return true; }
    if (s == "MGD") { mode = PolySelectMode::MGD; return true; }
    if (s == "MND") { mode = PolySelectMode::MND; return true; }
    if (s == "MNH") { mode = PolySelectMode::MNH; return true; }
    if (s == "FWC") { mode = PolySelectMode::FWC; return true; }
    if (s == "RTA") { mode = PolySelectMode::RTA; return true; }
    if (s == "RTB") { mode = PolySelectMode::RTB; return true; }
    if (s == "RTC") { mode = PolySelectMode::RTC; return true; }
    return false;
}

// Prints the help message listing available serial commands and settings to the Serial Monitor.
void printHelp() {
    Serial.println(F("Available commands:"));
    Serial.println(F("  HELP                 - Displays this help message."));
    Serial.println(F("  LIST_SETTINGS        - Shows current values of all settings."));
    Serial.println(F("  GET <setting_name>   - Retrieves the value of a specific setting."));
    Serial.println(F("  SET <setting_name> <value> - Sets a new value for a setting (RAM only)."));
    Serial.println(F("  SAVE_CONFIG          - Saves current settings from RAM to EEPROM."));
    Serial.println(F("  LOAD_CONFIG          - Loads settings from EEPROM into RAM and applies them."));
    Serial.println(F("Valid <setting_name> arguments for GET/SET:"));
    Serial.println(F("  MIDI_CHANNEL      (Value: 1-16)"));
    Serial.println(F("  OCTAVE_SHIFT      (Value: -2 to 2, or as per MIN/MAX_OCTAVE_SHIFT)"));
    Serial.println(F("  TRANSPOSE_SHIFT   (Value: -12 to 12)"));
    Serial.println(F("  FINGERING_MODE    (Value: EWI, EWX, SAX, EVI, EVR, TRUMPET_DEV)"));
    Serial.println(F("  POLYSELECT_MODE   (Value: OFF, MGR, MGD, MND, MNH, FWC, RTA, RTB, RTC)"));
    Serial.println(F("  BREATH_THRESHOLD  (Value: 0-1023)"));
    Serial.println(F("  BREATH_INTERVAL   (Value: ms, e.g., 10-100)"));
}

// Prints the current values of all configurable settings to the Serial Monitor.
void listSettings() {
    Serial.println(F("--- Current EVI Settings ---"));
    Serial.print(F("  MIDI Channel:       ")); Serial.println(currentMidiChannel);
    Serial.print(F("  Octave Shift:       ")); Serial.println(currentOctave);
    Serial.print(F("  Transpose Shift:    ")); Serial.println(currentTranspose);
    Serial.print(F("  Fingering Mode:     ")); Serial.println(fingeringModeToString(currentFingeringMode));
    Serial.print(F("  PolySelect Mode:    ")); Serial.println(polySelectModeToString(currentPolySelectMode));
    Serial.print(F("  Breath Threshold:   ")); Serial.println(currentBreathNoteOnThreshold);
    Serial.print(F("  Breath CC Interval: ")); Serial.print(currentCCBreathIntervalMs); Serial.println(F(" ms"));
    Serial.println(F("--------------------------"));
}

// --- MIDI State Variables ---
// These track the state of MIDI notes being played, necessary for correct Note On/Off and legato logic.

// Parses and executes a command string received via Serial.
// Parameters:
//   cmd: A null-terminated C-string containing the command and its arguments.
void executeSerialCommand(char* cmd) {
    char* token; // Pointer for strtok_r or strtok to parse command parts.
    token = strtok(cmd, " "); // Get the first part (the command itself).

    if (token == NULL) return;

    String commandStr = String(token);
    commandStr.toUpperCase(); // Case-insensitive commands

    if (commandStr == "HELP") {
        printHelp();
    } else if (commandStr == "LIST_SETTINGS") {
        listSettings();
    } else if (commandStr == "SAVE_CONFIG") {
        saveConfiguration();
        Serial.println(F("OK: Configuration saved to EEPROM."));
    } else if (commandStr == "LOAD_CONFIG") {
        loadConfiguration();
        Serial.println(F("OK: Configuration loaded from EEPROM and applied."));
        listSettings(); // Show new settings
    } else if (commandStr == "GET") {
        token = strtok(NULL, " "); // Get the setting name argument.
        if (token == NULL) { Serial.println(F("ERROR: Missing setting name for GET command. Use 'HELP' for options.")); return; }
        String settingName = String(token);
        settingName.toUpperCase(); // Standardize setting name to uppercase for comparison.

        // Respond with the value of the requested setting.
        if (settingName == "MIDI_CHANNEL") Serial.println(currentMidiChannel);
        else if (settingName == "OCTAVE_SHIFT") Serial.println(currentOctave);
        else if (settingName == "TRANSPOSE_SHIFT") Serial.println(currentTranspose);
        else if (settingName == "FINGERING_MODE") Serial.println(fingeringModeToString(currentFingeringMode));
        else if (settingName == "POLYSELECT_MODE") Serial.println(polySelectModeToString(currentPolySelectMode));
        else if (settingName == "BREATH_THRESHOLD") Serial.println(currentBreathNoteOnThreshold);
        else if (settingName == "BREATH_INTERVAL") Serial.println(currentCCBreathIntervalMs);
        else { Serial.print(F("ERROR: Unknown setting '")); Serial.print(settingName); Serial.println(F("' for GET."));}

    } else if (commandStr == "SET") {
        char* settingNameToken = strtok(NULL, " "); // Get the setting name argument.
        char* valueToken = strtok(NULL, " ");       // Get the value argument.
                                                    // Note: strtok will modify the command buffer by inserting nulls.
                                                    // This simple parsing assumes values are single tokens without spaces.
        if (settingNameToken == NULL || valueToken == NULL) { Serial.println(F("ERROR: Missing setting name or value for SET. Use 'SET <setting> <value>'.")); return; }

        String settingName = String(settingNameToken);
        settingName.toUpperCase(); // Standardize for comparison.
        long valueInt = atol(valueToken);    // Convert value part to long for numerical settings.
        String valueStr = String(valueToken); // Keep string form for enum parsing.

        bool success = false; // Flag to indicate if SET operation was valid.

        if (settingName == "MIDI_CHANNEL") {
            if (valueInt >= 1 && valueInt <= 16) { currentMidiChannel = (uint8_t)valueInt; success = true; }
            else { Serial.println(F("ERROR: MIDI_CHANNEL value must be between 1 and 16.")); }
        } else if (settingName == "OCTAVE_SHIFT") {
            if (valueInt >= MIN_OCTAVE_SHIFT && valueInt <= MAX_OCTAVE_SHIFT) { currentOctave = (int8_t)valueInt; success = true; }
            else { Serial.print(F("ERROR: OCTAVE_SHIFT value out of range (Min:")); Serial.print(MIN_OCTAVE_SHIFT); Serial.print(F(", Max:")); Serial.print(MAX_OCTAVE_SHIFT); Serial.println(F(").")); }
        } else if (settingName == "TRANSPOSE_SHIFT") {
            // Assuming a practical range for transpose, e.g., -24 to +24 semitones.
            // This should ideally use MIN/MAX constants from config.h if defined there.
            if (valueInt >= -24 && valueInt <= 24) { currentTranspose = (int8_t)valueInt; success = true; }
            else { Serial.println(F("ERROR: TRANSPOSE_SHIFT value out of range (e.g., -24 to 24).")); }
        } else if (settingName == "FINGERING_MODE") {
            FingeringMode tempMode;
            if (stringToFingeringMode(valueStr, tempMode)) { currentFingeringMode = tempMode; success = true; }
            else { Serial.print(F("ERROR: Invalid value '"));Serial.print(valueStr);Serial.println(F("' for FINGERING_MODE. Use HELP for options.")); }
        } else if (settingName == "POLYSELECT_MODE") {
            PolySelectMode tempMode;
            if (stringToPolySelectMode(valueStr, tempMode)) { currentPolySelectMode = tempMode; success = true; }
            else { Serial.print(F("ERROR: Invalid value '"));Serial.print(valueStr);Serial.println(F("' for POLYSELECT_MODE. Use HELP for options.")); }
        } else if (settingName == "BREATH_THRESHOLD") {
            if (valueInt >= 0 && valueInt <= 1023) { currentBreathNoteOnThreshold = (uint16_t)valueInt; success = true; }
            else { Serial.println(F("ERROR: BREATH_THRESHOLD value must be between 0 and 1023.")); }
        } else if (settingName == "BREATH_INTERVAL") {
            if (valueInt >= 5 && valueInt <= 1000) { currentCCBreathIntervalMs = (uint16_t)valueInt; success = true; } // Example valid range.
            else { Serial.println(F("ERROR: BREATH_INTERVAL value out of range (e.g., 5-1000 ms).")); }
        } else {
            Serial.print(F("ERROR: Unknown setting '"));Serial.print(settingName);Serial.println(F("' for SET."));
        }

        if(success) { // If the setting was successfully updated.
            Serial.println(F("OK"));
            // Optional: To show immediate effect or for confirmation, list the specific setting changed or all settings.
            // listSettings();
            // Note: Changes are in RAM. User must call SAVE_CONFIG to make them persistent.
        }

    } else {
        Serial.print(F("ERROR: Unknown command '")); Serial.print(commandStr); Serial.println(F("'. Type 'HELP' for options."));
    }
}

// Reads characters from the Serial input until a newline or carriage return is encountered,
// then processes the assembled command string.
void processSerialCommands() {
    while (Serial.available() > 0) { // While there are characters to read from serial...
        char incomingChar = Serial.read(); // Read one character.

        if (incomingChar == '\n' || incomingChar == '\r') { // If newline or carriage return (command delimiter)...
            if (serialCmdBufferPos > 0) { // And if the buffer has content...
                serialCmdBuffer[serialCmdBufferPos] = '\0'; // Null-terminate the string.
                Serial.print(F("> ")); Serial.println(serialCmdBuffer); // Echo the received command for user feedback.
                executeSerialCommand(serialCmdBuffer); // Process the command.
                serialCmdBufferPos = 0; // Reset buffer position for the next command.
            }
        } else if (serialCmdBufferPos < SERIAL_CMD_BUFFER_SIZE - 1) { // If it's a regular character and buffer has space...
            if (isprint(incomingChar)) { // Only add printable characters to the buffer.
                 serialCmdBuffer[serialCmdBufferPos++] = incomingChar;
            }
        }
        // Characters are ignored if buffer is full (prevents overflow) or if they are non-printable (except CR/LF).
    }
}

uint32_t previousControlKeyStates = 0; // Stores the state of UI control keys from the previous loop cycle for edge detection.
int previousMidiNote = -1;             // The last primary MIDI note sent (for Note Off and legato).
int previousHarmonyNotes[MAX_HARMONY_NOTES]; // Stores the last set of harmony notes sent.
int numPreviousHarmonyNotes = 0;       // Number of harmony notes active in the previous cycle.
bool noteIsCurrentlyPlaying = false;   // Flag indicating if any note (primary or harmony) is currently sounding.
const int PITCH_BEND_CENTER = 8192;    // MIDI pitch bend center value (14-bit resolution, 0-16383).
const int PITCH_BEND_DEADZONE = 10;    // Deadzone for pitch bend analog sensors to prevent jitter around the center.

// --- Musical Parameters ---
int8_t currentOctave = 0;              // Current octave shift (-MAX_OCTAVE_SHIFT to +MAX_OCTAVE_SHIFT).
int8_t currentTranspose = 0;           // Overall transposition in semitones (not yet UI-adjustable).
bool harmonyModeActive = false;        // Legacy flag, PolySelectMode is now used.
int8_t harmonyInterval = 4;            // Legacy interval, PolySelectMode is now used.

// --- Inline Filter Variables ---
float filteredBreathValue = 0.0f;      // Stores the current filtered value of the breath sensor.
// Smoothing factor for the Exponential Moving Average (EMA) filter applied to the breath sensor.
// Value is between 0.0 and 1.0. Smaller values = more smoothing.
const float BREATH_SMOOTHING_FACTOR = 0.1f;


// --- EEPROM Save/Load Functions ---

// Populates and returns a default EVIConfig structure.
EVIConfig getDefaultConfiguration() {
    EVIConfig defaultConfig;
    defaultConfig.version = EVI_CONFIG_VERSION;
    defaultConfig.midiChannel = 1;
    defaultConfig.octaveShift = 0;
    defaultConfig.transposeShift = 0;
    defaultConfig.fingeringMode = static_cast<uint8_t>(FingeringMode::SAX);
    defaultConfig.polySelectMode = static_cast<uint8_t>(PolySelectMode::OFF);
    defaultConfig.breathNoteOnThreshold = 50; // Default from original const
    defaultConfig.ccBreathIntervalMs = 20;    // Default from original const
    // Initialize any other members added to EVIConfig here
    return defaultConfig;
}

// Loads configuration from EEPROM into activeConfig and applies it to live variables.
// If EEPROM data is invalid or not found, loads default configuration.
void loadConfiguration() {
    EEPROM.get(0, activeConfig); // Read the entire struct from EEPROM address 0

    if (activeConfig.version != EVI_CONFIG_VERSION) {
        Serial.println("EEPROM: Invalid config version or no config found. Loading defaults.");
        activeConfig = getDefaultConfiguration();
        // Optionally, save these defaults back to EEPROM immediately:
        // EEPROM.put(0, activeConfig);
        // Serial.println("EEPROM: Default configuration saved to EEPROM.");
    } else {
        Serial.println("EEPROM: Configuration loaded successfully.");
    }

    // Apply loaded/default settings from activeConfig to live operational variables
    currentMidiChannel = activeConfig.midiChannel;
    currentOctave = activeConfig.octaveShift;
    currentTranspose = activeConfig.transposeShift;
    currentFingeringMode = static_cast<FingeringMode>(activeConfig.fingeringMode);
    currentPolySelectMode = static_cast<PolySelectMode>(activeConfig.polySelectMode);
    currentBreathNoteOnThreshold = activeConfig.breathNoteOnThreshold;
    currentCCBreathIntervalMs = activeConfig.ccBreathIntervalMs;
    // Apply any other settings from activeConfig to their live counterparts
}

// Saves the current live operational settings to EEPROM.
void saveConfiguration() {
    // Before saving, ensure activeConfig reflects the current live operational settings
    activeConfig.version = EVI_CONFIG_VERSION; // Ensure version is always current
    activeConfig.midiChannel = currentMidiChannel;
    activeConfig.octaveShift = currentOctave;
    activeConfig.transposeShift = currentTranspose;
    activeConfig.fingeringMode = static_cast<uint8_t>(currentFingeringMode);
    activeConfig.polySelectMode = static_cast<uint8_t>(currentPolySelectMode);
    activeConfig.breathNoteOnThreshold = currentBreathNoteOnThreshold;
    activeConfig.ccBreathIntervalMs = currentCCBreathIntervalMs;
    // Update any other fields in activeConfig from their live counterparts

    EEPROM.put(0, activeConfig);
    Serial.println("EEPROM: Configuration saved.");
}


// --- Fingering Chart Data (Definitions) ---
// These arrays map `currentKeyStates` (bitmasks) to MIDI note numbers for different fingering modes.
// Each entry is: {keyState_bitmask, midiNote_number}
// A midiNote_number of -1 acts as a sentinel to mark the end of the array.
// IMPORTANT: These are currently placeholders and MUST be populated with comprehensive, accurate data
// for the instrument to play correctly in each mode. The bitmasks must align with the physical
// sensor setup and the bit allocation defined in config.h and implemented in the sensor reading functions.
// Bit allocation assumed for these examples (total 30 bits used):
// Bits 0-1:   Teensy built-in TOUCH_PIN_0, TOUCH_PIN_1.
// Bits 2-13:  MPR121 #1 (cap1) channels 0-11.
// Bits 14-25: MPR121 #2 (cap2) channels 0-11.
// Bits 26-29: MPR121 #3 (cap3) channels 0-3 (used for UI control keys: Octave Up/Down, Fingering Cycle, PolySelect Cycle).
// Bit 30:     MPR121 #3 (cap3) channel 4 (used for UI control key: MIDI Channel Cycle).

const NoteMapping saxFingerings[] = {
    // {keyState bitmask, midiNote} - Example for SAX mode
    {0b0000000000000001, 60}, // Example: Built-in Touch 0 -> C4
    {0b0000000000000010, 62}, // Built-in Touch 1 -> D4
    {0b0000000000000100, 64}, // MPR121_1_CH0 (bit 2) -> E4
    {0b0000000000001000, 65}, // MPR121_1_CH1 (bit 3) -> F4
    {0b0000000000000011, 58}, // Built-in Touch 0 + 1 -> Bb3 (combo example)
    {0b0000000000010100, 67}, // MPR121_1_CH0 + MPR121_1_CH2 (bit 4) -> G4
    // ... many more entries needed
    {0xFFFFFFFF, -1} // Sentinel value: Invalid keyState, invalid MIDI note
};

const NoteMapping trumpetFingerings_DEV[] = { // Renamed from trumpetFingerings
    // Assuming trumpet valves map to lower bits of an MPR121 or built-in pins
    // Example: bit 2 = Valve 1, bit 3 = Valve 2, bit 4 = Valve 3
    // These are relative to a fundamental that might change with lip pressure (not implemented yet)
    {0b0000000000000000, 60}, // No valves (open) -> C4 (example for one harmonic series)
    {0b0000000000000100, 58}, // Valve 1 (bit 2) -> Bb3 (-2 semitones from C4)
    {0b0000000000001000, 59}, // Valve 2 (bit 3) -> B3  (-1 semitone from C4)
    {0b0000000000001100, 57}, // Valve 1+2 (bits 2+3) -> A3 (-3 semitones from C4)
    {0b0000000000010000, 55}, // Valve 3 (bit 4) -> G3 (-5 semitones from C4)
    {0b0000000000010100, 53}, // Valve 1+3 (bits 2+4) -> F3 (-7 semitones from C4)
    {0b0000000000011000, 54}, // Valve 2+3 (bits 3+4) -> F#3 (-6 semitones from C4)
    {0b0000000000011100, 52}, // Valve 1+2+3 (bits 2+3+4) -> E3 (-8 semitones from C4)
    // ... many more entries
    {0xFFFFFFFF, -1} // Sentinel value
};

const NoteMapping ewiFingerings[] = {
    // {keyState, midiNote}
    // Add specific EWI fingerings here
    {0xFFFFFFFF, -1} // Sentinel
};

const NoteMapping ewxFingerings[] = {
    // {keyState, midiNote}
    // Add specific EWX fingerings here
    {0xFFFFFFFF, -1} // Sentinel
};

const NoteMapping eviFingerings[] = {
    // {keyState, midiNote}
    // Add specific EVI fingerings here
    {0xFFFFFFFF, -1} // Sentinel
};

const NoteMapping evrFingerings[] = {
    // {keyState, midiNote}
    // Add specific EVR fingerings here
    {0xFFFFFFFF, -1} // Sentinel to mark end of array
};

// Placeholder for EWI specific fingerings
const NoteMapping ewiFingerings[] = {
    // {keyState, midiNote}
    {0xFFFFFFFF, -1}
};

// Placeholder for EWX specific fingerings
const NoteMapping ewxFingerings[] = {
    // {keyState, midiNote}
    {0xFFFFFFFF, -1}
};

// Placeholder for EVI specific fingerings
const NoteMapping eviFingerings[] = {
    // {keyState, midiNote}
    {0xFFFFFFFF, -1}
};

// Placeholder for EVR specific fingerings
const NoteMapping evrFingerings[] = {
    // {keyState, midiNote}
    {0xFFFFFFFF, -1}
};


// --- MIDI Note Calculation Function ---
// Determines the primary MIDI note to be played based on the current key states and selected mode.
// - Selects the appropriate fingering chart (e.g., saxFingerings) based on `currentFingeringMode`.
// - Iterates through the chosen chart to find an entry matching the `currentKeyStates` bitmask.
// - If a match is found, the base MIDI note is retrieved from the chart.
// - Applies octave shifts (`currentOctave`) and overall transposition (`currentTranspose`).
// - Clamps the final note to the valid MIDI range (0-127).
// Returns: The calculated MIDI note number (0-127), or -1 if no valid fingering is matched.
int calculateMidiNote() {
    const NoteMapping* currentChart = nullptr; // Pointer to the currently selected fingering chart.
    int baseNote = -1; // The MIDI note derived from the fingering chart, before adjustments.

    // 1. Select the current fingering chart based on the active mode.
    switch (currentFingeringMode) {
        case FingeringMode::SAX:
            currentChart = saxFingerings;
            break;
        case FingeringMode::EWI:
            currentChart = ewiFingerings;
            break;
        case FingeringMode::EWX:
            currentChart = ewxFingerings;
            break;
        case FingeringMode::EVI:
            currentChart = eviFingerings;
            break;
        case FingeringMode::EVR:
            currentChart = evrFingerings;
            break;
        case FingeringMode::TRUMPET_DEV:
            currentChart = trumpetFingerings_DEV;
            break;
        default:
            // Serial.println("Error: No valid fingering chart selected."); // Optional debug
            return -1;
    }

    if (currentChart == nullptr) { // Should not happen if default case is handled
        return -1;
    }

    // 2. Find the matching fingering in the selected chart by comparing keyState.
    for (int i = 0; currentChart[i].midiNote != -1; ++i) { // Iterate until the sentinel entry.
        if (currentChart[i].keyState == currentKeyStates) {
            baseNote = currentChart[i].midiNote;
            break; // Fingering match found.
        }
    }

    // 3. If a base note was found from the chart, apply octave and transposition.
    if (baseNote != -1) {
        baseNote += (currentOctave * 12); // Apply octave shift.
        baseNote += currentTranspose;     // Apply overall transposition.

        // Clamp the final note to the valid MIDI range (0-127).
        if (baseNote < 0) baseNote = 0;
        if (baseNote > 127) baseNote = 127;
    }

    return baseNote; // Return the final calculated MIDI note or -1 if no match.
}

// --- Harmony Note Generation Function ---
// Calculates additional harmony notes based on the `baseNote` and `currentPolySelectMode`.
// Populates the global `currentHarmonyNotes` array and updates `numHarmonyNotes`.
// Parameters:
//   baseNote: The primary MIDI note calculated from the current fingering.
void generateHarmonyNotes(int baseNote) {
    numHarmonyNotes = 0; // Reset the count of harmony notes for this cycle.
    if (baseNote == -1) return; // Cannot generate harmony if there's no valid base note.

    int tempHarmonyNotes[MAX_HARMONY_NOTES]; // Temporary local array to store calculated harmony notes.

    // Generate harmony notes based on the selected PolySelectMode.
    switch (currentPolySelectMode) {
        case PolySelectMode::MGR: // Major Root: Add Major 3rd and Perfect 5th.
            if (numHarmonyNotes < MAX_HARMONY_NOTES) tempHarmonyNotes[numHarmonyNotes++] = baseNote + 4; // Major 3rd interval
            if (numHarmonyNotes < MAX_HARMONY_NOTES) tempHarmonyNotes[numHarmonyNotes++] = baseNote + 7; // Perfect 5th interval
            break;

        case PolySelectMode::MGD: // Major 3rd Down: Add a note a Major 3rd below the base note.
            if (numHarmonyNotes < MAX_HARMONY_NOTES) tempHarmonyNotes[numHarmonyNotes++] = baseNote - 4; // Major 3rd down interval
            break;

        case PolySelectMode::MNH: // Minor Harmony: Add a Minor 3rd above the base note.
            if (numHarmonyNotes < MAX_HARMONY_NOTES) tempHarmonyNotes[numHarmonyNotes++] = baseNote + 3; // Minor 3rd interval
            break;

        // Example for PolySelectMode::MND (Minor Root - Root + Minor 3rd + Perfect 5th)
        // case PolySelectMode::MND:
        //     if (numHarmonyNotes < MAX_HARMONY_NOTES) tempHarmonyNotes[numHarmonyNotes++] = baseNote + 3; // Minor 3rd
        //     if (numHarmonyNotes < MAX_HARMONY_NOTES) tempHarmonyNotes[numHarmonyNotes++] = baseNote + 7; // Perfect 5th
        //     break;

        case PolySelectMode::OFF: // No harmony notes if mode is OFF.
        default: // Also no harmony for any other unhandled modes.
            // numHarmonyNotes remains 0.
            break;
    }

    // Copy valid (0-127) harmony notes from tempHarmonyNotes to the global currentHarmonyNotes array.
    int actualCopied = 0;
    for (int i = 0; i < numHarmonyNotes; ++i) {
        int note = tempHarmonyNotes[i];
        if (note >= 0 && note <= 127) { // Check if the note is within valid MIDI range.
            currentHarmonyNotes[actualCopied++] = note;
        }
    }
    numHarmonyNotes = actualCopied; // Update numHarmonyNotes to reflect only the valid, copied notes.
}


// --- Control Input Processing ---
// Reads dedicated UI control keys (e.g., for octave shifts, mode changes) and updates relevant settings.
// Uses edge detection: a change is processed only when a key is newly pressed.
void processControlInputs() {
    // Isolate the state of only the designated control keys from the overall currentKeyStates.
    uint32_t currentControlOnlyStates = currentKeyStates & CONTROL_KEY_MASK;

    // Detect which control keys have been *newly* pressed in this cycle (went from OFF to ON).
    uint32_t pressedControlKeys = currentControlOnlyStates & ~previousControlKeyStates;

    // Process Octave Up key press.
    if (pressedControlKeys & KEY_BIT_OCTAVE_UP) {
        if (currentOctave < MAX_OCTAVE_SHIFT) {
            currentOctave++;
            Serial.print("UI: Octave Up -> "); Serial.println(currentOctave);
        }
    }

    // Process Octave Down key press.
    if (pressedControlKeys & KEY_BIT_OCTAVE_DOWN) {
        if (currentOctave > MIN_OCTAVE_SHIFT) {
            currentOctave--;
            Serial.print("UI: Octave Down -> "); Serial.println(currentOctave);
        }
    }

    // Process Cycle Fingering Mode key press.
    if (pressedControlKeys & KEY_BIT_CYCLE_FINGERING) {
        int modeVal = static_cast<int>(currentFingeringMode);
        modeVal++;
        // Cycle through FingeringMode enum values, wrapping around at the end.
        // Assumes TRUMPET_DEV is the last 'normal' mode in the enum for cycling.
        if (modeVal > static_cast<int>(FingeringMode::TRUMPET_DEV)) {
            modeVal = static_cast<int>(FingeringMode::EWI); // Wrap to the first mode.
        }
        currentFingeringMode = static_cast<FingeringMode>(modeVal);
        Serial.print("UI: Fingering Mode -> "); Serial.println(modeVal); // Print integer value of enum for now.
    }

    // Process Cycle PolySelect Mode key press.
    if (pressedControlKeys & KEY_BIT_CYCLE_POLYSELECT) {
        int modeVal = static_cast<int>(currentPolySelectMode);
        modeVal++;
        // Cycle through PolySelectMode enum values, wrapping around at the end.
        // Assumes RTC is the last mode in the enum for cycling.
        if (modeVal > static_cast<int>(PolySelectMode::RTC)) {
            modeVal = static_cast<int>(PolySelectMode::OFF); // Wrap to the first mode (OFF).
        }
        currentPolySelectMode = static_cast<PolySelectMode>(modeVal);
        Serial.print("UI: PolySelect Mode -> "); Serial.println(modeVal); // Print integer value.
    }

    // Process Cycle MIDI Channel key press.
    if (pressedControlKeys & KEY_BIT_CYCLE_MIDI_CHANNEL) {
        currentMidiChannel++;
        if (currentMidiChannel > 16) { // MIDI channels are 1-16.
            currentMidiChannel = 1; // Wrap around from 16 back to 1.
        }
        Serial.print("UI: MIDI Channel -> "); Serial.println(currentMidiChannel);
    }

    // Store the current state of control keys for the next loop cycle's edge detection.
    previousControlKeyStates = currentControlOnlyStates;
}


// --- Sensor Reading Functions ---
// These functions are responsible for reading data from all connected hardware sensors.

// Reads Teensy's built-in capacitive touch pins and updates `currentKeyStates`.
// Each built-in touch pin is mapped to a specific bit in `currentKeyStates`.
void readBuiltInTouchSensors() {
  // Example for TOUCH_PIN_0 (assumed to map to bit 0 of currentKeyStates)
  if (touchRead(TOUCH_PIN_0) > TOUCH_THRESHOLD) { // TOUCH_THRESHOLD from config.h
    currentKeyStates |= (1 << 0); // Set bit 0 if touched
  } else {
    currentKeyStates &= ~(1 << 0); // Clear bit 0 if not touched
  }

  // Example for TOUCH_PIN_1 (assumed to map to bit 1 of currentKeyStates)
  if (touchRead(TOUCH_PIN_1) > TOUCH_THRESHOLD) {
    currentKeyStates |= (1 << 1); // Set bit 1 if touched
  } else {
    currentKeyStates &= ~(1 << 1); // Clear bit 1 if not touched
  }
  // Add logic for other built-in touch pins if used, assigning unique bits.
}

// Reads all three MPR121 capacitive touch sensors and updates `currentKeyStates`.
// MPR121_1 (cap1) maps to bits 2-13.
// MPR121_2 (cap2) maps to bits 14-25.
// MPR121_3 (cap3) maps to bits 26 onwards (currently bits 26-30 for control keys).
void readMPR121Sensors() {
  uint16_t touched1 = cap1.touched(); // Returns a 12-bit mask for electrodes 0-11.
  for (int i = 0; i < 12; i++) {      // Iterate through the 12 electrodes of MPR121 #1.
    if ((touched1 >> i) & 1) {        // If the i-th electrode is touched...
      currentKeyStates |= (1 << (i + 2)); // Set the corresponding bit in currentKeyStates (offset by 2 for built-in pins).
    } else {
      currentKeyStates &= ~(1 << (i + 2)); // Clear the bit if not touched.
    }
  }

  uint16_t touched2 = cap2.touched(); // Read MPR121 #2.
  for (int i = 0; i < 12; i++) {      // Iterate through the 12 electrodes of MPR121 #2.
    if ((touched2 >> i) & 1) {
      currentKeyStates |= (1 << (i + 14)); // Set bits 14-25.
    } else {
      currentKeyStates &= ~(1 << (i + 14));
    }
  }

  uint16_t touched3 = cap3.touched(); // Read MPR121 #3 (used for control keys).
  // Iterate through the channels of MPR121_3 that are used for control keys.
  // Currently, bits 26 (CH0) through 30 (CH4) are used. So, 5 channels (0 to 4).
  for (int i = 0; i < 5; i++) { // Iterate for channels 0 through 4 of MPR121_3.
    if ((touched3 >> i) & 1) {
      currentKeyStates |= (1 << (i + 26)); // Base bit offset for MPR121_3 is 26.
    } else {
      currentKeyStates &= ~(1 << (i + 26));
    }
  }
}

// Reads the analog breath sensor, applies an EMA low-pass filter, and returns the filtered value.
// The result is typically in the 0-1023 range of the ADC.
int readBreathSensor() {
  int rawValue = analogRead(BREATH_PIN); // Read raw value from the defined breath sensor pin.

  // Apply simple Exponential Moving Average (EMA) filter to smooth the reading.
  filteredBreathValue = (static_cast<float>(rawValue) * BREATH_SMOOTHING_FACTOR) +
                        (filteredBreathValue * (1.0f - BREATH_SMOOTHING_FACTOR));

  return static_cast<int>(filteredBreathValue); // Return the integer part of the filtered value.
}

// Reads the first pitch bend sensor (e.g., for bending up). Returns raw ADC value (0-1023).
int readPitchBendSensor1() {
  return analogRead(PITCH_BEND_PIN_1);
}

// Reads the second pitch bend sensor (e.g., for bending down). Returns raw ADC value (0-1023).
int readPitchBendSensor2() {
  return analogRead(PITCH_BEND_PIN_2);
}

// Reads the vibrato/portamento lever sensor. Returns raw ADC value (0-1023).
// Note: This sensor's value is read but not currently mapped to a MIDI message in the loop.
int readVibratoLeverSensor() {
  return analogRead(VIBRATO_LEVER_PIN);
}

// Reads the bite sensor (typically for vibrato depth). Returns raw ADC value (0-1023).
int readBiteSensor() {
  return analogRead(BITE_SENSOR_PIN);
}

// Reads the lip sensor. Returns raw ADC value (0-1023).
int readLipSensor() {
  return analogRead(LIP_SENSOR_PIN);
}


// --- MIDI Event Handlers (Callbacks for Incoming MIDI) ---
// These functions are called by the usbMIDI library when MIDI messages are received.
// For this EVI project, which primarily acts as a MIDI sender, these might not be
// strictly necessary unless MIDI Thru functionality or external control is desired.
// They are included as good practice for a complete MIDI device.

// Called when a MIDI Note On message is received.
void OnNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  Serial.print("Incoming MIDI Note On: Ch="); // Debug print for received message.
  Serial.print(channel);    // Incoming MIDI channel
  Serial.print(", Note=");  // Incoming MIDI note
  Serial.print(note);
  Serial.print(", Vel=");   // Incoming MIDI velocity
  Serial.println(velocity);
}

// Called when a MIDI Note Off message is received.
void OnNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
  Serial.print("Incoming MIDI Note Off: Ch="); // Debug print.
  Serial.print(channel);
  Serial.print(", Note=");
  Serial.print(note);
  Serial.print(", Vel=");
  Serial.println(velocity);
}

// --- MPR121 Sensor Objects ---
// Instantiate Adafruit_MPR121 objects for each of the three sensors.
Adafruit_MPR121 cap1 = Adafruit_MPR121();
Adafruit_MPR121 cap2 = Adafruit_MPR121();
Adafruit_MPR121 cap3 = Adafruit_MPR121(); // Often used for control keys.

// --- Arduino Setup Function ---
// This function runs once when the Teensy powers up or resets.
// It's used for initializing hardware, communication, and default states.
void setup() {
  // Initialize Serial communication for debugging output via the USB connection.
  Serial.begin(115200); // Set baud rate for the Serial Monitor.
  // Wait for Serial port to connect, with a timeout (useful for some Teensy models).
  while (!Serial && millis() < 5000) {
    ; // Loop until Serial is available or timeout.
  }
  Serial.println("Serial communication initialized.");

  // Load configuration from EEPROM or set defaults
  loadConfiguration();

  // Initialize MIDI State Variables to their default startup values.
  previousMidiNote = -1; // No note previously played.
  for (int i = 0; i < MAX_HARMONY_NOTES; ++i) {
      previousHarmonyNotes[i] = -1; // Clear previous harmony notes.
  }
  numPreviousHarmonyNotes = 0;
  noteIsCurrentlyPlaying = false; // No note is playing at startup.

  // Initialize USB MIDI.
  // The Teensy `usb_midi` library typically makes MIDI available immediately for sending.
  // Callbacks for *incoming* MIDI (like OnNoteOn, OnNoteOff) can be set here if needed:
  // usbMIDI.setHandleNoteOn(OnNoteOn);
  // usbMIDI.setHandleNoteOff(OnNoteOff); // Optional: Attach Note Off handler.
  // Note: For Teensy, MIDI is often available immediately after sketch start for sending.
  // usbMIDI.begin() might not be needed depending on core version.
  Serial.println("USB MIDI interface configured (handlers for incoming MIDI are optional for this send-focused EVI).");

  // Initialize Teensy's built-in capacitive touch pins.
  // Reading them once can help with calibration or setting initial state.
  touchRead(TOUCH_PIN_0); // Initialize/calibrate built-in touch pin 0.
  touchRead(TOUCH_PIN_1); // Initialize/calibrate built-in touch pin 1.
  Serial.println("Built-in Teensy capacitive touch pins initialized.");

  // Initialize the three MPR121 Capacitive Touch Sensor breakout boards.
  Serial.println("Initializing MPR121 capacitive touch sensors...");
  if (!cap1.begin(MPR121_ADDR_1)) { // Attempt to initialize the first MPR121.
    Serial.println("Error: MPR121 #1 not found or failed to initialize. Check wiring, I2C address, and pull-up resistors.");
    while (1); // Halt execution if a critical sensor fails, as fingerings will be incorrect.
  }
  Serial.println("MPR121 #1 initialized successfully.");

  if (!cap2.begin(MPR121_ADDR_2)) { // Initialize the second MPR121.
    Serial.println("Error: MPR121 #2 not found or failed to initialize.");
    // Depending on design, might halt or continue with reduced functionality.
  } else {
    Serial.println("MPR121 #2 initialized successfully.");
  }

  if (!cap3.begin(MPR121_ADDR_3)) { // Initialize the third MPR121 (often for UI controls).
    Serial.println("Error: MPR121 #3 not found or failed to initialize.");
  } else {
    Serial.println("MPR121 #3 initialized successfully.");
  }
  Serial.println("All MPR121 sensor initialization attempts complete.");

  // Set pinMode for all analog sensor inputs. This is good practice, though analogRead works without it.
  pinMode(BREATH_PIN, INPUT);
  Serial.println("Breath sensor pin set to INPUT.");
  pinMode(PITCH_BEND_PIN_1, INPUT);
  pinMode(PITCH_BEND_PIN_2, INPUT);
  Serial.println("Pitch bend sensor pins set to INPUT.");
  pinMode(VIBRATO_LEVER_PIN, INPUT);
  Serial.println("Vibrato/portamento lever pin set to INPUT.");
  pinMode(BITE_SENSOR_PIN, INPUT);
  Serial.println("Bite sensor pin set to INPUT.");
  pinMode(LIP_SENSOR_PIN, INPUT);
  Serial.println("Lip sensor pin set to INPUT.");

  Serial.println("--- EVI Firmware Setup Complete ---");
}

// --- Arduino Loop Function ---
// This function is called repeatedly after `setup()` completes. It forms the main operational cycle of the EVI.
void loop() {
    // ---------- 1. Read Sensor Inputs ----------
    // Gather data from all physical sensors. `currentKeyStates` is updated globally by these functions.
    readBuiltInTouchSensors();
    readMPR121Sensors();

    // Read analog sensors and store their values in local variables for this loop cycle.
    int breathVal = readBreathSensor(); // `readBreathSensor` applies EMA filter internally.
    int pitchBendVal1 = readPitchBendSensor1();
    int pitchBendVal2 = readPitchBendSensor2();
    int biteVal = readBiteSensor();
    int lipVal = readLipSensor();
    int vibratoLeverVal = readVibratoLeverSensor(); // Value read but not currently mapped to MIDI.

    // ---------- 2. Process User Interface Controls ----------
    // Check for presses on dedicated UI keys (e.g., octave up/down, mode changes)
    // and update corresponding settings (currentOctave, currentFingeringMode, etc.).
    processControlInputs();

    // ---------- 3. Musical Calculations ----------
    // Determine the primary MIDI note based on `currentKeyStates` and active settings.
    int currentMidiNote = calculateMidiNote();
    // If a valid primary note is fingered, generate any associated harmony notes.
    if (currentMidiNote != -1) {
        generateHarmonyNotes(currentMidiNote);
    } else {
        numHarmonyNotes = 0; // Ensure no harmony if no base note.
    }

    // ---------- 4. MIDI Message Generation and Sending ----------

    // --- Note On/Off Logic ---
    // This block determines whether to start new notes, stop currently playing notes, or handle legato transitions.
    bool activeFingering = (currentMidiNote != -1); // Is a valid fingering currently active?
    bool breathActive = (breathVal >= currentBreathNoteOnThreshold); // Use loaded/configured threshold

    if (activeFingering && breathActive && !noteIsCurrentlyPlaying) {
        // START NEW NOTES: Conditions met (fingering, breath) and no notes were previously playing.
        usbMIDI.sendNoteOn(currentMidiNote, DEFAULT_NOTE_VELOCITY, currentMidiChannel);
        for (int i = 0; i < numHarmonyNotes; ++i) {
            if (currentHarmonyNotes[i] != -1) {
                usbMIDI.sendNoteOn(currentHarmonyNotes[i], DEFAULT_NOTE_VELOCITY, currentMidiChannel);
            }
        }
        noteIsCurrentlyPlaying = true; // Update state: notes are now playing.
        // Store the currently playing notes to `previous` state variables for the next cycle.
        previousMidiNote = currentMidiNote;
        numPreviousHarmonyNotes = numHarmonyNotes;
        for (int i = 0; i < numHarmonyNotes; ++i) {
            previousHarmonyNotes[i] = currentHarmonyNotes[i];
        }
        for (int i = numHarmonyNotes; i < MAX_HARMONY_NOTES; ++i) {
            previousHarmonyNotes[i] = -1; // Clear unused slots.
        }

    } else if (noteIsCurrentlyPlaying && (!activeFingering || !breathActive)) {
        // STOP NOTES: Notes were playing, but now either the fingering is invalid or breath has dropped.
        if (previousMidiNote != -1) { // Send Note Off for the main previous note.
            usbMIDI.sendNoteOff(previousMidiNote, 0, currentMidiChannel);
        }
        for (int i = 0; i < numPreviousHarmonyNotes; ++i) { // Send Note Off for all previous harmony notes.
            if (previousHarmonyNotes[i] != -1) {
                usbMIDI.sendNoteOff(previousHarmonyNotes[i], 0, currentMidiChannel);
            }
        }
        noteIsCurrentlyPlaying = false; // Update state: no notes are playing.
        previousMidiNote = -1;          // Reset previous note state.
        for (int i = 0; i < MAX_HARMONY_NOTES; ++i) {
            previousHarmonyNotes[i] = -1;
        }
        numPreviousHarmonyNotes = 0;

    } else if (noteIsCurrentlyPlaying && activeFingering && breathActive) {
        // LEGATO CHANGE or NOTE CONTINUATION: Notes are playing, fingering and breath are still active.
        // Check if the new set of notes (primary + harmony) is different from the previous set.
        bool notesChanged = false;
        if (currentMidiNote != previousMidiNote) {
            notesChanged = true;
        } else if (numHarmonyNotes != numPreviousHarmonyNotes) {
            notesChanged = true;
        } else {
            // If primary note and harmony count are the same, check if harmony notes themselves differ.
            // (This is a basic check; a more robust method might sort arrays before comparing).
            for(int i=0; i < numHarmonyNotes; ++i) {
                bool currentNoteFoundInPrevious = false;
                for(int j=0; j < numPreviousHarmonyNotes; ++j) {
                    if(currentHarmonyNotes[i] == previousHarmonyNotes[j]) {
                        currentNoteFoundInPrevious = true;
                        break;
                    }
                }
                if(!currentNoteFoundInPrevious) { notesChanged = true; break; }
            }
            if(!notesChanged) {
               for(int i=0; i < numPreviousHarmonyNotes; ++i) {
                  bool previousNoteFoundInCurrent = false;
                  for(int j=0; j < numHarmonyNotes; ++j) {
                      if(previousHarmonyNotes[i] == currentHarmonyNotes[j]) {
                          previousNoteFoundInCurrent = true;
                          break;
                      }
                  }
                  if(!previousNoteFoundInCurrent) { notesChanged = true; break; }
              }
            }
        }

        if (notesChanged) { // If notes have changed (legato transition).
            // Send Note Off for all previously playing notes.
            if (previousMidiNote != -1) usbMIDI.sendNoteOff(previousMidiNote, 0, currentMidiChannel);
            for (int i = 0; i < numPreviousHarmonyNotes; ++i) {
                 if (previousHarmonyNotes[i] != -1) usbMIDI.sendNoteOff(previousHarmonyNotes[i], 0, currentMidiChannel);
            }
            // Send Note On for the new set of notes.
            if(currentMidiNote != -1) usbMIDI.sendNoteOn(currentMidiNote, DEFAULT_NOTE_VELOCITY, currentMidiChannel);
            for (int i = 0; i < numHarmonyNotes; ++i) {
                if (currentHarmonyNotes[i] != -1) usbMIDI.sendNoteOn(currentHarmonyNotes[i], DEFAULT_NOTE_VELOCITY, currentMidiChannel);
            }
            // Update previous note state to reflect the new notes.
            previousMidiNote = currentMidiNote;
            numPreviousHarmonyNotes = numHarmonyNotes;
            for (int i = 0; i < numHarmonyNotes; ++i) previousHarmonyNotes[i] = currentHarmonyNotes[i];
            for (int i = numHarmonyNotes; i < MAX_HARMONY_NOTES; ++i) previousHarmonyNotes[i] = -1;
        }
    }

  // --- CCs and Pitch Bend ---
  // Send MIDI Control Change (CC) messages and Pitch Bend based on current sensor values.

  // Breath CC (MIDI CC2): Sent at a defined interval (currentCCBreathIntervalMs) to avoid flooding MIDI.
  unsigned long currentTime = millis();
  if (currentTime - lastBreathCCSendTime >= currentCCBreathIntervalMs) { // Use loaded/configured interval
      int breathCCValue = map(breathVal, 0, 1023, 0, 127); // Map ADC range to MIDI CC range (0-127).
      usbMIDI.sendControlChange(MIDI_CC_BREATH, constrain(breathCCValue, 0, 127), currentMidiChannel);
      lastBreathCCSendTime = currentTime; // Record time of this send.
  }

  // Channel Aftertouch: Sent if a note is currently playing, value derived from breath.
  if (noteIsCurrentlyPlaying) {
      int aftertouchValue = map(breathVal, 0, 1023, 0, 127);
      usbMIDI.sendAfterTouch(constrain(aftertouchValue, 0, 127), currentMidiChannel);
  }

  // Pitch Bend: Calculated from two proportional sensors (pitchBendVal1, pitchBendVal2).
  int pitchBendFinal = PITCH_BEND_CENTER; // Default to no bend (center value).
  if (pitchBendVal1 > pitchBendVal2 && pitchBendVal1 > PITCH_BEND_DEADZONE) { // Sensor 1 for bend up.
      pitchBendFinal = map(pitchBendVal1, PITCH_BEND_DEADZONE, 1023, PITCH_BEND_CENTER, 16383);
  } else if (pitchBendVal2 > pitchBendVal1 && pitchBendVal2 > PITCH_BEND_DEADZONE) { // Sensor 2 for bend down.
      pitchBendFinal = map(pitchBendVal2, PITCH_BEND_DEADZONE, 1023, PITCH_BEND_CENTER, 0);
  }
  usbMIDI.sendPitchBend(constrain(pitchBendFinal, 0, 16383), currentMidiChannel); // Send 14-bit pitch bend value.

  // Bite Sensor for Vibrato Depth (MIDI CC1 - Modulation).
  int biteCCValue = map(biteVal, 0, 1023, 0, 127);
  usbMIDI.sendControlChange(MIDI_CC_VIBRATO, constrain(biteCCValue, 0, 127), currentMidiChannel);

  // Lip Sensor (MIDI CC16 - General Purpose Controller 1).
  int lipCCValue = map(lipVal, 0, 1023, 0, 127);
  usbMIDI.sendControlChange(MIDI_CC_LIP_SENSOR, constrain(lipCCValue, 0, 127), currentMidiChannel);

  // ---------- 5. Serial Debugging Output ----------
  // Provides a detailed printout of the EVI's current state and sensor readings to the Serial Monitor.
  // This is invaluable for troubleshooting, calibration, and understanding behavior.
  Serial.print("Loop | Keys:0x"); Serial.print(currentKeyStates, HEX); // Current combined key state.
  Serial.print(" Oct:"); Serial.print(currentOctave);                   // Active octave shift.
  Serial.print(" Mode:"); Serial.print(static_cast<int>(currentFingeringMode)); // Active fingering mode (as int).
  Serial.print(" MIDI Ch:"); Serial.print(currentMidiChannel);          // Current MIDI channel.
  Serial.print(" Note:"); Serial.print(currentMidiNote);                // Calculated primary MIDI note.
  Serial.print(" Poly:"); Serial.print(static_cast<int>(currentPolySelectMode)); // Active polyphony mode (as int).
  Serial.print(" HarmN:"); Serial.print(numHarmonyNotes);              // Number of active harmony notes.
  if (numHarmonyNotes > 0) { // If harmony notes exist, print them.
      Serial.print(" HNotes:");
      for (int i = 0; i < numHarmonyNotes; ++i) {
          Serial.print(currentHarmonyNotes[i]);
          if (i < numHarmonyNotes - 1) Serial.print(" "); // Space separated.
      }
  }
  // Simplified Note State Debugging based on current flags and conditions.
  if (noteIsCurrentlyPlaying) {
    if(activeFingering && breathActive) { // Note is on and conditions remain to keep it on.
        // Check if it's a legato (note changed) or sustained (note same) situation.
        // This simplified harmony check might not perfectly reflect complex legato changes in harmony.
        if(currentMidiNote != previousMidiNote || numHarmonyNotes != numPreviousHarmonyNotes) {
             Serial.print(" -> LEGATO");
        } else {
             Serial.print(" -> SUSTAIN");
        }
    } else {
         // This case implies notes were playing, but conditions (fingering/breath) to keep them on ceased.
         // The main Note Off logic should have already handled this and set noteIsCurrentlyPlaying to false.
         // If this prints, it might indicate a logic discrepancy.
         Serial.print(" -> NOTE OFF (State Error?)");
    }
  } else { // Not currently playing.
      if(activeFingering && breathActive) { // Conditions are met to start a new note.
          Serial.print(" -> NOTE ON");
      } else { // No conditions to start a note
          Serial.print(" -> IDLE");
      }
  }

  Serial.print(" | Breath:"); Serial.print(breathVal); // Raw (filtered) breath sensor value.
  Serial.print(" PB1:"); Serial.print(pitchBendVal1); Serial.print(" PB2:"); Serial.print(pitchBendVal2);
  Serial.print(" (PBVal:"); Serial.print(pitchBendFinal); Serial.print(")"); // Final calculated pitch bend value.
  Serial.print(" Bite:"); Serial.print(biteVal); // Raw bite sensor value.
  if (noteIsCurrentlyPlaying) { Serial.print(" AT:"); Serial.print(map(breathVal, 0, 1023, 0, 127)); } // Current Aftertouch value.
  Serial.print(" Lip:"); Serial.print(lipVal); // Raw lip sensor value.
  Serial.print(" VibrLvr:"); Serial.print(vibratoLeverVal); // Raw vibrato lever (currently unassigned to MIDI).
  Serial.println(); // Newline to complete the debug message for this loop cycle.


  // ---------- 6. MIDI Library Processing & Loop Delay ----------
  // Ensures all queued MIDI messages are actually sent out over USB.
