#include "RtMidi.h"
#include <iostream>
#include <vector>
#include <string>

// RtMidiOut class implementation (stub)
RtMidiOut::RtMidiOut( RtMidi::Api /*api*/ ) {
    // std::cout << "RtMidiOut created (stub)" << std::endl;
}

RtMidiOut::~RtMidiOut() {
    // std::cout << "RtMidiOut destroyed (stub)" << std::endl;
}

unsigned int RtMidiOut::getPortCount() {
    // std::cout << "RtMidiOut::getPortCount() called (stub)" << std::endl;
    return 1; // Stub: Pretend there's one port
}

std::string RtMidiOut::getPortName(unsigned int portNumber) {
    // std::cout << "RtMidiOut::getPortName(" << portNumber << ") called (stub)" << std::endl;
    if (portNumber == 0) {
        return "Stub MIDI Output Port"; // Stub
    }
    return "";
}

void RtMidiOut::openPort(unsigned int portNumber, const std::string &/*name*/) {
    // std::cout << "RtMidiOut::openPort(" << portNumber << ") called (stub)" << std::endl;
    // In a real implementation, this would set up the port.
    // For the stub, we can just pretend it's always successful if port 0.
    if (apiData_ == nullptr && portNumber == 0) { // Simulate opening a port
        apiData_ = reinterpret_cast<void*>(0x1); // Non-null indicates "open" for isPortOpen
    }
}

void RtMidiOut::openVirtualPort(const std::string &/*name*/) {
    // std::cout << "RtMidiOut::openVirtualPort() called (stub)" << std::endl;
    // Stub: Similar to openPort, mark as "open"
    if (apiData_ == nullptr) {
         apiData_ = reinterpret_cast<void*>(0x1); // Non-null indicates "open"
    }
}

void RtMidiOut::closePort() {
    // std::cout << "RtMidiOut::closePort() called (stub)" << std::endl;
    apiData_ = nullptr; // Mark as "closed"
}

bool RtMidiOut::isPortOpen() const {
    // std::cout << "RtMidiOut::isPortOpen() called (stub)" << std::endl;
    return apiData_ != nullptr; // "Open" if apiData_ is not null
}

void RtMidiOut::sendMessage(const std::vector<unsigned char> *message) {
    if (!message || message->empty()) return;
    // std::cout << "RtMidiOut::sendMessage(vector) called (stub) for " << message->size() << " bytes. First byte: 0x" << std::hex << (int)((*message)[0]) << std::dec << std::endl;
    // In a real implementation, this would send the MIDI message.
}

void RtMidiOut::sendMessage(const unsigned char *message, size_t size ) {
    if (!message || size == 0) return;
    // std::cout << "RtMidiOut::sendMessage(ptr,size) called (stub) for " << size << " bytes. First byte: 0x" << std::hex << (int)(message[0]) << std::dec << std::endl;
}


// RtMidiIn class implementation (stub)
RtMidiIn::RtMidiIn( RtMidi::Api /*api*/, const std::string& /*clientName*/, unsigned int /*queueSizeLimit*/ ) {
    // std::cout << "RtMidiIn created (stub)" << std::endl;
}

RtMidiIn::~RtMidiIn() {
    // std::cout << "RtMidiIn destroyed (stub)" << std::endl;
    apiData_ = nullptr; // Ensure closed state
}

unsigned int RtMidiIn::getPortCount() {
    // std::cout << "RtMidiIn::getPortCount() called (stub)" << std::endl;
    return 1; // Stub: Pretend there's one port
}

std::string RtMidiIn::getPortName(unsigned int portNumber) {
    // std::cout << "RtMidiIn::getPortName(" << portNumber << ") called (stub)" << std::endl;
    if (portNumber == 0) {
        return "Stub MIDI Input Port"; // Stub
    }
    return "";
}

void RtMidiIn::openPort(unsigned int portNumber, const std::string &/*name*/) {
    // std::cout << "RtMidiIn::openPort(" << portNumber << ") called (stub)" << std::endl;
    if (apiData_ == nullptr && portNumber == 0) { // Simulate opening a port
        apiData_ = reinterpret_cast<void*>(0x1); // Non-null indicates "open"
    }
}

void RtMidiIn::openVirtualPort(const std::string &/*name*/) {
    // std::cout << "RtMidiIn::openVirtualPort() called (stub)" << std::endl;
    if (apiData_ == nullptr) {
         apiData_ = reinterpret_cast<void*>(0x1); // Non-null indicates "open"
    }
}

void RtMidiIn::closePort() {
    // std::cout << "RtMidiIn::closePort() called (stub)" << std::endl;
    apiData_ = nullptr; // Mark as "closed"
    userCallback_ = nullptr;
    userData_ = nullptr;
}

bool RtMidiIn::isPortOpen() const {
    // std::cout << "RtMidiIn::isPortOpen() called (stub)" << std::endl;
    return apiData_ != nullptr;
}

void RtMidiIn::setCallback(RtMidiCallback callback, void *userData) {
    // std::cout << "RtMidiIn::setCallback() called (stub)" << std::endl;
    if (apiData_) { // Port should be open
        userCallback_ = callback;
        userData_ = userData;
    }
}

void RtMidiIn::cancelCallback() {
    // std::cout << "RtMidiIn::cancelCallback() called (stub)" << std::endl;
    userCallback_ = nullptr;
    userData_ = nullptr;
}

void RtMidiIn::ignoreTypes(bool /*midiSysex*/, bool /*midiTime*/, bool /*midiSense*/) {
    // std::cout << "RtMidiIn::ignoreTypes() called (stub)" << std::endl;
    // In a real implementation, this would configure message filtering.
}

double RtMidiIn::getMessage( std::vector<unsigned char> *message ) {
    // std::cout << "RtMidiIn::getMessage() called (stub) - not for callback usage" << std::endl;
    if (message) message->clear();
    // This stub is for polling mode, which we are not primarily using if using callbacks.
    // However, a basic implementation might be useful for testing.
    // To simulate a message for testing polling:
    // if (firstMessage_) {
    //    message->push_back(0x90); message->push_back(60); message->push_back(100);
    //    firstMessage_ = false;
    //    return 0.0; // deltaTime
    // }
    return 0.0;
}

// Other RtMidi methods (like getApiDisplayName, etc.) could be stubbed if needed.
