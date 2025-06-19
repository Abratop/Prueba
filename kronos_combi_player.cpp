#include "kronos_combi_player.h"
#include "pcg_parser.h" // For local PcgParser instance
#include <iostream>
#include <string>
#include <vector>
#include <thread> // For std::this_thread::sleep_for
#include <chrono> // For std::chrono::milliseconds
#include <limits> // For std::numeric_limits
#include <atomic> // For std::atomic with arp_thread_

KronosCombiPlayer::KronosCombiPlayer()
    : arpeggiator_([this](uint8_t n, uint8_t v, bool on, uint8_t ch) {
          if (midi_output_.isPortOpen()) {
              if (on) {
                  midi_output_.sendNoteOn(ch, n, v);
              } else {
                  midi_output_.sendNoteOff(ch, n, v);
              }
          }
      }),
      run_arp_thread_(false), // Ensure this is initialized
      initialized_(false),
      ports_selected_(false),
      current_combi_index_(-1) {
    std::cout << "Kronos Combi Player initialized." << std::endl;
}

KronosCombiPlayer::~KronosCombiPlayer() {
    std::cout << "KronosCombiPlayer destructor called." << std::endl;
    shutdown();
}

void KronosCombiPlayer::initialize() {
    std::cout << "\n--- Initializing MIDI Ports ---" << std::endl;

    // Output Port
    std::cout << "\n--- MIDI Output Port Selection ---" << std::endl;
    unsigned int out_port_count = midi_output_.getPortCount();
    std::cout << "Found " << out_port_count << " MIDI output ports." << std::endl;
    if (out_port_count == 0) {
        std::cerr << "ERROR: No MIDI output ports available! Cannot proceed." << std::endl;
        initialized_ = false;
        ports_selected_ = false;
        return;
    }
    for (unsigned int i = 0; i < out_port_count; ++i) {
        std::cout << "  Output Port " << i << ": " << midi_output_.getPortName(i) << std::endl;
    }
    unsigned int selected_out_port;
    std::cout << "Select MIDI output port number: ";
    while (!(std::cin >> selected_out_port) || selected_out_port >= out_port_count) {
        std::cerr << "Invalid selection. Please enter a number between 0 and " << out_port_count -1 << ": ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    std::cout << "Attempting to open MIDI Output port: " << selected_out_port << " - " << midi_output_.getPortName(selected_out_port) << std::endl;
    if (!midi_output_.openPort(selected_out_port)) {
        std::cerr << "ERROR: Failed to open MIDI output port " << selected_out_port << " - " << midi_output_.getPortName(selected_out_port) << std::endl;
        initialized_ = false;
        ports_selected_ = false;
        return;
    }
    std::cout << "Successfully opened MIDI Output port: " << selected_out_port << " - " << midi_output_.getPortName(selected_out_port) << std::endl;

    // Input Port
    std::cout << "\n--- MIDI Input Port Selection ---" << std::endl;
    unsigned int in_port_count = midi_input_.getPortCount();
    std::cout << "Found " << in_port_count << " MIDI input ports." << std::endl;
    if (in_port_count == 0) {
        std::cerr << "ERROR: No MIDI input ports available! Cannot proceed." << std::endl;
        midi_output_.closePort();
        initialized_ = false;
        ports_selected_ = false;
        return;
    }
    for (unsigned int i = 0; i < in_port_count; ++i) {
        std::cout << "  Input Port " << i << ": " << midi_input_.getPortName(i) << std::endl;
    }
    unsigned int selected_in_port;
    std::cout << "Select MIDI input port number: ";
    while (!(std::cin >> selected_in_port) || selected_in_port >= in_port_count) {
        std::cerr << "Invalid selection. Please enter a number between 0 and " << in_port_count -1 << ": ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    std::cout << "Attempting to open MIDI Input port: " << selected_in_port << " - " << midi_input_.getPortName(selected_in_port) << std::endl;
    if (!midi_input_.openPort(selected_in_port)) {
        std::cerr << "ERROR: Failed to open MIDI input port " << selected_in_port << " - " << midi_input_.getPortName(selected_in_port) << std::endl;
        midi_output_.closePort();
        initialized_ = false;
        ports_selected_ = false;
        return;
    }
    std::cout << "Successfully opened MIDI Input port: " << selected_in_port << " - " << midi_input_.getPortName(selected_in_port) << std::endl;

    // Setup MIDI input callbacks
    std::cout << "Setting up MIDI input callbacks..." << std::endl;
    midi_input_.setArpeggioNoteCallback(
        [this](uint8_t ch, uint8_t n, uint8_t v, bool on) {
            this->handleNoteEvent(ch, n, v, on);
        });
    midi_input_.setArpeggioControlCallback(
        [this](uint8_t ch, uint8_t c, uint8_t val) {
            this->handleControlEvent(ch, c, val);
        });

    std::cout << "\nMIDI ports configured successfully." << std::endl;
    ports_selected_ = true;
    initialized_ = true;

    // Default Arpeggiator settings
    arp_target_channel_ = 0; // Default target channel for arp input
    arp_output_channel_ = 0; // Default output channel for arp notes
    arpeggiator_.setChannel(arp_output_channel_);
    arpeggiator_.setRate(4.0f); // Default 4 notes per second (120 BPM 16ths)
    arpeggiator_.setOctaves(1); // Default to 1 octave range (original + 1 up)
    arpeggiator_.setPattern(ArpPattern::UP);
    std::cout << "Default Arpeggiator settings: Target CH" << (int)arp_target_channel_
              << ", Output CH" << (int)arp_output_channel_
              << ", Rate " << arpeggiator_.getRate() << " Hz, Octaves " << 1
              << ", Pattern UP" << std::endl;
}

bool KronosCombiPlayer::loadPcgFile(const std::string& filePath) {
    if (!ports_selected_) { // Should be guaranteed by initialized_ check before calling this
        std::cerr << "ERROR: MIDI ports not selected/initialized. Please call initialize() first." << std::endl;
        return false;
    }
    std::cout << "Loading PCG file: " << filePath << std::endl;
    PcgParser parser(filePath);
    if (!parser.parse()) {
        std::cerr << "ERROR: Failed to load or parse PCG file: " << filePath << std::endl;
        return false;
    }
    loaded_combis_ = parser.getCombis();
    if (loaded_combis_.empty()) {
        std::cout << "PCG file loaded, but no combis were found." << std::endl;
    } else {
        std::cout << "PCG file loaded successfully. Found " << loaded_combis_.size() << " combis." << std::endl;
    }
    current_combi_index_ = -1;
    return true;
}

void KronosCombiPlayer::listCombis() const {
    if (loaded_combis_.empty()) {
        std::cout << "\nNo combis loaded. Load a PCG file first." << std::endl;
        return;
    }
    std::cout << "\n--- Available Combis (" << loaded_combis_.size() << ") ---" << std::endl;
    for (size_t i = 0; i < loaded_combis_.size(); ++i) {
        // Ensure name is null-terminated before creating std::string
        char name_buffer[25] = {0};
        strncpy(name_buffer, loaded_combis_[i].name, 24);
        std::cout << i << ": " << name_buffer << std::endl;
    }
}

bool KronosCombiPlayer::selectCombi(int combiIndex) {
    if (!initialized_) {
        std::cerr << "ERROR: Player not initialized. Cannot select combi." << std::endl;
        return false;
    }
    if (combiIndex < 0 || static_cast<size_t>(combiIndex) >= loaded_combis_.size()) {
        std::cerr << "ERROR: Invalid combi index: " << combiIndex
                  << ". Please choose between 0 and " << loaded_combis_.size() - 1 << "." << std::endl;
        return false;
    }

    current_combi_index_ = combiIndex;
    current_combi_details_ = loaded_combis_[combiIndex];

    char name_buffer[25] = {0};
    strncpy(name_buffer, current_combi_details_.name, 24);
    std::cout << "\nSelecting Combi " << combiIndex << ": " << name_buffer << std::endl;

    bool first_active_timbre_found = false;
    std::cout << "Setting up timbres on MIDI output..." << std::endl;
    for (int i = 0; i < 16; ++i) {
        const TimbreData& timbre = current_combi_details_.timbres[i];
        uint8_t midi_channel = static_cast<uint8_t>(i);

        if (timbre.status == 0x21) { // 'INT' (Internal)
            std::cout << "  Setting up Timbre " << i << " (Channel " << (int)midi_channel
                      << "): Prog LSB=0x" << std::hex << (int)timbre.program_lsb
                      << ", Bank MSB=0x" << (int)timbre.program_bank_msb << std::dec
                      << " (Status: Active 0x" << std::hex << (int)timbre.status << std::dec << ")" << std::endl;
            midi_output_.setupTimbre(timbre, midi_channel);
            if (!first_active_timbre_found) {
                arp_target_channel_ = midi_channel;
                arp_output_channel_ = midi_channel;
                arpeggiator_.setChannel(arp_output_channel_);
                std::cout << "  Arpeggiator Target/Output Channel automatically set to: " << (int)arp_target_channel_ << std::endl;
                first_active_timbre_found = true;
            }
        } else {
            // std::cout << "  Timbre " << i << " (Channel " << (int)midi_channel << ") is Inactive (Status: 0x"
            //           << std::hex << (int)timbre.status << std::dec << "). Sending AllNotesOff/Vol0." << std::endl;
            midi_output_.sendControlChange(midi_channel, 123, 0);
            midi_output_.sendControlChange(midi_channel, 7, 0);
        }
    }

    if (!first_active_timbre_found) {
        std::cout << "  WARNING: No active internal timbres found in this combi. Arpeggiator will use default channel "
                  << (int)arp_target_channel_ << " if notes are sent there." << std::endl;
    }

    startArpThread();
    std::cout << "Combi setup complete for '" << name_buffer << "'." << std::endl;
    return true;
}

void KronosCombiPlayer::startArpThread() {
    if (run_arp_thread_) {
        std::cout << "Arpeggiator thread already running. Stopping and restarting..." << std::endl;
        stopArpThread();
    }
    run_arp_thread_ = true;
    arp_thread_ = std::thread([this]() {
        std::cout << "Arpeggiator thread loop started. Rate: " << arpeggiator_.getRate()
                  << " notes/sec. (Target CH" << (int)arp_target_channel_
                  << ", Output CH" << (int)arp_output_channel_ << ")" << std::endl;
        while (run_arp_thread_) {
            if (arpeggiator_.isActive()) {
                arpeggiator_.tick();
            }
            float rate_hz = arpeggiator_.getRate();
            long sleep_ms = 100;
            if (rate_hz > 0.001f && rate_hz <= 100.0f) { // Max 100 Hz to prevent too small sleep
                 sleep_ms = static_cast<long>(1000.0f / rate_hz);
                 if (sleep_ms < 10) sleep_ms = 10;
            } else if (rate_hz == 0) {
                sleep_ms = 200;
            } else if (rate_hz > 100.0f) { // Cap rate for sleep calculation
                std::cerr << "Warning: Arp rate " << rate_hz << " Hz is very high. Capping effective rate for sleep." << std::endl;
                sleep_ms = static_cast<long>(1000.0f / 100.0f); // Cap at 100Hz for sleep
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
        }
        std::cout << "Arpeggiator thread loop finished." << std::endl;
        if (arpeggiator_.isActive() && arpeggiator_.last_note_is_on_ ) {
             std::cout << "Arp thread: Sending final note off for " << (int)arpeggiator_.last_played_note_ << std::endl;
             midi_output_.sendNoteOff(arpeggiator_.output_channel_, arpeggiator_.last_played_note_, 0);
        }
    });
}

void KronosCombiPlayer::stopArpThread() {
    std::cout << "Stopping arpeggiator thread..." << std::endl;
    run_arp_thread_ = false;
    if (arp_thread_.joinable()) {
        arp_thread_.join();
        std::cout << "Arpeggiator thread successfully stopped." << std::endl;
    } else {
        std::cout << "Arpeggiator thread was not joinable (already stopped or not started)." << std::endl;
    }
}


void KronosCombiPlayer::run() {
    if (!initialized_ || current_combi_index_ == -1) {
        std::cerr << "Player not initialized or no combi selected. Exiting run loop." << std::endl;
        return;
    }
    // The actual run loop is now in main.cpp, this method might be deprecated or repurposed.
    // For now, if called, it can just print status.
    std::cout << "\nKronos Combi Player run() called (main loop is in main.cpp)." << std::endl;
    std::cout << "Selected Combi: " << current_combi_index_ << std::endl;
    std::cout << "Arpeggiator is " << (run_arp_thread_ ? "active" : "inactive")
              << " targeting channel " << (int)arp_target_channel_
              << ", outputting to channel " << (int)arp_output_channel_ << std::endl;
    // The thread in main.cpp will keep it alive.
}

void KronosCombiPlayer::shutdown() {
    std::cout << "\nShutting down Kronos Combi Player..." << std::endl;
    stopArpThread(); // Stop arpeggiator thread first

    if (midi_output_.isPortOpen()) {
        for (uint8_t ch = 0; ch < 16; ++ch) {
            midi_output_.sendControlChange(ch, 123, 0); // CC123: All Notes Off
            midi_output_.sendControlChange(ch, 121, 0); // CC121: Reset All Controllers
        }
        midi_output_.closePort();
    }
    if (midi_input_.isPortOpen()) {
        midi_input_.closePort();
    }
    initialized_ = false;
    ports_selected_ = false;
    std::cout << "Shutdown complete." << std::endl;
}

void KronosCombiPlayer::handleNoteEvent(uint8_t channel, uint8_t note, uint8_t velocity, bool isNoteOn) {
    std::cout << "MIDI Note Event: Chan=" << (int)channel
              << " Note=" << (int)note
              << " Vel=" << (int)velocity
              << " On=" << (isNoteOn ? "True" : "False") << std::endl;

    // General log for any incoming note
    // std::cout << "MIDI Note Event: Chan=" << (int)channel << " Note=" << (int)note
    //           << " Vel=" << (int)velocity << " On=" << (isNoteOn ? "True" : "False") << std::endl;

    if (channel == arp_target_channel_) {
        std::cout << "ARP INPUT: Chan=" << (int)channel << " Note=" << (int)note
                  << " Vel=" << (int)velocity << " On=" << (isNoteOn ? "True" : "False") << std::endl;
        if (isNoteOn) {
            arpeggiator_.noteOn(note, velocity, arp_output_channel_);
        } else {
            arpeggiator_.noteOff(note, arp_output_channel_);
        }
    } else {
        std::cout << "PASS-THROUGH: Chan=" << (int)channel << " Note=" << (int)note
                  << " Vel=" << (int)velocity << " On=" << (isNoteOn ? "True" : "False") << std::endl;
        if (midi_output_.isPortOpen() && current_combi_index_ != -1 && channel < 16) {
            const TimbreData& targetTimbre = current_combi_details_.timbres[channel];
            if (targetTimbre.status == 0x21) {
                if (isNoteOn) {
                    midi_output_.sendNoteOn(channel, note, velocity);
                } else {
                    midi_output_.sendNoteOff(channel, note, velocity);
                }
            } else {
                 std::cout << "  Note on channel " << (int)channel << " ignored (timbre not active)." << std::endl;
            }
        }
    }
}

void KronosCombiPlayer::handleControlEvent(uint8_t channel, uint8_t controller, uint8_t value) {
    std::cout << "MIDI CC Event: Chan=" << (int)channel
              << " CC=" << (int)controller
              << " Val=" << (int)value << std::endl;

    // Pass through CC messages on all active channels in the combi,
    // including the arp_target_channel (could be used for expression, etc.)
    if (midi_output_.isPortOpen() && current_combi_index_ != -1 && channel < 16) {
        const TimbreData& targetTimbre = current_combi_details_.timbres[channel];
        if (targetTimbre.status == 0x21) {
            std::cout << "  Passing through CC on channel " << (int)channel << std::endl;
            midi_output_.sendControlChange(channel, controller, value);
        } else {
            std::cout << "  CC on channel " << (int)channel << " ignored (timbre not active)." << std::endl;
        }
    }
}
