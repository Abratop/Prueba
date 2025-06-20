#include "midi_output.h"
#include <vector>
#include <iostream> // For std::cout and std::cerr
// No need to include <stdexcept> here if RtMidi.h provides RtMidiError,
// and midi_output.h already includes it.

MidiOutput::MidiOutput() {
    try {
        // Explicitly select an API (e.g., WINDOWS_MM for Windows)
        // RtMidi::Api::UNSPECIFIED will allow RtMidi to choose a default.
        // For this adaptation, we'll specify WINDOWS_MM as per the prompt.
        #ifdef _WIN32
            midi_out_ = std::make_unique<RtMidiOut>(RtMidi::Api::WINDOWS_MM);
            std::cout << "MidiOutput: Requested WINDOWS_MM API." << std::endl;
        #else
            // For other OS, let RtMidi pick (or choose another specific API like LINUX_ALSA, MACOSX_CORE)
            midi_out_ = std::make_unique<RtMidiOut>(RtMidi::Api::UNSPECIFIED);
            std::cout << "MidiOutput: Requested UNSPECIFIED API (auto-select)." << std::endl;
        #endif
    } catch (const RtMidiError &error) {
        // Assuming RtMidiError has getMessage()
        std::cerr << "ERROR: MidiOutput Constructor: RtMidiError - " << error.getMessage() << std::endl;
        // midi_out_ will remain nullptr. Subsequent calls should check.
    } catch (const std::exception &e) {
        std::cerr << "ERROR: MidiOutput Constructor: std::exception - " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "ERROR: MidiOutput Constructor: Unknown exception." << std::endl;
    }
}

MidiOutput::~MidiOutput() {
    if (midi_out_ && isPortOpen()) { // isPortOpen itself handles !midi_out_ check
        closePort();
    }
}

bool MidiOutput::openPort(unsigned int portNumber) {
    if (!midi_out_) {
        std::cerr << "ERROR: MidiOutput::openPort: RtMidiOut not initialized (constructor likely failed)." << std::endl;
        return false;
    }
    if (isPortOpen()) { // isPortOpen has its own try-catch
        std::cout << "MidiOutput: Port was already open. Closing first." << std::endl;
        closePort(); // closePort has its own try-catch
    }

    unsigned int portCount = 0;
    try {
        portCount = midi_out_->getPortCount();
    } catch (const RtMidiError &error) {
        std::cerr << "ERROR: MidiOutput::openPort: RtMidiError getting port count - " << error.getMessage() << std::endl;
        return false;
    }

    if (portNumber >= portCount) {
        std::cerr << "ERROR: MidiOutput::openPort: Invalid port number: " << portNumber
                  << ". Available ports: " << portCount << "." << std::endl;
        return false;
    }

    try {
        std::string portNameStr = midi_out_->getPortName(portNumber); // Get name for logging before opening
        std::cout << "MidiOutput: Attempting to open output port " << portNumber << " (" << portNameStr << ")" << std::endl;
        midi_out_->openPort(portNumber, "KronosPlayerOut"); // Port name for the client
        std::cout << "MidiOutput: Successfully opened output port " << portNumber << " (" << portNameStr << ")" << std::endl;
        return true;
    } catch (const RtMidiError &error) {
        std::cerr << "ERROR: MidiOutput::openPort: RtMidiError - " << error.getMessage() << std::endl;
    }
    return false;
}

void MidiOutput::closePort() {
    if (!midi_out_) {
        // std::cerr << "MidiOutput::closePort: RtMidiOut not initialized." << std::endl; // Can be noisy
        return;
    }
    if (isPortOpen()) { // Use our isPortOpen which has try-catch
        std::cout << "MidiOutput: Closing MIDI output port." << std::endl;
        try {
            midi_out_->closePort();
        } catch (const RtMidiError &error) {
            std::cerr << "ERROR: MidiOutput::closePort: RtMidiError - " << error.getMessage() << std::endl;
        }
    }
}

unsigned int MidiOutput::getPortCount() const {
    if (!midi_out_) {
        // std::cerr << "MidiOutput::getPortCount: RtMidiOut not initialized." << std::endl; // Can be noisy
        return 0;
    }
    try {
        return midi_out_->getPortCount();
    } catch (const RtMidiError &error) {
        std::cerr << "ERROR: MidiOutput::getPortCount: RtMidiError - " << error.getMessage() << std::endl;
        return 0;
    }
}

std::string MidiOutput::getPortName(unsigned int portNumber) const {
    if (!midi_out_) {
        // std::cerr << "MidiOutput::getPortName: RtMidiOut not initialized." << std::endl; // Can be noisy
        return "[Error: Not Initialized]";
    }
    try {
        return midi_out_->getPortName(portNumber);
    } catch (const RtMidiError &error) {
        std::cerr << "ERROR: MidiOutput::getPortName for port " << portNumber << ": RtMidiError - " << error.getMessage() << std::endl;
        return "[Error: RtMidiError]";
    }
}

bool MidiOutput::isPortOpen() const {
    if (!midi_out_) {
        return false;
    }
    try {
        return midi_out_->isPortOpen();
    } catch (const RtMidiError &error) {
        std::cerr << "ERROR: MidiOutput::isPortOpen: RtMidiError - " << error.getMessage() << std::endl;
        return false; // Assume not open if error occurs
    }
}

void MidiOutput::sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (!isPortOpen()) return; // isPortOpen has logging and checks midi_out_
    if (channel > 15 || note > 127 || velocity > 127) {
        std::cerr << "ERROR: MidiOutput::sendNoteOn: Invalid MIDI parameter." << std::endl;
        return;
    }
    std::vector<unsigned char> message = {static_cast<unsigned char>(0x90 | channel), note, velocity};
    try {
        midi_out_->sendMessage(&message);
    } catch (const RtMidiError &error) {
        std::cerr << "ERROR: MidiOutput::sendNoteOn: RtMidiError - " << error.getMessage() << std::endl;
    }
}

void MidiOutput::sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (!isPortOpen()) return;
    if (channel > 15 || note > 127 || velocity > 127) {
        std::cerr << "ERROR: MidiOutput::sendNoteOff: Invalid MIDI parameter." << std::endl;
        return;
    }
    std::vector<unsigned char> message = {static_cast<unsigned char>(0x80 | channel), note, velocity};
    try {
        midi_out_->sendMessage(&message);
    } catch (const RtMidiError &error) {
        std::cerr << "ERROR: MidiOutput::sendNoteOff: RtMidiError - " << error.getMessage() << std::endl;
    }
}

void MidiOutput::sendProgramChange(uint8_t channel, uint8_t programNumber) {
    if (!isPortOpen()) return;
    if (channel > 15 || programNumber > 127) {
        std::cerr << "ERROR: MidiOutput::sendProgramChange: Invalid MIDI parameter." << std::endl;
        return;
    }
    std::vector<unsigned char> message = {static_cast<unsigned char>(0xC0 | channel), programNumber};
    try {
        midi_out_->sendMessage(&message);
    } catch (const RtMidiError &error) {
        std::cerr << "ERROR: MidiOutput::sendProgramChange: RtMidiError - " << error.getMessage() << std::endl;
    }
}

void MidiOutput::sendControlChange(uint8_t channel, uint8_t controllerNumber, uint8_t value) {
    if (!isPortOpen()) return;
    if (channel > 15 || controllerNumber > 127 || value > 127) {
        std::cerr << "ERROR: MidiOutput::sendControlChange: Invalid MIDI parameter." << std::endl;
        return;
    }
    std::vector<unsigned char> message = {static_cast<unsigned char>(0xB0 | channel), controllerNumber, value};
    try {
        midi_out_->sendMessage(&message);
    } catch (const RtMidiError &error) {
        std::cerr << "ERROR: MidiOutput::sendControlChange: RtMidiError - " << error.getMessage() << std::endl;
    }
}

// setupTimbre remains largely the same, but relies on the above methods which now have error handling
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
