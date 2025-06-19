#ifndef KRONOS_COMBI_PLAYER_H
#define KRONOS_COMBI_PLAYER_H

#include "pcg_parser.h"
#include "midi_output.h"
#include "midi_input.h"
#include "pcg_structures.h" // Though included by pcg_parser.h, explicit for clarity
#include "arpeggiator.h"    // Include Arpeggiator

#include <string>
#include <vector>
#include <iostream> // For user interaction
#include <thread>   // For std::thread
#include <atomic>   // For std::atomic<bool>

class KronosCombiPlayer {
public:
    KronosCombiPlayer();
    ~KronosCombiPlayer(); // To ensure shutdown is called if object is destroyed

    // Initialization and Setup
    void initialize(); // Prompts for MIDI port selection
    bool loadPcgFile(const std::string& filePath); // Loads combis from PCG file
    void listCombis() const; // Lists loaded combi names and indices
    bool selectCombi(int combiIndex); // Selects a combi and sets up MIDI output channels

    // Main operation and shutdown
    void run();       // Starts listening and keeps alive (e.g., for MIDI callbacks)
    void shutdown();  // Cleans up MIDI ports

private:
    // MIDI Event Handlers (called by MidiInput callbacks)
    void handleNoteEvent(uint8_t channel, uint8_t note, uint8_t velocity, bool isNoteOn);
    void handleControlEvent(uint8_t channel, uint8_t controller, uint8_t value);

    // Member Variables
    MidiOutput midi_output_;
    MidiInput midi_input_;

    std::vector<CombiData> loaded_combis_;
    int current_combi_index_ = -1;
    CombiData current_combi_details_; // Store details of the selected combi

    // Arpeggiator members
    Arpeggiator arpeggiator_;
    std::thread arp_thread_;
    std::atomic<bool> run_arp_thread_{false}; // Initialize to false
    uint8_t arp_target_channel_ = 0; // MIDI Channel whose input notes feed the arpeggiator
    uint8_t arp_output_channel_ = 0; // MIDI Channel where arpeggiator sends its output

    bool initialized_ = false;
    bool ports_selected_ = false; // To track if MIDI ports have been successfully selected

private:
    void startArpThread();
    void stopArpThread();

public:
    bool isInitialized() const { return initialized_ && ports_selected_; }
    size_t getCombiCount() const { return loaded_combis_.size(); }
};

#endif // KRONOS_COMBI_PLAYER_H
