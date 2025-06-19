#include "midi_input.h"
#include <iostream> // For std::cerr, std::cout (logging)

MidiInput::MidiInput() {
    try {
        midi_in_ = std::make_unique<RtMidiIn>();
    } catch (const RtMidiError &error) {
        std::cerr << "Error creating RtMidiIn: " << error.what() << std::endl;
        // midi_in_ will remain nullptr. Methods should check for midi_in_ validity.
    }
}

MidiInput::~MidiInput() {
    if (midi_in_ && midi_in_->isPortOpen()) {
        closePort(); // Also cancels callback
    }
    // std::unique_ptr handles deletion of midi_in_
}

bool MidiInput::openPort(unsigned int portNumber) {
    if (!midi_in_) {
        std::cerr << "MidiInput: RtMidiIn not initialized." << std::endl;
        return false;
    }
    if (midi_in_->isPortOpen()) {
        std::cout << "MidiInput: Port already open. Closing first." << std::endl;
        // closePort() also cancels previous callbacks
        midi_in_->closePort();
    }
    if (portNumber >= midi_in_->getPortCount()) {
        std::cerr << "MidiInput: Invalid port number: " << portNumber << std::endl;
        return false;
    }
    try {
        midi_in_->openPort(portNumber);
        std::cout << "MidiInput: Opened input port " << portNumber << " (" << midi_in_->getPortName(portNumber) << ")" << std::endl;
        // Set up the callback function.
        midi_in_->setCallback(&MidiInput::rtMidiCallbackWrapper, this);
        // Ignore SysEx, timing, and active sensing messages for typical arpeggio control.
        midi_in_->ignoreTypes(true, true, true);
    } catch (const RtMidiError &error) {
        std::cerr << "MidiInput: Error opening port " << portNumber << ": " << error.what() << std::endl;
        return false;
    }
    return midi_in_->isPortOpen();
}

void MidiInput::closePort() {
    if (midi_in_ && midi_in_->isPortOpen()) {
        std::cout << "MidiInput: Closing MIDI input port." << std::endl;
        try {
            midi_in_->cancelCallback(); // Important to do this before or during closePort
            midi_in_->closePort();
        } catch (const RtMidiError &error) {
            std::cerr << "MidiInput: Error closing port: " << error.what() << std::endl;
        }
    }
}

unsigned int MidiInput::getPortCount() const {
    if (!midi_in_) {
        std::cerr << "MidiInput: RtMidiIn not initialized." << std::endl;
        return 0;
    }
    try {
        return midi_in_->getPortCount();
    } catch (const RtMidiError &error) {
        std::cerr << "MidiInput: Error getting port count: " << error.what() << std::endl;
        return 0;
    }
}

std::string MidiInput::getPortName(unsigned int portNumber) const {
    if (!midi_in_) {
        std::cerr << "MidiInput: RtMidiIn not initialized." << std::endl;
        return "";
    }
    try {
        return midi_in_->getPortName(portNumber);
    } catch (const RtMidiError &error) {
        std::cerr << "MidiInput: Error getting port name for port " << portNumber << ": " << error.what() << std::endl;
        return "";
    }
}

bool MidiInput::isPortOpen() const {
    if (!midi_in_) {
        return false;
    }
    return midi_in_->isPortOpen();
}

void MidiInput::setArpeggioNoteCallback(std::function<void(uint8_t, uint8_t, uint8_t, bool)> callback) {
    note_callback_ = callback;
}

void MidiInput::setArpeggioControlCallback(std::function<void(uint8_t, uint8_t, uint8_t)> callback) {
    control_callback_ = callback;
}

// Static callback wrapper
void MidiInput::rtMidiCallbackWrapper(double deltaTime, std::vector<unsigned char> *message, void *userData) {
    if (userData) {
        MidiInput *thisInstance = static_cast<MidiInput*>(userData);
        thisInstance->processMidiMessage(deltaTime, message);
    }
}

// Internal handler called by the static wrapper
void MidiInput::processMidiMessage(double /* deltaTime */, std::vector<unsigned char> *message) {
    if (!message || message->empty()) {
        return;
    }

    unsigned char statusByte = (*message)[0];
    unsigned char messageType = statusByte & 0xF0; // Get the message type (upper nibble)
    unsigned char channel = statusByte & 0x0F;   // Get the MIDI channel (lower nibble)

    // For logging received messages (optional)
    // std::cout << "MIDI In: Status 0x" << std::hex << (int)statusByte << std::dec;
    // for (size_t i = 1; i < message->size(); ++i) {
    //     std::cout << " Data" << i << " 0x" << std::hex << (int)(*message)[i] << std::dec;
    // }
    // std::cout << std::endl;

    if (messageType == 0x90) { // Note On
        if (message->size() >= 3) {
            uint8_t note = (*message)[1];
            uint8_t velocity = (*message)[2];
            if (velocity == 0) { // Note On with velocity 0 is often treated as Note Off
                if (note_callback_) {
                    note_callback_(channel, note, velocity, false); // isNoteOn = false
                }
            } else {
                if (note_callback_) {
                    note_callback_(channel, note, velocity, true); // isNoteOn = true
                }
            }
        }
    } else if (messageType == 0x80) { // Note Off
        if (message->size() >= 3) {
            uint8_t note = (*message)[1];
            uint8_t velocity = (*message)[2];
            if (note_callback_) {
                note_callback_(channel, note, velocity, false); // isNoteOn = false
            }
        }
    } else if (messageType == 0xB0) { // Control Change (CC)
        if (message->size() >= 3) {
            uint8_t controller = (*message)[1];
            uint8_t value = (*message)[2];
            if (control_callback_) {
                control_callback_(channel, controller, value);
            }
        }
    } else {
        // std::cout << "MidiInput: Received other message type 0x" << std::hex << (int)messageType << std::dec << std::endl;
    }
}
