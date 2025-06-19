#pragma once
#include <vector>
#include <string>
#include <memory> // For std::unique_ptr in MidiOutput, though RtMidi itself doesn't use it in the API

// Define API_WINMM, API_MACOSX_CORE, API_UNIX_ALSA, API_UNIX_JACK, API_DUMMY in a platform specific way if needed
// For stub, not critical.

class RtMidiError : public std::runtime_error
{
public:
  RtMidiError(const std::string& message) : std::runtime_error(message) {}
};

class RtMidi
{
public:
  enum Api {
    UNSPECIFIED,    /*!< Search for a working compiled API. */
    MACOSX_CORE,    /*!< Macintosh OS X CoreMIDI API. */
    LINUX_ALSA,     /*!< The Advanced Linux Sound Architecture API. */
    UNIX_JACK,      /*!< The JACK Low-Latency Audio Server API. */
    WINDOWS_MM,     /*!< The Microsoft Multimedia MIDI API. */
    RTMIDI_DUMMY    /*!< A compilable but non-functional API. */
  };
  RtMidi() {} // Default constructor
  virtual ~RtMidi() {} // Destructor
  virtual unsigned int getPortCount() = 0;
  virtual std::string getPortName(unsigned int portNumber = 0) = 0;
  virtual void openPort( unsigned int portNumber = 0, const std::string &name = std::string( "RtMidi Client" ) ) = 0;
  virtual void openVirtualPort( const std::string &name = std::string( "RtMidi Output" ) ) = 0;
  virtual void closePort() = 0;
  virtual bool isPortOpen() const = 0;
};


class RtMidiOut : public RtMidi
{
public:
    RtMidiOut( RtMidi::Api api = RtMidi::Api::UNSPECIFIED );
    ~RtMidiOut() override; // Make destructor virtual if RtMidi has virtual methods and is base
    unsigned int getPortCount() override;
    std::string getPortName(unsigned int portNumber = 0) override;
    void openPort( unsigned int portNumber = 0, const std::string &name = std::string( "RtMidi Output" ) ) override;
    void openVirtualPort( const std::string &name = std::string( "RtMidi Output" ) ) override;
    void closePort() override;
    bool isPortOpen() const override;
    void sendMessage(const std::vector<unsigned char> *message);
    void sendMessage(const unsigned char *message, size_t size );


private:
  // This is a stub, so no actual implementation details are needed here for private members
  // In real RtMidi, there would be a pointer to internal data (e.g., MidiApi subclass instance)
  void *apiData_{nullptr}; // Placeholder to mimic structure
};

class RtMidiIn : public RtMidi
{
public:
    typedef void (*RtMidiCallback)(double deltaTime, std::vector<unsigned char> *message, void *userData);

    RtMidiIn( RtMidi::Api api = RtMidi::Api::UNSPECIFIED,
              const std::string& clientName = "RtMidi Input Client",
              unsigned int queueSizeLimit = 100);
    ~RtMidiIn() override;

    unsigned int getPortCount() override;
    std::string getPortName(unsigned int portNumber = 0) override;
    void openPort( unsigned int portNumber = 0, const std::string &name = std::string( "RtMidi Input" ) ) override;
    void openVirtualPort( const std::string &name = std::string( "RtMidi Input" ) ) override;
    void closePort() override;
    bool isPortOpen() const override;

    void setCallback(RtMidiCallback callback, void *userData = nullptr);
    void cancelCallback();
    void ignoreTypes(bool midiSysex = true, bool midiTime = true, bool midiSense = true);
    virtual double getMessage( std::vector<unsigned char> *message );


private:
  // This is a stub, so no actual implementation details are needed here for private members
  void *apiData_{nullptr}; // Placeholder to mimic structure
  RtMidiCallback userCallback_{nullptr};
  void *userData_{nullptr};
  bool firstMessage_{true}; // To simulate some behavior if needed
};
