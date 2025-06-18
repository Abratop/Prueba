#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h> // Required for uint32_t, uint16_t, uint8_t

// --- Pin Definitions ---
// These constants define which Teensy pins are connected to various sensors.
// Analog pins are typically prefixed with 'A' (e.g., A0, A1), but can also be referred to by their digital pin number.
const int BREATH_PIN = A0;            // Analog pin for the breath sensor (e.g., MPX5010DP or MP3V5004GP)
const int PITCH_BEND_PIN_1 = A1;      // Analog pin for the first pitch bend sensor/pad
const int PITCH_BEND_PIN_2 = A2;      // Analog pin for the second pitch bend sensor/pad
const int VIBRATO_LEVER_PIN = A3;     // Analog pin for the vibrato or portamento lever
const int BITE_SENSOR_PIN = A4;       // Analog pin for the bite sensor (modulates vibrato CC)
const int LIP_SENSOR_PIN = A5;        // Analog pin for the lip sensor (sends a dedicated CC)

// --- Built-in Capacitive Touch Pins (Teensy specific) ---
// Define which Teensy pins are used for its native capacitive touch capability.
// These are distinct from the MPR121 touch sensors.
const int TOUCH_PIN_0 = 0;            // Example: Pin 0 on Teensy
const int TOUCH_PIN_1 = 1;            // Example: Pin 1 on Teensy
// Add more built-in touch pins here if used, and ensure they map to unique bits in currentKeyStates.

// --- MPR121 I2C Addresses ---
// Define the I2C addresses for the three MPR121 capacitive touch sensor breakout boards.
// Standard MPR121 addresses are 0x5A, 0x5B, 0x5C, 0x5D (set by connecting ADDR pin).
const uint8_t MPR121_ADDR_1 = 0x5A;   // I2C address for the first MPR121 chip
const uint8_t MPR121_ADDR_2 = 0x5B;   // I2C address for the second MPR121 chip (ensure hardware matches)
const uint8_t MPR121_ADDR_3 = 0x5C;   // I2C address for the third MPR121 chip (ensure hardware matches)

// --- Fingering Modes ---
// Defines the available fingering systems for the instrument.
enum class FingeringMode {
    EWI,        // Akai EWI-style fingering (Boehm-like with rollers)
    EWX,        // User specified
    SAX,        // User specified (was SAXOPHONE)
    EVI,        // User specified
    EVR,        // User specified
    TRUMPET_DEV // Example, can be removed if not needed for dev/testing
};

// PolySelectMode Enum
const int MAX_HARMONY_NOTES = 4; // Maximum number of additional harmony notes

enum class PolySelectMode {
    OFF, // Standard monophonic
    MGR, // Major Root (Harmony/Chord)
    MGD, // Major 3rd Down (Harmony/Chord)
    MND, // Minor Root (Harmony/Chord)
    MNH, // Minor Harmony (Harmony/Chord)
    FWC, // Full Voicing Chord (Harmony/Chord)
    RTA, // Real-Time Assignable A
    RTB, // Real-Time Assignable B
    RTC  // Real-Time Assignable C
};

// Note Representation
struct NoteMapping {
    uint32_t keyState; // Bitmask representing active keys
    int8_t midiNote;   // MIDI note number (or relative interval to a base_note)
};

// Fingering Charts (Placeholder declarations)
extern const NoteMapping saxFingerings[]; // Renamed from saxophoneFingerings
extern const NoteMapping ewiFingerings[];
extern const NoteMapping ewxFingerings[];
extern const NoteMapping eviFingerings[];
extern const NoteMapping evrFingerings[];
extern const NoteMapping trumpetFingerings_DEV[]; // For TRUMPET_DEV mode

// Response Curve Structure
struct ResponseCurvePoint {
    uint16_t input;  // Sensor input value (0-1023 for typical ADC)
    uint16_t output; // Mapped output value (e.g., 0-127 for MIDI CC)
};

// Example: Placeholder for breath curve (actual definition will be in a .cpp file)
// extern const ResponseCurvePoint breathCurve[];

// --- EEPROM Configuration Data Structure ---
// This structure defines how settings are stored in EEPROM for persistence across power cycles.
const uint16_t EVI_CONFIG_VERSION = 0xEV01; // Unique version number for the EVIConfig structure.
                                            // Used to validate EEPROM data on load. Change if struct layout changes significantly.

struct EVIConfig {
    uint16_t version;                 // Stores EVI_CONFIG_VERSION to check data validity upon loading from EEPROM.

    uint8_t  midiChannel;             // Active MIDI Channel (1-16) for sending all MIDI messages.
    int8_t   octaveShift;             // Current octave shift applied to all notes (e.g., -2 to +2 octaves).
    int8_t   transposeShift;          // Overall transposition in semitones (e.g., -12 to +12 semitones).
    uint8_t  fingeringMode;           // Stores the active FingeringMode enum, cast to uint8_t for EEPROM storage.
    uint8_t  polySelectMode;          // Stores the active PolySelectMode enum, cast to uint8_t for EEPROM storage.

    uint16_t breathNoteOnThreshold;   // Breath sensor ADC value (0-1023) that must be exceeded to trigger a Note On event.
    uint16_t ccBreathIntervalMs;      // Minimum interval (in milliseconds) between sending MIDI CC2 (Breath Control) messages.

    // Future expansion examples for other EEPROM-configurable settings:
    // uint16_t touchThreshold;       // Sensitivity for built-in Teensy touch pins.
    // uint8_t  pitchBendDeadzone;    // Deadzone for pitch bend sensors to prevent jitter.
    // float    breathSmoothingFactor;  // If making the EMA filter factor configurable.
};

// --- MIDI & Operational Settings ---
// const int MIDI_CHANNEL = 1; // Removed, use currentMidiChannel from main.cpp (loaded from EEPROM)
// const int BREATH_NOTE_ON_THRESHOLD = 50; // Replaced by currentBreathNoteOnThreshold (loaded from EEPROM)
// const unsigned long CC_BREATH_INTERVAL = 20; // Replaced by currentCCBreathIntervalMs (loaded from EEPROM)
const int DEFAULT_NOTE_VELOCITY = 100;           // Default MIDI velocity (0-127) for Note On messages. (Could be EEPROM too)


// Control Key bit positions (within currentKeyStates)
// These map to MPR121_3 (cap3) channels 0-3 (bits 26-29) and potentially one more (bit 30)
const uint32_t KEY_BIT_OCTAVE_UP        = (1 << 26); // MPR121_3 CH0 -> currentKeyStates bit 26
const uint32_t KEY_BIT_OCTAVE_DOWN      = (1 << 27); // MPR121_3 CH1 -> currentKeyStates bit 27
const uint32_t KEY_BIT_CYCLE_FINGERING  = (1 << 28); // MPR121_3 CH2 -> currentKeyStates bit 28
const uint32_t KEY_BIT_CYCLE_POLYSELECT = (1 << 29); // MPR121_3 CH3 -> currentKeyStates bit 29
const uint32_t KEY_BIT_CYCLE_MIDI_CHANNEL = (1 << 30); // Example: Bit 30 (ensure this is a free bit)

// Combined mask for all control keys, useful for updating previousControlKeyStates
const uint32_t CONTROL_KEY_MASK = KEY_BIT_OCTAVE_UP | KEY_BIT_OCTAVE_DOWN | KEY_BIT_CYCLE_FINGERING | KEY_BIT_CYCLE_POLYSELECT | KEY_BIT_CYCLE_MIDI_CHANNEL;

// Limits for Octave
const int MAX_OCTAVE_SHIFT = 2;
const int MIN_OCTAVE_SHIFT = -2;

// --- Global State Variables (extern declarations) ---
// These variables reflect the current live operational state of the instrument.
// They are defined in main.cpp. Many are initialized from values loaded from the EVIConfig struct in EEPROM.
extern uint8_t currentMidiChannel;            // Live MIDI channel (1-16) used for sending messages. Loaded from EEPROM.
extern uint16_t currentBreathNoteOnThreshold; // Live breath threshold for Note On events (0-1023). Loaded from EEPROM.
extern uint16_t currentCCBreathIntervalMs;    // Live interval (ms) for Breath CC messages. Loaded from EEPROM.
extern FingeringMode currentFingeringMode;    // Live active fingering mode. Loaded from EEPROM.
extern PolySelectMode currentPolySelectMode;  // Live active polyphony/harmony mode. Loaded from EEPROM.
extern int8_t currentOctave;               // Live octave shift (e.g., -2 to +2). Loaded from EEPROM.
extern int8_t currentTranspose;           // Live overall transposition in semitones (e.g., -12 to +12). Loaded from EEPROM.
extern bool harmonyModeActive;
extern int8_t harmonyInterval;        // e.g., MIDI note interval for simple harmony

// MIDI CC Definitions
// Defines which MIDI Control Change (CC) numbers are used for various functions.
const int MIDI_CC_BREATH = 2;            // CC for breath control (standard).
const int MIDI_CC_VIBRATO = 1;           // CC for vibrato depth (typically Modulation Wheel).
const int MIDI_CC_PORTAMENTO_SWITCH = 65;// CC for Portamento On/Off (standard). (Not yet implemented in UI)
const int MIDI_CC_LIP_SENSOR = 16;       // CC for the lip sensor (General Purpose Controller 1).

// Sensor Thresholds
const int TOUCH_THRESHOLD = 400; // Example threshold for capacitive touch sensors

// Key/Sensor Identifiers (Logical, not physical pins)
// These will be bits in the keyState for NoteMapping
// Example: Assuming up to 32 keys/sensors can contribute to a fingering
// For MPR121 (12 keys each) and built-in (a few)
// Bit allocation for currentKeyStates (uint32_t):
// Bits 0-1: Built-in touch TOUCH_PIN_0, TOUCH_PIN_1.
// Bits 2-13: cap1 (MPR121_1) channels 0-11.
// Bits 14-25: cap2 (MPR121_2) channels 0-11.
// Bits 26-29: cap3 (MPR121_3) channels 0-3 (using only first 4 channels of 3rd chip).
// Total 2 + 12 + 12 + 4 = 30 bits used.

#endif // CONFIG_H
