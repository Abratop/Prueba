#include "arpeggiator.h"
#include <iostream>
#include <string>   // For pattern to string
#include <random>   // For std::random_device to seed mt19937

// Helper to convert ArpPattern to string for logging
std::string patternToString(ArpPattern pattern) {
    switch (pattern) {
        case ArpPattern::UP: return "UP";
        case ArpPattern::DOWN: return "DOWN";
        case ArpPattern::UP_DOWN: return "UP_DOWN";
        case ArpPattern::RANDOM: return "RANDOM";
        default: return "UNKNOWN";
    }
}

Arpeggiator::Arpeggiator(MidiNoteCallback noteOutputCallback)
    : noteOutputCallback_(noteOutputCallback),
      current_pattern_(ArpPattern::UP),
      octaves_(0),
      rate_hz_(4.0f),
      gate_(0.5f),
      current_step_(0),
      up_down_direction_up_(true),
      last_played_note_(0),
      last_note_is_on_(false),
      current_velocity_(100),
      output_channel_(0) {
    std::random_device rd;
    random_engine_ = std::mt19937(rd());
    // std::cout << "Arpeggiator created." << std::endl; // Optional: Log constructor
}

void Arpeggiator::noteOn(uint8_t note, uint8_t velocity, uint8_t /*channel*/) {
    std::cout << "Arp: Note On " << (int)note << " Vel " << (int)velocity << " received." << std::endl;
    bool first_note = held_notes_orig_.empty();
    if (std::find(held_notes_orig_.begin(), held_notes_orig_.end(), note) == held_notes_orig_.end()) {
        held_notes_orig_.push_back(note);
        std::sort(held_notes_orig_.begin(), held_notes_orig_.end());
    }
    current_velocity_ = velocity;

    if (first_note) {
        current_step_ = 0;
        up_down_direction_up_ = true;
        last_note_is_on_ = false;
        std::cout << "Arp: First note added, resetting step." << std::endl;
    }
    rebuildArpNotes();
}

void Arpeggiator::noteOff(uint8_t note, uint8_t /*channel*/) {
    std::cout << "Arp: Note Off " << (int)note << " received." << std::endl;
    held_notes_orig_.erase(std::remove(held_notes_orig_.begin(), held_notes_orig_.end(), note), held_notes_orig_.end());

    if (held_notes_orig_.empty()) {
        std::cout << "Arp: All notes released." << std::endl;
        if (last_note_is_on_ && noteOutputCallback_) {
            std::cout << "Arp: Sending Note Off for last played note " << (int)last_played_note_ << std::endl;
            noteOutputCallback_(last_played_note_, current_velocity_, false, output_channel_);
            last_note_is_on_ = false;
        }
        current_step_ = 0;
        up_down_direction_up_ = true;
    }
    rebuildArpNotes();
}

void Arpeggiator::tick() {
    if (last_note_is_on_ && noteOutputCallback_) {
        // std::cout << "Arp Tick: Turning off last note " << (int)last_played_note_ << std::endl;
        noteOutputCallback_(last_played_note_, current_velocity_, false, output_channel_);
        last_note_is_on_ = false;
    }

    if (current_arp_notes_.empty() || !noteOutputCallback_) {
        // std::cout << "Arp Tick: No notes to play or no callback." << std::endl;
        return;
    }

    if (current_step_ >= static_cast<int>(current_arp_notes_.size())) { // Safety check
        current_step_ = 0;
        if (current_arp_notes_.empty()) return; // Double check after reset
    }
        return;
    }

    uint8_t note_to_play = 0;

    // Determine note_to_play based on pattern and current_step_
    // (Implementation of pattern logic from previous step)
    switch (current_pattern_) {
        case ArpPattern::UP:
            note_to_play = current_arp_notes_[current_step_];
            current_step_ = (current_step_ + 1) % current_arp_notes_.size();
            break;
        case ArpPattern::DOWN:
            note_to_play = current_arp_notes_[current_arp_notes_.size() - 1 - current_step_];
            current_step_ = (current_step_ + 1) % current_arp_notes_.size();
            break;
        case ArpPattern::UP_DOWN:
             if (current_arp_notes_.size() == 1) {
                 note_to_play = current_arp_notes_[0];
                 current_step_ = 0;
            } else {
                if (up_down_direction_up_) {
                    note_to_play = current_arp_notes_[current_step_];
                    current_step_++;
                    if (current_step_ >= static_cast<int>(current_arp_notes_.size())) {
                        up_down_direction_up_ = false;
                        current_step_ = std::max(0, static_cast<int>(current_arp_notes_.size()) - 2);
                    }
                } else {
                    note_to_play = current_arp_notes_[current_step_];
                    current_step_--;
                    if (current_step_ < 0) {
                        up_down_direction_up_ = true;
                        current_step_ = std::min(1, static_cast<int>(current_arp_notes_.size()) -1 );
                         if (current_arp_notes_.size() == 1) current_step_ = 0;
                    }
                }
            }
            break;
        case ArpPattern::RANDOM:
            if (!current_arp_notes_.empty()) {
                std::uniform_int_distribution<int> dist(0, current_arp_notes_.size() - 1);
                current_step_ = dist(random_engine_);
                note_to_play = current_arp_notes_[current_step_];
            } else { // Should not happen if current_arp_notes_ is checked above
                return;
            }
            break;
    }

    std::cout << "Arp Tick: Pattern=" << patternToString(current_pattern_) << ", Step=" << current_step_
              << ", Note=" << (int)note_to_play << ", Vel=" << (int)current_velocity_
              << ", Chan=" << (int)output_channel_ << std::endl;

    noteOutputCallback_(note_to_play, current_velocity_, true, output_channel_);
    last_played_note_ = note_to_play;
    last_note_is_on_ = true;
}

void Arpeggiator::rebuildArpNotes() {
    // std::cout << "Arp: Rebuilding arpeggiator notes..." << std::endl;
    current_arp_notes_.clear();
    if (held_notes_orig_.empty()) {
        // std::cout << "Arp: No original notes held." << std::endl;
        return;
    }

    for (uint8_t base_note : held_notes_orig_) {
        current_arp_notes_.push_back(base_note);
        for (int o = 1; o <= octaves_; ++o) {
            if (base_note + (12 * o) <= 127) {
                current_arp_notes_.push_back(base_note + (12 * o));
            }
        }
    }
    std::sort(current_arp_notes_.begin(), current_arp_notes_.end());
    current_arp_notes_.erase(std::unique(current_arp_notes_.begin(), current_arp_notes_.end()), current_arp_notes_.end());

    if (current_step_ >= static_cast<int>(current_arp_notes_.size())) {
        current_step_ = 0;
    }
    // Note: Pattern-specific adjustments to current_step_ on rebuild might be needed for some patterns.
    // For UP_DOWN, if notes change, the current direction and step might lead to unexpected jumps.
    // Resetting to a known state (e.g., step 0, direction up) might be safer on rebuild for UP_DOWN.
    // if (current_pattern_ == ArpPattern::UP_DOWN) {
    //    up_down_direction_up_ = true;
    //    current_step_ = 0; // Or try to find closest note
    // }
    std::cout << "Arp: Rebuilt. Total arp notes: " << current_arp_notes_.size()
              << " (Original held: " << held_notes_orig_.size() << ")" << std::endl;
}

void Arpeggiator::setPattern(ArpPattern pattern) {
    std::cout << "Arp: Setting pattern to " << patternToString(pattern) << std::endl;
    current_pattern_ = pattern;
    current_step_ = 0;
    up_down_direction_up_ = true;
}

void Arpeggiator::setOctaves(int octaves) {
    if (octaves >= 0) {
        std::cout << "Arp: Setting octaves to " << octaves << std::endl;
        octaves_ = octaves;
        rebuildArpNotes();
    }
}

void Arpeggiator::setRate(float notesPerSecond) {
    if (notesPerSecond >= 0) { // Allow 0 to pause arp via timer logic
        std::cout << "Arp: Setting rate to " << notesPerSecond << " notes/sec." << std::endl;
        rate_hz_ = notesPerSecond;
    }
}

void Arpeggiator::setGate(float gate) {
    if (gate >= 0.0f && gate <= 1.0f) {
        std::cout << "Arp: Setting gate to " << gate << std::endl;
        gate_ = gate;
    }
}

void Arpeggiator::setChannel(uint8_t channel) {
    if (channel <= 15) {
        std::cout << "Arp: Setting output channel to " << (int)channel << std::endl;
        output_channel_ = channel;
    }
}

bool Arpeggiator::isActive() const {
    return !held_notes_orig_.empty();
}

float Arpeggiator::getRate() const {
    return rate_hz_;
}
