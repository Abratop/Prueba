#include "libs/glad/include/glad/glad.h" // Should be first among GL headers
#include <GLFW/glfw3.h> // Needs to be included after GLAD or configured not to include gl.h
                       // For stubs, order might be less critical but good practice

#include "libs/imgui/imgui.h"
#include "libs/imgui/backends/imgui_impl_glfw.h"
#include "libs/imgui/backends/imgui_impl_opengl3.h"

#include "kronos_combi_player.h" // Player logic
#include <iostream>
#include <string>
#include <vector>
#include <limits>   // For std::numeric_limits
#include <csignal>  // For signal handling

// Global variables / Forward Declarations
GLFWwindow* g_window = nullptr;
KronosCombiPlayer g_player; // Global player instance for simplicity in GUI
volatile sig_atomic_t g_signal_flag = 0; // For Ctrl+C from console (less relevant in GUI app)

// GUI State variables
static int g_selected_midi_in_port = -1;
static int g_selected_midi_out_port = -1;
static char g_pcg_file_path_buffer[256] = "";
static int g_selected_combi_index = -1;
static bool g_arp_enabled_checkbox = false;
static int g_arp_pattern_combo_idx = 0; // Corresponds to ArpPattern enum
static float g_arp_rate_slider = 4.0f;
static int g_arp_octaves_slider = 1;


// GLFW error callback
static void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// GLFW framebuffer size callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window; // Unused parameter
    glViewport(0, 0, width, height);
}

// GUI Rendering Function
void renderAppGui(KronosCombiPlayer& player) {
    ImGui::Begin("Kronos Combi Player", nullptr, ImGuiWindowFlags_MenuBar);

    // Menu Bar (Example)
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load PCG...", "Ctrl+O")) { /* TODO: Implement file dialog or use input text */ }
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                if(g_window) glfwSetWindowShouldClose(g_window, true);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // Section: MIDI Port Setup
    ImGui::SeparatorText("MIDI Setup");
    if (!player.isInitialized()) {
        const auto& in_ports = player.getMidiInputPortNames();
        const auto& out_ports = player.getMidiOutputPortNames();

        // Convert vector<string> to const char* array for ImGui::Combo
        std::vector<const char*> in_port_c_strs;
        for(const auto& name : in_ports) in_port_c_strs.push_back(name.c_str());
        std::vector<const char*> out_port_c_strs;
        for(const auto& name : out_ports) out_port_c_strs.push_back(name.c_str());

        if (!in_ports.empty()) {
            ImGui::Combo("MIDI Input", &g_selected_midi_in_port, in_port_c_strs.data(), in_port_c_strs.size());
        } else {
            ImGui::Text("No MIDI Input Ports Found.");
        }
        if (!out_ports.empty()) {
            ImGui::Combo("MIDI Output", &g_selected_midi_out_port, out_port_c_strs.data(), out_port_c_strs.size());
        } else {
            ImGui::Text("No MIDI Output Ports Found.");
        }

        if (ImGui::Button("Connect MIDI Ports") && g_selected_midi_in_port != -1 && g_selected_midi_out_port != -1) {
            if (player.openMidiPorts(g_selected_midi_in_port, g_selected_midi_out_port)) {
                std::cout << "GUI: MIDI Ports opened successfully." << std::endl;
                // Arp target/output channels might be set here or when a combi loads
                player.setArpTargetChannel(0); // Default for now
                player.setArpOutputChannel(0); // Default for now
                 player.getArpeggiator().setChannel(player.getArpOutputChannel());
            } else {
                std::cerr << "GUI: Failed to open MIDI ports." << std::endl;
            }
        }
    } else {
        ImGui::Text("MIDI Ports Connected: Input: %s, Output: %s",
            player.getMidiInput().getPortName(g_selected_midi_in_port).c_str(), // Assuming getPortName works after open
            player.getMidiOutput().getPortName(g_selected_midi_out_port).c_str()
        );
        if (ImGui::Button("Disconnect MIDI Ports")) {
            player.shutdown(); // Shutdown closes ports and stops arp
            g_selected_combi_index = -1; // Reset combi selection
            // Player state (initialized_) is handled by player.shutdown()
        }
    }

    ImGui::Spacing();

    // Section: PCG File Loading
    ImGui::SeparatorText("PCG File");
    if (player.isInitialized()) { // Only allow loading if MIDI is set up, or adjust logic
        ImGui::InputText("PCG File Path", g_pcg_file_path_buffer, sizeof(g_pcg_file_path_buffer));
        if (ImGui::Button("Load PCG File")) {
            if (strlen(g_pcg_file_path_buffer) > 0) {
                if (player.loadPcgFile(g_pcg_file_path_buffer)) {
                    g_selected_combi_index = -1; // Reset selection on new file
                }
            }
        }
    } else {
        ImGui::Text("Please connect MIDI ports before loading a PCG file.");
    }

    ImGui::Spacing();

    // Section: Combi Selection
    ImGui::SeparatorText("Combis");
    if (player.getCombiCount() > 0) {
        const auto& combi_names = player.getCombiNamesForGui();
        std::vector<const char*> combi_names_c_strs;
        for(const auto& name : combi_names) combi_names_c_strs.push_back(name.c_str());

        if (ImGui::BeginListBox("##Combis", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing()))) {
            for (int i = 0; i < (int)combi_names_c_strs.size(); ++i) {
                const bool is_selected = (g_selected_combi_index == i);
                if (ImGui::Selectable(combi_names_c_strs[i], is_selected)) {
                    g_selected_combi_index = i;
                    std::cout << "GUI: Selected Combi Index: " << i << std::endl;
                    player.selectCombi(i);
                    // Update arp enabled checkbox based on player state if necessary
                    g_arp_enabled_checkbox = player.isArpeggiatorEnabled();
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndListBox();
        }
    } else {
        ImGui::Text("No Combis loaded. Load a PCG file.");
    }

    ImGui::Spacing();

    // Section: Arpeggiator Controls
    ImGui::SeparatorText("Arpeggiator");
    if (player.isInitialized() && g_selected_combi_index != -1) { // Only show if ready and combi selected
        if (ImGui::Checkbox("Enable Arpeggiator", &g_arp_enabled_checkbox)) {
            player.setArpeggiatorEnabled(g_arp_enabled_checkbox);
        }

        const char* patterns[] = {"Up", "Down", "Up/Down", "Random"};
        if (ImGui::Combo("Pattern", &g_arp_pattern_combo_idx, patterns, IM_ARRAYSIZE(patterns))) {
            player.getArpeggiator().setPattern(static_cast<ArpPattern>(g_arp_pattern_combo_idx));
        }

        if (ImGui::SliderFloat("Rate (Hz)", &g_arp_rate_slider, 0.5f, 20.0f, "%.1f Hz")) {
            player.getArpeggiator().setRate(g_arp_rate_slider);
        }

        if (ImGui::SliderInt("Octaves", &g_arp_octaves_slider, 0, 4)) {
            player.getArpeggiator().setOctaves(g_arp_octaves_slider);
        }
        // Add controls for target/output channels if desired
        // uint8_t currentArpTarget = player.getArpTargetChannel();
        // if (ImGui::SliderInt("Arp Target Channel", (int*)&currentArpTarget, 0, 15)) { player.setArpTargetChannel(currentArpTarget); }
        // ... similar for output channel ...

    } else {
        ImGui::Text("Select MIDI ports and a Combi to enable Arpeggiator controls.");
    }


    ImGui::End(); // End main window
}


// Main Application
int main(int argc, char* argv[]) {
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return 1;
    }

    // GL 3.3 + GLSL 330
    const char* glsl_version = "#version 330 core";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    g_window = glfwCreateWindow(1280, 720, "Kronos Combi Player", NULL, NULL);
    if (g_window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(g_window);
    glfwSetFramebufferSizeCallback(g_window, framebuffer_size_callback);
    glfwSwapInterval(1); // Enable vsync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(g_window);
        glfwTerminate();
        return 1;
    }
    std::cout << "GLAD Initialized. OpenGL Version: " << (const char*)glGetString(GL_VERSION) << std::endl;


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Optional: if you want docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Optional: if you want multi-viewports

    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    ImGui_ImplGlfw_InitForOpenGL(g_window, true); // true = install callbacks
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Initial scan of MIDI ports for the GUI
    g_player.scanMidiPorts();
    g_arp_enabled_checkbox = g_player.isArpeggiatorEnabled(); // Sync GUI state
    g_arp_pattern_combo_idx = static_cast<int>(g_player.getArpeggiator().getPattern()); // Assuming getPattern exists
    g_arp_rate_slider = g_player.getArpeggiator().getRate();
    g_arp_octaves_slider = g_player.getArpeggiator().getOctaves(); // Assuming getOctaves exists

    // Main loop
    while (!glfwWindowShouldClose(g_window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        renderAppGui(g_player);

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(g_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.12f, 1.00f); // Darker background
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows (if viewports enabled)
        // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        //     GLFWwindow* backup_current_context = glfwGetCurrentContext();
        //     ImGui::UpdatePlatformWindows();
        //     ImGui::RenderPlatformWindowsDefault();
        //     glfwMakeContextCurrent(backup_current_context);
        // }

        glfwSwapBuffers(g_window);
    }

    // Cleanup
    std::cout << "Shutting down..." << std::endl;
    g_player.shutdown(); // Shuts down MIDI, arpeggiator thread

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(g_window);
    glfwTerminate();
    std::cout << "Application terminated." << std::endl;

    return 0;
}
