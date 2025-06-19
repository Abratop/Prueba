#ifndef MIDI_INPUT_H
#define MIDI_INPUT_H

#include "libs/rtmidi/RtMidi.h"
#include <string>
#include <vector>
#include <iostream> // For potential logging
#include <functional> // For std::function
#include <memory>     // For std::unique_ptr

class MidiInput {
public:
    MidiInput();
    ~MidiInput();

    // Port Management Methods
    bool openPort(unsigned int portNumber);
    void closePort();
    unsigned int getPortCount() const;
    std::string getPortName(unsigned int portNumber) const;
    bool isPortOpen() const;

    // Callback Setup
    // Callback for Note On/Off messages: channel (0-15), note (0-127), velocity (0-127), isNoteOn (true for NoteOn, false for NoteOff)
    void setArpeggioNoteCallback(std::function<void(uint8_t channel, uint8_t note, uint8_t velocity, bool isNoteOn)> callback);
    // Callback for Control Change messages: channel (0-15), controller (0-127), value (0-127)
    void setArpeggioControlCallback(std::function<void(uint8_t channel, uint8_t controller, uint8_t value)> callback);

    // MIDI Processing Control (Optional - RtMidiIn usually starts listening after callback is set and port is open)
    // void startListening(); // Could call midi_in_->setCallback here if not done in openPort
    // void stopListening();  // Could call midi_in_->cancelCallback here

private:
    std::unique_ptr<RtMidiIn> midi_in_;
    std::function<void(uint8_t channel, uint8_t note, uint8_t velocity, bool isNoteOn)> note_callback_;
    std::function<void(uint8_t channel, uint8_t controller, uint8_t value)> control_callback_;

    // Static callback wrapper to be passed to RtMidiIn
    static void rtMidiCallbackWrapper(double deltaTime, std::vector<unsigned char> *message, void *userData);

    // Internal handler called by the static wrapper
    void processMidiMessage(double deltaTime, std::vector<unsigned char> *message);
};

#endif // MIDI_INPUT_H
