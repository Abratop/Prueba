# Python GUI Configuration Tool Guide for Teensy EVI

## 1. Introduction
This guide provides a starting point for developing a Python-based Graphical User Interface (GUI) to configure the Teensy EVI firmware in real-time. The EVI firmware includes a serial communication protocol (detailed in the main `README.md`) that this Python application will use.

This guide outlines a suggested application structure, recommended libraries, and provides functional Python code examples for serial communication and a basic Tkinter GUI.

## 2. Prerequisites
- Python 3.x installed on your system.
- `pip` (Python package installer) available.
- The Teensy EVI firmware (including the serial protocol features) flashed and running on your Teensy 4.1.

## 3. Key Python Libraries

### 3.1. `pyserial` (for Serial Communication)
This library is essential for communicating with the Teensy EVI over its USB serial connection.
- **Installation:**
  ```bash
  pip install pyserial
  ```
- **Documentation:** [https://pyserial.readthedocs.io/](https://pyserial.readthedocs.io/)

### 3.2. GUI Libraries (Choose one)

#### a) Tkinter (Recommended for Simplicity)
Tkinter is Python's standard built-in GUI library. It's suitable for creating simple to moderately complex interfaces without requiring external dependencies beyond your Python installation.
- **Pros:** Built-in, easy to start with for basic UIs.
- **Cons:** Can look dated without theming; complex layouts require more effort.

#### b) PyQt (e.g., PyQt6 or PyQt5 - For Richer UIs)
PyQt provides Python bindings for the powerful Qt application framework. It allows for modern, feature-rich, and highly customizable UIs.
- **Pros:** Extensive widget set, strong layout tools, modern look and feel.
- **Cons:** Steeper learning curve, requires separate installation, licensing (GPL or commercial).
- **Installation (PyQt6):**
  ```bash
  pip install PyQt6
  ```

This guide will use **Tkinter** for its GUI examples due to its simplicity and built-in nature.

## 4. Proposed Application Structure
A modular approach is recommended:
- **`serial_handler.py`:** Manages all serial communication with the EVI.
- **`evi_gui_tkinter.py` (or `app.py`):** Contains the main GUI logic and user interface elements.

## 5. Serial Communication (`serial_handler.py`)
This module will contain a class (e.g., `EVICommunicator`) to handle sending commands to the EVI and parsing its responses.

```python
# Placeholder for serial_handler.py
#
# The full Python code for this module, as described in the plan "Provide Python Code Examples for Serial Communication,"
# should be inserted here by the user.
#
# This class (e.g., EVICommunicator) should include methods for:
#   - Listing available serial ports.
#   - Connecting to a specified serial port with a given baud rate (115200 for EVI).
#   - Disconnecting from the serial port.
#   - Sending a command string to the EVI and waiting for a response.
#   - Handling potential timeouts if the EVI doesn't respond.
#   - Parsing responses (e.g., splitting multi-line responses, extracting values).
#   - Convenience methods for common EVI commands like:
#       - get_setting(setting_name)
#       - set_setting(setting_name, value)
#       - list_settings()
#       - save_config()
#       - load_config()
#
# Example (Conceptual - replace with actual implementation):
#
# import serial
# import time
#
# class EVICommunicator:
#     def __init__(self):
#         self.ser = None
#
#     def connect(self, port, baudrate=115200, timeout=1):
#         try:
#             self.ser = serial.Serial(port, baudrate, timeout=timeout)
#             time.sleep(2) # Wait for connection to establish
#             if self.ser.is_open:
#                 self.ser.flushInput()
#                 self.ser.flushOutput()
#                 return True, "Connected"
#             else:
#                 return False, "Failed to open port"
#         except serial.SerialException as e:
#             return False, str(e)
#
#     def disconnect(self):
#         if self.ser and self.ser.is_open:
#             self.ser.close()
#         return True, "Disconnected"
#
#     def send_command(self, command_str):
#         if self.ser and self.ser.is_open:
#             try:
#                 self.ser.write(command_str.encode('utf-8') + b'\n')
#                 response_lines = []
#                 # Read until a timeout or a specific end-of-response marker if EVI firmware provides one
#                 # For simplicity, reading with timeout or assuming EVI sends OK/ERROR quickly
#                 while True:
#                     line = self.ser.readline().decode('utf-8').strip()
#                     if line:
#                         response_lines.append(line)
#                     else: # Timeout or empty line
#                         break
#                 return True, response_lines
#             except Exception as e:
#                 return False, [str(e)]
#         return False, ["Not connected"]
#
#     # Add more methods like get_setting, set_setting etc.
#
```

**Key features of `EVICommunicator`:**
- Lists available serial ports.
- Connects/disconnects from the EVI.
- Sends commands (e.g., `GET MIDI_CHANNEL`, `SET OCTAVE_SHIFT -1`) and retrieves responses.
- Includes basic error handling and timeouts.
- Provides convenience methods for common EVI commands.

## 6. Basic GUI with Tkinter (`evi_gui_tkinter.py`)
This script demonstrates how to build a simple GUI that uses the `EVICommunicator` to interact with the EVI.

```python
# Placeholder for evi_gui_tkinter.py
#
# The full Python code for this module, as described in the plan "Illustrate Basic GUI Integration Concepts (e.g., with Tkinter),"
# should be inserted here by the user.
#
# This script should demonstrate:
#   - Importing the EVICommunicator from serial_handler.py.
#   - Creating a main Tkinter window.
#   - UI elements for:
#       - Serial port selection (e.g., Combobox populated with available ports).
#       - Connect/Disconnect button.
#       - A section to display/edit EVI settings (e.g., using Labels and Entry widgets for each setting).
#           - MIDI Channel
#           - Octave Shift
#           - Transpose Shift
#           - Fingering Mode (perhaps a Combobox or Radiobuttons)
#           - PolySelect Mode (Combobox or Radiobuttons)
#           - Breath Threshold
#           - Breath Interval
#       - Buttons for GET, SET, LIST_SETTINGS, SAVE_CONFIG, LOAD_CONFIG for individual or all settings.
#       - A text area (e.g., ScrolledText) for logging commands sent and responses received.
#   - Functions to handle button clicks, which would:
#       - Call appropriate methods on the EVICommunicator instance.
#       - Update the GUI with responses or new settings.
#   - A basic layout of these UI elements.
#
# Example (Conceptual - replace with actual implementation):
#
# import tkinter as tk
# from tkinter import ttk, scrolledtext, messagebox
# from serial_handler import EVICommunicator # Assuming EVICommunicator is in serial_handler.py
# import serial.tools.list_ports
#
# class EVIGUIApp:
#     def __init__(self, root):
#         self.root = root
#         self.root.title("EVI Configuration Tool")
#         self.evi_comm = EVICommunicator()
#
#         # Serial Port Selection
#         # ... (code for port selection Combobox, connect/disconnect buttons) ...
#
#         # Settings Area (example for one setting)
#         # ... (code for labels, entry fields for MIDI_CHANNEL, etc.) ...
#         # ... (buttons for GET MIDI_CHANNEL, SET MIDI_CHANNEL) ...
#
#         # Global Actions
#         # ... (buttons for LIST_SETTINGS, SAVE_CONFIG, LOAD_CONFIG) ...
#
#         # Log Area
#         # self.log_area = scrolledtext.ScrolledText(root, height=10)
#         # self.log_area.pack(pady=10, padx=10, fill=tk.BOTH, expand=True)
#
#     def log_message(self, message):
#         # self.log_area.insert(tk.END, message + "\n")
#         # self.log_area.see(tk.END)
#         print(message) # Simple print for placeholder
#
#     # ... (implement functions for connect, disconnect, refresh_ports, send_command, get_setting, set_setting etc.)
#
# if __name__ == "__main__":
#     root = tk.Tk()
#     app = EVIGUIApp(root)
#     root.mainloop()
#
```

**Key Tkinter concepts illustrated:**
- Creating a main window and frames.
- Using widgets like `Label`, `Button`, `Combobox` (for port selection), `Entry` (for input), and `ScrolledText` (for logging).
- Using `StringVar` to link Python variables with Tkinter widgets.
- Basic layout using `pack()` or `grid()`.
- Handling button clicks to trigger actions (like connecting or sending commands).
- Displaying responses from the EVI in a log area.

**Extending the GUI:**
- **More Settings:** Add more input fields, labels, and buttons for each configurable EVI setting (Octave, Transpose, Fingering Mode, PolySelect Mode, Thresholds).
- **Populating Fields:** When connecting or after a `LOAD_CONFIG`, use the `LIST_SETTINGS` command. Parse its output to populate all the input fields in your GUI with the EVI's current settings.
- **Input Validation:** Add more robust validation for user inputs before sending `SET` commands.
- **Status Bar:** A status bar can provide feedback on connection status or last action.
- **Error Handling:** Display errors from `EVICommunicator` more prominently using `messagebox`.

## 7. Running the Example
1. Save the `EVICommunicator` code as `serial_handler.py`.
2. Save the Tkinter GUI code as `evi_gui_tkinter.py` in the same directory.
3. Ensure your Teensy EVI is programmed with the latest firmware and connected to your PC.
4. Identify the correct serial port for your Teensy (e.g., from the Arduino IDE or the "Refresh Ports" button in the example GUI).
5. Run the Tkinter application from your terminal:
   ```bash
   python evi_gui_tkinter.py
   ```
6. Select the port, connect, and interact with your EVI.

## 8. Further Development
- Implement UI elements for all EVI settings.
- Improve response parsing from `LIST_SETTINGS` to auto-populate all GUI fields.
- Add more robust error handling and user feedback.
- Consider saving the last used serial port in a local configuration file for the Python app.
- Explore advanced GUI features if using a library like PyQt.

This guide and the example code should provide a strong foundation for building your custom EVI configuration tool.
