#ifndef ARPEGGIATOR_H
#define ARPEGGIATOR_H

#include <vector>
#include <cstdint>
#include <algorithm> // For std::sort, std::remove, std::find
#include <functional> // For std::function
#include <random>     // For std::mt19937 and std::uniform_int_distribution (for RANDOM pattern)

enum class ArpPattern {
    UP,
    DOWN,
    UP_DOWN, // Will require state to know if going up or down
    RANDOM
};

class Arpeggiator {
public:
    using MidiNoteCallback = std::function<void(uint8_t note, uint8_t velocity, bool isNoteOn, uint8_t channel)>;

    Arpeggiator(MidiNoteCallback noteOutputCallback);

    void noteOn(uint8_t note, uint8_t velocity, uint8_t channel); // Input channel of the note source
    void noteOff(uint8_t note, uint8_t channel); // Input channel of the note source

    void tick(); // Advances arpeggio by one step

    void setPattern(ArpPattern pattern);
    void setOctaves(int octaves); // 0 for original, 1 for original + 1 octave up, etc.
    void setRate(float notesPerSecond); // How many notes should play per second
    void setGate(float gate); // Note duration (0.0 to 1.0 of step time) - simplified for now
    void setChannel(uint8_t channel); // Sets the output MIDI channel for arpeggiated notes

    bool isActive() const;
    float getRate() const;
    ArpPattern getPattern() const { return current_pattern_; }
    int getOctaves() const { return octaves_; }


private:
    MidiNoteCallback noteOutputCallback_;
    std::vector<uint8_t> held_notes_orig_; // Original notes held by the player, kept sorted
    std::vector<uint8_t> current_arp_notes_; // Notes currently being arpeggiated including octaves & pattern order

    ArpPattern current_pattern_ = ArpPattern::UP;
    int octaves_ = 0; // Actual number of additional octaves (0 means only base notes)
    float rate_hz_ = 4.0f;
    float gate_ = 0.5f; // Currently conceptual, tick() does noteOff then noteOn

    int current_step_ = 0;
    bool up_down_direction_up_ = true; // For UP_DOWN pattern

    uint8_t last_played_note_ = 0;
    bool last_note_is_on_ = false;
    uint8_t current_velocity_ = 100;
    uint8_t output_channel_ = 0; // MIDI channel for arpeggiated output

    std::mt19937 random_engine_; // For RANDOM pattern

    void rebuildArpNotes();
};

#endif // ARPEGGIATOR_H
