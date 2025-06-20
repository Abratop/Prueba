#include "midi_input.h"
#include <iostream> // For std::cerr, std::cout (logging)

MidiInput::MidiInput() {
    try {
        #ifdef _WIN32
            midi_in_ = std::make_unique<RtMidiIn>(RtMidi::Api::WINDOWS_MM, "KronosPlayerInClient");
            std::cout << "MidiInput: Requested WINDOWS_MM API." << std::endl;
        #else
            midi_in_ = std::make_unique<RtMidiIn>(RtMidi::Api::UNSPECIFIED, "KronosPlayerInClient");
            std::cout << "MidiInput: Requested UNSPECIFIED API (auto-select)." << std::endl;
        #endif
    } catch (const RtMidiError &error) {
        std::cerr << "ERROR: MidiInput Constructor: RtMidiError - " << error.getMessage() << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "ERROR: MidiInput Constructor: std::exception - " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "ERROR: MidiInput Constructor: Unknown exception." << std::endl;
    }
}

MidiInput::~MidiInput() {
    if (midi_in_ && isPortOpen()) { // isPortOpen has try-catch
        closePort();
    }
}

bool MidiInput::openPort(unsigned int portNumber) {
    if (!midi_in_) {
        std::cerr << "ERROR: MidiInput::openPort: RtMidiIn not initialized (constructor likely failed)." << std::endl;
        return false;
    }
    if (isPortOpen()) {
        std::cout << "MidiInput: Port was already open. Closing first." << std::endl;
        closePort();
    }

    unsigned int portCount = 0;
    try {
        portCount = midi_in_->getPortCount();
    } catch (const RtMidiError &error) {
        std::cerr << "ERROR: MidiInput::openPort: RtMidiError getting port count - " << error.getMessage() << std::endl;
        return false;
    }

    if (portNumber >= portCount) {
        std::cerr << "ERROR: MidiInput::openPort: Invalid port number: " << portNumber
                  << ". Available ports: " << portCount << "." << std::endl;
        return false;
    }

    try {
        std::string portNameStr = midi_in_->getPortName(portNumber);
        std::cout << "MidiInput: Attempting to open input port " << portNumber << " (" << portNameStr << ")" << std::endl;
        midi_in_->openPort(portNumber, "KronosPlayerIn");
        std::cout << "MidiInput: Successfully opened input port " << portNumber << " (" << portNameStr << ")" << std::endl;

        std::cout << "MidiInput: Setting callback function." << std::endl;
        midi_in_->setCallback(&MidiInput::rtMidiCallbackWrapper, this);

        std::cout << "MidiInput: Setting ignoreTypes (Sysex, Time, Sense)." << std::endl;
        midi_in_->ignoreTypes(true, true, true);
        return true;
    } catch (const RtMidiError &error) {
        std::cerr << "ERROR: MidiInput::openPort: RtMidiError - " << error.getMessage() << std::endl;
    }
    return false;
}

void MidiInput::closePort() {
    if (!midi_in_) {
        return;
    }
    if (isPortOpen()) { // isPortOpen has try-catch
        std::cout << "MidiInput: Closing MIDI input port." << std::endl;
        try {
            midi_in_->cancelCallback();
            std::cout << "MidiInput: Callback cancelled." << std::endl;
            midi_in_->closePort();
            std::cout << "MidiInput: Port closed." << std::endl;
        } catch (const RtMidiError &error) {
            std::cerr << "ERROR: MidiInput::closePort: RtMidiError - " << error.getMessage() << std::endl;
        }
    }
}

unsigned int MidiInput::getPortCount() const {
    if (!midi_in_) {
        return 0;
    }
    try {
        return midi_in_->getPortCount();
    } catch (const RtMidiError &error) {
        std::cerr << "ERROR: MidiInput::getPortCount: RtMidiError - " << error.getMessage() << std::endl;
        return 0;
    }
}

std::string MidiInput::getPortName(unsigned int portNumber) const {
    if (!midi_in_) {
        return "[Error: Not Initialized]";
    }
    try {
        return midi_in_->getPortName(portNumber);
    } catch (const RtMidiError &error) {
        std::cerr << "ERROR: MidiInput::getPortName for port " << portNumber << ": RtMidiError - " << error.getMessage() << std::endl;
        return "[Error: RtMidiError]";
    }
}

bool MidiInput::isPortOpen() const {
    if (!midi_in_) {
        return false;
    }
    try {
        return midi_in_->isPortOpen();
    } catch (const RtMidiError &error) {
        std::cerr << "ERROR: MidiInput::isPortOpen: RtMidiError - " << error.getMessage() << std::endl;
        return false;
    }
}

void MidiInput::setArpeggioNoteCallback(std::function<void(uint8_t, uint8_t, uint8_t, bool)> callback) {
    note_callback_ = callback;
}

void MidiInput::setArpeggioControlCallback(std::function<void(uint8_t, uint8_t, uint8_t)> callback) {
    control_callback_ = callback;
}

// Static callback wrapper
void MidiInput::rtMidiCallbackWrapper(double deltaTime, std::vector<unsigned char> *message, void *userData) {
    if (userData && message) { // Ensure message is not null
        MidiInput *thisInstance = static_cast<MidiInput*>(userData);
        // It's good practice to check if the instance is still valid if complex lifetime management occurs,
        // but for unique_ptr owned by KronosCombiPlayer and proper shutdown, it should be fine.
        thisInstance->processMidiMessage(deltaTime, message);
    }
}

// Internal handler called by the static wrapper
void MidiInput::processMidiMessage(double /* deltaTime */, std::vector<unsigned char> *message) {
    // message pointer already checked by wrapper
    if (message->empty()) {
        std::cerr << "MidiInput::processMidiMessage: Received empty MIDI message." << std::endl;
        return;
    }

    unsigned char statusByte = (*message)[0];
    unsigned char messageType = statusByte & 0xF0;
    unsigned char channel = statusByte & 0x0F;

    // Optional: Detailed logging of raw messages
    // std::cout << "MIDI In Raw: Status 0x" << std::hex << (int)statusByte << std::dec;
    // for (size_t i = 1; i < message->size(); ++i) {
    //     std::cout << " Data" << i << "=0x" << std::hex << (int)(*message)[i] << std::dec;
    // }
    // std::cout << std::endl;

    if (messageType == 0x90) { // Note On
        if (message->size() >= 3) {
            uint8_t note = (*message)[1];
            uint8_t velocity = (*message)[2];
            if (note_callback_) { // Check if callback is set
                if (velocity == 0) {
                    note_callback_(channel, note, velocity, false);
                } else {
                    note_callback_(channel, note, velocity, true);
                }
            }
        } else {
            std::cerr << "MidiInput::processMidiMessage: Note On message too short (size " << message->size() << ")." << std::endl;
        }
    } else if (messageType == 0x80) { // Note Off
        if (message->size() >= 3) {
            uint8_t note = (*message)[1];
            uint8_t velocity = (*message)[2];
            if (note_callback_) {
                note_callback_(channel, note, velocity, false);
            }
        } else {
             std::cerr << "MidiInput::processMidiMessage: Note Off message too short (size " << message->size() << ")." << std::endl;
        }
    } else if (messageType == 0xB0) { // Control Change (CC)
        if (message->size() >= 3) {
            uint8_t controller = (*message)[1];
            uint8_t value = (*message)[2];
            if (control_callback_) {
                control_callback_(channel, controller, value);
            }
        } else {
            std::cerr << "MidiInput::processMidiMessage: Control Change message too short (size " << message->size() << ")." << std::endl;
        }
    } else {
        // Optional: Log other message types if needed, but ignoreTypes should filter most non-channel messages.
        // std::cout << "MidiInput: Ignoring message type 0x" << std::hex << (int)messageType << std::dec << std::endl;
    }
}
