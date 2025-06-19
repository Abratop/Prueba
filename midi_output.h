#ifndef MIDI_OUTPUT_H
#define MIDI_OUTPUT_H

#include "libs/rtmidi/RtMidi.h"
#include "pcg_structures.h" // For TimbreData
#include <string>
#include <vector>
#include <iostream> // For logging in setupTimbre
#include <memory>   // For std::unique_ptr

class MidiOutput {
public:
    MidiOutput();
    ~MidiOutput();

    // Port Management Methods
    bool openPort(unsigned int portNumber);
    void closePort();
    unsigned int getPortCount() const;
    std::string getPortName(unsigned int portNumber) const;
    bool isPortOpen() const;

    // MIDI Message Sending Methods
    void sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
    void sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);
    void sendProgramChange(uint8_t channel, uint8_t programNumber);
    void sendControlChange(uint8_t channel, uint8_t controllerNumber, uint8_t value);

    // Combi Playback Helper Method
    // Sets up a single timbre on a given MIDI channel.
    // Channel parameter is 0-15.
    void setupTimbre(const TimbreData& timbre, uint8_t channel);

private:
    std::unique_ptr<RtMidiOut> midi_out_;
};

#endif // MIDI_OUTPUT_H
