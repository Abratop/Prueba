# Korg Kronos Combi Player (GUI Edition)

A GUI application for parsing and playing Korg Kronos `.PCG` (Program, Combi, Global) files, specifically focusing on Combi data. It features MIDI output for up to 16 timbres per Combi, an integrated arpeggiator, and a graphical user interface built with ImGui.

## Features

*   Graphical User Interface (GUI) for all major operations.
*   Parses Korg Kronos `.PCG` files to extract Combi program data.
*   MIDI output for up to 16 timbres per Combi, mapping each timbre to a corresponding MIDI channel (0-15).
*   Integrated MIDI arpeggiator with selectable patterns (UP, DOWN, UP/DOWN, RANDOM), octave control, and adjustable rate.
*   Selection of MIDI Input and Output ports via the GUI.
*   Basic pass-through of MIDI messages for non-arpeggiator channels.

## Dependencies

*   **C++11 Compatible Compiler:** (e.g., g++ for MinGW-w64, MSVC for Visual Studio).
*   **RtMidi Library (Full Version):**
    *   Handles real-time MIDI I/O.
    *   Source code needs to be obtained from [https://github.com/thestk/rtmidi](https://github.com/thestk/rtmidi).
    *   Place the RtMidi source files (e.g., `RtMidi.h`, `RtMidi.cpp`, and platform-specific files like `RtMidiWinMM.cpp` for Windows) into the `libs/rtmidi/` directory, replacing any previous stub files.
*   **ImGui Library:**
    *   Used for the graphical user interface.
    *   Source code available at [https://github.com/ocornut/imgui](https://github.com/ocornut/imgui).
    *   Core ImGui files (e.g., `imgui.h`, `imgui.cpp`, `imgui_draw.cpp`, `imgui_widgets.cpp`, `imgui_tables.cpp`) should be placed in `libs/imgui/`.
    *   Backend files for GLFW and OpenGL3 (e.g., `imgui_impl_glfw.h`, `imgui_impl_glfw.cpp`, `imgui_impl_opengl3.h`, `imgui_impl_opengl3.cpp`) should be placed in `libs/imgui/backends/`.
*   **GLFW:**
    *   Used for creating windows, OpenGL contexts, and handling input.
    *   Version 3.3+ recommended. Download from [https://www.glfw.org/](https://www.glfw.org/).
    *   Headers (e.g., `GLFW/glfw3.h`) need to be accessible in your include path.
    *   The GLFW library file (e.g., `glfw3.lib` for MSVC, `libglfw3.a` for MinGW) needs to be in your library path for linking.
*   **GLAD (OpenGL Loader):**
    *   Used to load OpenGL function pointers at runtime.
    *   Generate for OpenGL 3.3+ Core profile from the GLAD web service: [https://glad.dav1d.de/](https://glad.dav1d.de/).
    *   Place `glad.h` under a `glad/` directory within your include paths (e.g., `libs/glad/include/glad/glad.h`).
    *   Compile the generated `glad.c` file (e.g., `libs/glad/src/glad.c`) with your project.
*   **OpenGL 3.3+ Drivers:** Your system must have graphics drivers that support OpenGL 3.3 or higher.

## File Structure (Key Files)

*   `main.cpp`: Main application entry point, GUI rendering loop, ImGui/GLFW/GLAD setup.
*   `kronos_combi_player.h/.cpp`: Core player logic, managing MIDI I/O, PCG data, and arpeggiator state.
*   `arpeggiator.h/.cpp`: The arpeggiator engine.
*   `pcg_parser.h/.cpp`, `pcg_structures.h`: Logic for parsing Korg Kronos PCG files.
*   `midi_input.h/.cpp`, `midi_output.h/.cpp`: MIDI input/output handling wrappers (using RtMidi).
*   `libs/rtmidi/`: Directory intended to contain the full RtMidi source files.
*   `libs/imgui/`: Directory intended to contain ImGui core and backend source files.
*   `libs/glad/`: Directory intended to contain GLAD loader files (`include/glad/glad.h` and `src/glad.c`).

## Compilation

*   **General Notes:** Compilation requires correctly configuring include paths for all dependencies (RtMidi, ImGui, GLFW, GLAD) and library paths for GLFW. The exact commands can vary based on your system and compiler setup.
*   **Windows with MinGW-w64 (g++ example):**
    ```bash
    g++ main.cpp kronos_combi_player.cpp arpeggiator.cpp pcg_parser.cpp midi_input.cpp midi_output.cpp \
        libs/rtmidi/RtMidi.cpp libs/rtmidi/RtMidiWinMM.cpp \
        libs/imgui/imgui.cpp libs/imgui/imgui_draw.cpp libs/imgui/imgui_widgets.cpp libs/imgui/imgui_tables.cpp \
        libs/imgui/backends/imgui_impl_glfw.cpp libs/imgui/backends/imgui_impl_opengl3.cpp \
        libs/glad/src/glad.c \
        -o KronosCombiPlayer.exe -std=c++11 -pthread -Wall -Wextra \
        -I./libs/imgui -I./libs/imgui/backends -I./libs/glad/include -I<path_to_glfw_include> -I./libs/rtmidi \
        -L<path_to_glfw_lib_directory> \
        -static-libgcc -static-libstdc++ -lglfw3 -lopengl32 -lgdi32 -lwinmm
    ```
    *   Replace `<path_to_glfw_include>` and `<path_to_glfw_lib_directory>` with the actual paths to your GLFW installation.
    *   This command assumes RtMidi source files are directly in `libs/rtmidi/` and GLAD source in `libs/glad/src/`. Adjust paths as needed.
*   **Windows with Visual Studio (MSVC):**
    1.  Create a new C++ Project (e.g., "Empty Project" or "Desktop Application").
    2.  Add all `.cpp` files from the project root, `libs/rtmidi/`, `libs/imgui/`, `libs/imgui/backends/`, and `libs/glad/src/` to your project's source files.
    3.  Configure Include Directories (Project Properties -> VC++ Directories -> Include Directories):
        *   Path to ImGui core (`libs/imgui/`)
        *   Path to ImGui backends (`libs/imgui/backends/`)
        *   Path to GLAD headers (`libs/glad/include/`)
        *   Path to GLFW headers (e.g., `path/to/glfw/include`)
        *   Path to RtMidi headers (`libs/rtmidi/`)
    4.  Configure Library Directories (Project Properties -> VC++ Directories -> Library Directories):
        *   Path to GLFW `.lib` file (e.g., `path/to/glfw/lib-vcXXXX`).
    5.  Configure Additional Dependencies (Project Properties -> Linker -> Input -> Additional Dependencies):
        *   `glfw3.lib`
        *   `opengl32.lib`
        *   `gdi32.lib` (Usually inherited)
        *   `winmm.lib` (For RtMidi WinMM API)
    6.  Ensure C++ Language Standard is C++11 or later (usually C++14 or C++17 by default in modern VS).
    7.  Define necessary preprocessor symbols if RtMidi or GLFW require them (e.g., `__WINDOWS_MM__` for RtMidi, though RtMidi usually defines this itself).
    8.  Build the solution.

## Usage

1.  **Running:** Execute the compiled application (e.g., `KronosCombiPlayer.exe`).
2.  **MIDI Setup:**
    *   The application window will appear. In the "MIDI Setup" section:
    *   Select your desired MIDI Input port from the "MIDI Input" dropdown.
    *   Select your desired MIDI Output port from the "MIDI Output" dropdown.
    *   Click the "Connect MIDI Ports" button. Status messages will be logged (likely to console if launched from one, or a future GUI log window).
3.  **Load PCG File:**
    *   In the "PCG File" section, type or paste the full path to your Korg Kronos `.PCG` file into the input field.
    *   Click the "Load PCG File" button. The "Combis" list will update.
4.  **Select Combi:**
    *   If Combis are loaded successfully, they will appear in the "Combis" list box.
    *   Click on a Combi name to select it. The selected Combi's timbres will be configured on MIDI channels 0-15.
5.  **Arpeggiator Controls:**
    *   In the "Arpeggiator" section (active after a Combi is selected):
    *   Use the "Enable Arpeggiator" checkbox to turn the arpeggiator on or off.
    *   Configure the arpeggio pattern (Up, Down, etc.), rate (in notes per second), and octave range using the respective controls.
6.  **Playing:**
    *   **Arpeggiator:** MIDI Note On/Off messages sent to the `arp_target_channel_` (typically the channel of the first active timbre of the selected Combi) will trigger and control the arpeggiator. Arpeggiated notes will be output on the `arp_output_channel_` (also typically the first active timbre's channel by default).
    *   **Direct Pass-Through:** MIDI messages on other channels (not the `arp_target_channel_`) will be passed through to their corresponding output MIDI channel if that timbre is active in the selected Combi.
7.  **Quitting:** Close the application window. This will trigger a shutdown sequence for MIDI ports and other resources.

## MIDI Implementation Notes

*   **CRITICAL: Bank Select Mapping:** The MIDI Program Change messages sent when a Combi is selected are accompanied by Bank Select Control Change messages (CC0 for MSB, CC32 for LSB). The values currently used for these Bank Select messages in `MidiOutput::setupTimbre()` are **placeholders** (e.g., MSB=0, LSB derived from `timbre.program_bank_msb`). For correct sound selection on your Korg Kronos or other synthesizer, you **must** consult your synthesizer's MIDI Implementation Chart and update this logic in the `MidiOutput::setupTimbre()` method in `midi_output.cpp`.
*   **Timbre Status:** The application only activates timbres for MIDI output if their status in the PCG file is marked as 'INT' (Internal, typically status value `0x21`). Other timbre statuses (Off, EXT, EX2) will result in their channels being silenced (All Notes Off, Volume 0 sent).

## Known Issues & Limitations

*   **Arpeggiator Gate Time:** The arpeggiator has a `setGate()` function and a `gate_` member. However, the current `tick()` logic results in a fixed, short gate time for arpeggiated notes (the previous note off is sent immediately before the current note on). A true variable gate implementation requires more complex timer scheduling for individual note-off events.
*   **PCG File Errors:** While the parser attempts to skip unknown data chunks, robust parsing for all PCG file variations or heavily corrupted files is not guaranteed.
*   **No Native File Dialog:** The PCG file path must be manually entered into the text field. A native OS file dialog is not yet implemented.
*   **Arpeggiator Channel Configuration:** While the arpeggiator target and output channels are determined (usually from the first active timbre), detailed GUI controls for manually re-assigning these are not yet present.
*   **Logging to Console:** Most detailed log messages (port connections, errors, MIDI events) are still primarily output to the console if the application is launched from one. A dedicated log window within the GUI is not implemented.
*   **Real-time Parameter Changes:** Some parameters, once set (like arpeggiator target/output channels after combi selection), might not be dynamically reconfigurable without re-selecting the combi or restarting, depending on current GUI capabilities.
