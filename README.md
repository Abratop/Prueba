# Korg Kronos Combi Player

A command-line application to parse and play Korg Kronos `.PCG` (Program, Combi, Global) files, specifically focusing on Combi data. It features MIDI output for up to 16 timbres per Combi, each on its own channel, and includes an integrated arpeggiator.

## Features

*   Parses Korg Kronos `.PCG` files to extract Combi program data.
*   Outputs MIDI for up to 16 timbres per Combi, mapping each timbre to a corresponding MIDI channel (0-15).
*   Integrated MIDI arpeggiator with selectable patterns (UP, DOWN, UP/DOWN, RANDOM), octave control, and adjustable rate.
*   Command-line interface for selecting MIDI input/output ports, loading PCG files, and choosing Combis.
*   Basic pass-through of MIDI messages for non-arpeggiator channels.

## Dependencies

*   C++11 compatible compiler (e.g., g++).
*   **RtMidi library**: Note: The current version in this repository uses STUB implementations of RtMidi (`libs/rtmidi/`) for development and testing purposes without actual MIDI hardware. For real MIDI hardware interaction, you would need to replace these stubs with the full RtMidi library source or link against a pre-compiled RtMidi library.
*   Standard C++ libraries (iostream, vector, string, thread, atomic, functional, algorithm, random, limits, csignal).

## File Structure

*   `main.cpp`: Main application entry point and command-line interface logic.
*   `kronos_combi_player.h/.cpp`: Core player logic, managing MIDI I/O, PCG data processing, and the arpeggiator.
*   `arpeggiator.h/.cpp`: The arpeggiator engine.
*   `pcg_parser.h/.cpp`: Korg Kronos PCG file parsing logic.
*   `pcg_structures.h`: C++ structures representing the layout of data within PCG files.
*   `midi_input.h/.cpp`: MIDI input handling wrapper around RtMidi.
*   `midi_output.h/.cpp`: MIDI output handling wrapper around RtMidi.
*   `libs/rtmidi/RtMidi.h, RtMidi.cpp`: RtMidi library files (currently stub implementations).

## Compilation

An example g++ command to compile the application:

```bash
g++ main.cpp kronos_combi_player.cpp arpeggiator.cpp pcg_parser.cpp midi_input.cpp midi_output.cpp libs/rtmidi/RtMidi.cpp -o KronosCombiPlayer -std=c++11 -pthread -Wall -Wextra
```

*   `-std=c++11`: Ensures C++11 standard.
*   `-pthread`: Necessary for `std::thread` (used by the arpeggiator's timing mechanism).
*   `-Wall -Wextra`: Recommended flags to enable most compiler warnings for good practice.

## Usage

1.  **Running:**
    Execute the compiled application from your terminal:
    ```bash
    ./KronosCombiPlayer
    ```
    Optionally, you can provide the path to a `.PCG` file as a command-line argument:
    ```bash
    ./KronosCombiPlayer /path/to/your/file.PCG
    ```

2.  **Initial Setup:**
    *   **MIDI Ports:** The application will first list available MIDI output ports and prompt you to select one by number. Then, it will list available MIDI input ports and prompt for selection.
    *   **PCG File:** If not provided as a command-line argument, you will be prompted to enter the full path to a Korg Kronos `.PCG` file.

3.  **Combi Selection:**
    *   If the PCG file is loaded successfully, a list of Combis found in the file will be displayed with their respective indices.
    *   Enter the index of the Combi you wish to load and play.

4.  **Playing:**
    *   Once a Combi is selected, its active internal timbres are configured on MIDI channels 0-15. The application sends Program Change and placeholder Bank Select messages for each active timbre.
    *   **Arpeggiator:**
        *   The application will log the MIDI channel designated as the `arp_target_channel_` (input for the arpeggiator) and `arp_output_channel_` (where arpeggiated notes are sent). By default, these are set to the channel of the first active timbre in the selected Combi.
        *   MIDI Note On/Off messages sent to the `arp_target_channel_` will control the notes held and processed by the arpeggiator.
        *   Arpeggiated notes will be output on the `arp_output_channel_`.
    *   **Direct Pass-Through:** MIDI Note On/Off and Control Change messages on other channels (not the `arp_target_channel_`) will be passed through to the corresponding output channel if that timbre is active in the selected Combi.

5.  **Quitting:**
    *   Press `Ctrl+C` in the terminal to gracefully shut down the application. MIDI ports will be closed, and any active arpeggiator threads will be stopped.

## MIDI Implementation Notes

*   **CRITICAL: Bank Select Mapping:** The MIDI Program Change messages sent when a Combi is selected are accompanied by Bank Select Control Change messages (CC0 for MSB, CC32 for LSB). Currently, the values used for these Bank Select messages in `MidiOutput::setupTimbre()` are **placeholders** (e.g., MSB=0, LSB derived from `timbre.program_bank_msb`). For correct sound selection on your Korg Kronos or other synthesizer, you **must** consult your synthesizer's MIDI Implementation Chart and update this logic in the `MidiOutput::setupTimbre()` method in `midi_output.cpp`.
*   **Timbre Status:** The application only activates timbres for MIDI output if their status in the PCG file is marked as 'INT' (Internal, typically status value `0x21`). Other timbre statuses (Off, EXT, EX2) will result in their channels being silenced (All Notes Off, Volume 0).

## Known Issues & Limitations

*   **RtMidi Stubs:** The included RtMidi files (`libs/rtmidi/`) are minimal stub implementations. They log MIDI actions to the console instead of interacting with actual MIDI hardware or system MIDI services. To use this application with a synthesizer or virtual instruments, you must replace these stub files with the full RtMidi source code or link against a system-installed RtMidi library during compilation.
*   **Arpeggiator Gate Time:** The arpeggiator has a `setGate()` function and a `gate_` member. However, the current `tick()` logic in `Arpeggiator.cpp` results in a fixed, short gate time for arpeggiated notes (the previous note off is sent immediately before the current note on). A true variable gate implementation would require more complex timer scheduling for individual note-off events, which is not currently implemented.
*   **PCG File Format Variations:** The PCG parser is based on available specifications and examples. It attempts to be robust by skipping unknown data chunks, but it may not correctly parse all PCG file variations or handle heavily corrupted files perfectly.
*   **Arpeggiator Parameter Configuration:** Arpeggiator parameters (pattern, rate, octaves) are currently set to default values in `KronosCombiPlayer::initialize()`. They are not configurable at runtime via the command-line interface in this version. Future enhancements could add CLI commands or a configuration file for this.
*   **Signal Handling:** Ctrl+C handling is basic. In some terminal environments or under specific conditions, it might not shut down MIDI resources perfectly cleanly, though it attempts to do so.
