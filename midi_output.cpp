#include "midi_output.h"
#include <vector>
#include <iostream> // For std::cout and std::cerr

MidiOutput::MidiOutput() {
    try {
        midi_out_ = std::make_unique<RtMidiOut>();
    } catch (const RtMidiError &error) {
        std::cerr << "Error creating RtMidiOut: " << error.what() << std::endl;
        // midi_out_ will remain nullptr, isPortOpen() will be false.
        // Methods should check for midi_out_ validity.
    }
}

MidiOutput::~MidiOutput() {
    if (midi_out_ && midi_out_->isPortOpen()) {
        closePort();
    }
    // std::unique_ptr will handle deletion of midi_out_
}

bool MidiOutput::openPort(unsigned int portNumber) {
    if (!midi_out_) {
        std::cerr << "MidiOutput: RtMidiOut not initialized." << std::endl;
        return false;
    }
    if (midi_out_->isPortOpen()) {
        std::cout << "MidiOutput: Port already open. Closing first." << std::endl;
        midi_out_->closePort();
    }
    if (portNumber >= midi_out_->getPortCount()) {
        std::cerr << "MidiOutput: Invalid port number: " << portNumber << std::endl;
        return false;
    }
    try {
        midi_out_->openPort(portNumber);
        std::cout << "MidiOutput: Opened port " << portNumber << " (" << midi_out_->getPortName(portNumber) << ")" << std::endl;
    } catch (const RtMidiError &error) {
        std::cerr << "MidiOutput: Error opening port " << portNumber << ": " << error.what() << std::endl;
        return false;
    }
    return midi_out_->isPortOpen();
}

void MidiOutput::closePort() {
    if (midi_out_ && midi_out_->isPortOpen()) {
        std::cout << "MidiOutput: Closing MIDI port." << std::endl;
        try {
            midi_out_->closePort();
        } catch (const RtMidiError &error) {
            std::cerr << "MidiOutput: Error closing port: " << error.what() << std::endl;
        }
    }
}

unsigned int MidiOutput::getPortCount() const {
    if (!midi_out_) {
        std::cerr << "MidiOutput: RtMidiOut not initialized." << std::endl;
        return 0;
    }
    try {
        return midi_out_->getPortCount();
    } catch (const RtMidiError &error) {
        std::cerr << "MidiOutput: Error getting port count: " << error.what() << std::endl;
        return 0;
    }
}

std::string MidiOutput::getPortName(unsigned int portNumber) const {
    if (!midi_out_) {
        std::cerr << "MidiOutput: RtMidiOut not initialized." << std::endl;
        return "";
    }
    try {
        return midi_out_->getPortName(portNumber);
    } catch (const RtMidiError &error) {
        std::cerr << "MidiOutput: Error getting port name for port " << portNumber << ": " << error.what() << std::endl;
        return "";
    }
}

bool MidiOutput::isPortOpen() const {
    if (!midi_out_) {
        return false;
    }
    return midi_out_->isPortOpen();
}

void MidiOutput::sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (!isPortOpen()) {
        // std::cerr << "MidiOutput: Port not open. Cannot send Note On." << std::endl;
        return;
    }
    if (channel > 15 || note > 127 || velocity > 127) {
        std::cerr << "MidiOutput: Invalid Note On parameter." << std::endl;
        return;
    }
    std::vector<unsigned char> message = {static_cast<unsigned char>(0x90 | channel), note, velocity};
    try {
        midi_out_->sendMessage(&message);
    } catch (const RtMidiError &error) {
        std::cerr << "MidiOutput: Error sending Note On: " << error.what() << std::endl;
    }
}

void MidiOutput::sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (!isPortOpen()) {
        // std::cerr << "MidiOutput: Port not open. Cannot send Note Off." << std::endl;
        return;
    }
     if (channel > 15 || note > 127 || velocity > 127) {
        std::cerr << "MidiOutput: Invalid Note Off parameter." << std::endl;
        return;
    }
    std::vector<unsigned char> message = {static_cast<unsigned char>(0x80 | channel), note, velocity};
    try {
        midi_out_->sendMessage(&message);
    } catch (const RtMidiError &error) {
        std::cerr << "MidiOutput: Error sending Note Off: " << error.what() << std::endl;
    }
}

void MidiOutput::sendProgramChange(uint8_t channel, uint8_t programNumber) {
    if (!isPortOpen()) {
        // std::cerr << "MidiOutput: Port not open. Cannot send Program Change." << std::endl;
        return;
    }
    if (channel > 15 || programNumber > 127) {
        std::cerr << "MidiOutput: Invalid Program Change parameter." << std::endl;
        return;
    }
    std::vector<unsigned char> message = {static_cast<unsigned char>(0xC0 | channel), programNumber};
    try {
        midi_out_->sendMessage(&message);
    } catch (const RtMidiError &error) {
        std::cerr << "MidiOutput: Error sending Program Change: " << error.what() << std::endl;
    }
}

void MidiOutput::sendControlChange(uint8_t channel, uint8_t controllerNumber, uint8_t value) {
    if (!isPortOpen()) {
        // std::cerr << "MidiOutput: Port not open. Cannot send Control Change." << std::endl;
        return;
    }
    if (channel > 15 || controllerNumber > 127 || value > 127) {
        std::cerr << "MidiOutput: Invalid Control Change parameter." << std::endl;
        return;
    }
    std::vector<unsigned char> message = {static_cast<unsigned char>(0xB0 | channel), controllerNumber, value};
    try {
        midi_out_->sendMessage(&message);
    } catch (const RtMidiError &error) {
        std::cerr << "MidiOutput: Error sending Control Change: " << error.what() << std::endl;
    }
}

void MidiOutput::setupTimbre(const TimbreData& timbre, uint8_t channel) {
    if (channel > 15) {
        std::cerr << "MidiOutput::setupTimbre: Invalid MIDI channel " << (int)channel << std::endl;
        return;
    }

    // Korg Timbre Status:
    // From user spec: "Status 20->00 = Off 2: 0010 Int -> (byte && 0xE0)>>5 000-> Off 001->Int 011->Ext 100->EX2"
    // Example values: Timbre 0 Status = 0x00 (Off), Timbre 1 Status = 0x21 (Int)
    // This implies direct check of status byte might be sufficient for common cases.
    // Let's assume 0x21 means 'INT' (Internal, playable)
    // Other possible values: 0x00 (Off), 0x62 (Ext), 0x83 (EX2)
    if (timbre.status != 0x21) { // Check for 'INT' status
        std::cout << "MidiOutput: Timbre on channel " << (int)channel
                  << " is not active (Status: 0x" << std::hex << (int)timbre.status << std::dec << "). Skipping setup." << std::endl;
        return;
    }

    // IMPORTANT: This is a placeholder for Korg-specific bank select.
    // Actual MSB/LSB values depend on timbre.program_bank_msb and Korg's MIDI implementation guide.
    // Korg often uses program_bank_msb to indicate the bank type (e.g., I-A to I-F, U-A to U-G, GM).
    // Each of these banks then maps to a specific MSB/LSB pair.
    // For example:
    //   - GM bank: MSB=0, LSB can be 0 or other values.
    //   - Standard Korg banks (I-A, HD-1 Program Banks): Often MSB=0, LSB=bank_number (0 for A, 1 for B, etc.)
    //   - EXi Program Banks: Might use different MSB values.
    // The timbre.program_bank_msb field (e.g., 0x07 in "Michel's 7s" example for Timbre 1)
    // needs to be translated. If 0x07 means "Bank I-H", we'd need to find the MIDI MSB/LSB for I-H.
    // Let's assume for this placeholder that timbre.program_bank_msb is the LSB and MSB is 0 for common internal banks.
    // This is a common pattern for many synths for their main program banks but is NOT universally correct for all Korg banks.

    uint8_t bankMsb = 0;    // Placeholder - often 0 for main program banks on many synths.
    uint8_t bankLsb = timbre.program_bank_msb; // Placeholder - ASSUMING program_bank_msb directly maps to LSB.
                                               // This needs to be verified against Korg MIDI Implementation.

    std::cout << "MidiOutput: Setting up Timbre on Channel " << (int)channel
              << ": Original Korg Bank Info: 0x" << std::hex << (int)timbre.program_bank_msb << std::dec
              << ", Original Korg Program LSB: " << (int)timbre.program_lsb
              << " (Status: 0x" << std::hex << (int)timbre.status << std::dec << ")"
              << std::endl;

    if (!isPortOpen()) {
        std::cerr << "MidiOutput: Port not open. Cannot setup Timbre." << std::endl;
        return;
    }

    // Send Bank Select MSB (CC0), Bank Select LSB (CC32), then Program Change
    sendControlChange(channel, 0, bankMsb);       // CC0: Bank Select MSB
    sendControlChange(channel, 32, bankLsb);      // CC32: Bank Select LSB
    sendProgramChange(channel, timbre.program_lsb); // Program Change

    std::cout << "  Sent MIDI to Channel " << (int)channel << ": Bank MSB " << (int)bankMsb << ", LSB " << (int)bankLsb
              << ", Program " << (int)timbre.program_lsb << std::endl;
    std::cout << "  NOTE: Bank selection (MSB/LSB) logic is a placeholder and needs Korg-specific mapping." << std::endl;
}
