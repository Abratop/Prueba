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
    std::atomic<bool> run_arp_thread_{false};
    uint8_t arp_target_channel_ = 0;
    uint8_t arp_output_channel_ = 0;
    bool arpeggiator_enabled_ = false;

    bool initialized_ = false; // Tracks if MIDI ports are open and basic setup done
    bool ports_selected_ = false;
    std::vector<std::string> midi_input_port_names_;
    std::vector<std::string> midi_output_port_names_;
    std::vector<std::string> loaded_combi_names_for_gui_;


private:
    void startArpThread();
    void stopArpThread();
    void populatePortNames(); // Helper to fill port name vectors

public:
    // Constructor still initializes arpeggiator_
    // Destructor still calls shutdown()

    // void initialize(); // Old console-based initialize - to be replaced or re-purposed
    bool openMidiPorts(int inputPortIndex, int outputPortIndex);
    void scanMidiPorts(); // Fills the port name vectors

    // Getters for GUI
    const std::vector<std::string>& getMidiInputPortNames() const { return midi_input_port_names_; }
    const std::vector<std::string>& getMidiOutputPortNames() const { return midi_output_port_names_; }
    const std::vector<std::string>& getCombiNamesForGui() const { return loaded_combi_names_for_gui_; }
    // getCombiName(int index) might not be needed if we populate a string list above
    // const std::string& getCombiName(int index) const; // Requires CombiData.name to be std::string or converted

    bool isInitialized() const { return initialized_; } // Now means MIDI ports are successfully open
    size_t getCombiCount() const { return loaded_combis_.size(); }

    // Arpeggiator controls from GUI
    void setArpeggiatorEnabled(bool enabled);
    bool isArpeggiatorEnabled() const { return arpeggiator_enabled_; }
    Arpeggiator& getArpeggiator() { return arpeggiator_; }
    void setArpTargetChannel(uint8_t channel) { arp_target_channel_ = channel; } // GUI might need this
    uint8_t getArpTargetChannel() const { return arp_target_channel_; }
    void setArpOutputChannel(uint8_t channel) { // GUI might need this
        arp_output_channel_ = channel;
        arpeggiator_.setChannel(arp_output_channel_);
    }
    uint8_t getArpOutputChannel() const { return arp_output_channel_; }

    // Expose MidiInput/Output for direct port count/name access if needed by GUI before full init
    MidiInput& getMidiInput() { return midi_input_; }
    MidiOutput& getMidiOutput() { return midi_output_; }


    // loadPcgFile, selectCombi, run, shutdown remain mostly the same signatures
    // but their internal logic might adapt (e.g. run() might become a no-op)
};

#endif // KRONOS_COMBI_PLAYER_H
