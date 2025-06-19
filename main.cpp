#include "kronos_combi_player.h"
#include <iostream>
#include <string>
#include <limits> // For std::numeric_limits

// Basic signal handler for Ctrl+C (SIGINT)
#include <csignal>

// Global pointer to the player instance for the signal handler
KronosCombiPlayer* g_player_instance = nullptr;
volatile sig_atomic_t g_signal_flag = 0; // Flag to indicate signal occurrence

void signalHandler(int signum) {
    // Set a flag that the main loop can check.
    // Avoid complex operations inside the signal handler.
    g_signal_flag = signum;
}


int main(int argc, char* argv[]) {
    std::cout << "Welcome to the Kronos Combi Player!" << std::endl;

    KronosCombiPlayer player;
    g_player_instance = &player;

    // Register signal handler for SIGINT (Ctrl+C)
    signal(SIGINT, signalHandler);
    std::cout << "Registered signal handler for Ctrl+C." << std::endl;

    player.initialize();

    if (!player.isInitialized()) {
        std::cerr << "FATAL: Player initialization failed. Please check MIDI port availability and permissions. Exiting." << std::endl;
        return 1;
    }
    std::cout << "Player initialized successfully." << std::endl;

    std::string pcgFilePath;
    if (argc > 1) {
        pcgFilePath = argv[1];
        std::cout << "\nUsing PCG file provided as command line argument: " << pcgFilePath << std::endl;
    } else {
        std::cout << "\nEnter the path to the Korg Kronos PCG file: ";
        std::getline(std::cin, pcgFilePath);
        if (pcgFilePath.empty()) {
            std::cerr << "No PCG file path entered. Exiting." << std::endl;
            player.shutdown();
            return 1;
        }
    }

    if (!player.loadPcgFile(pcgFilePath)) {
        std::cerr << "FATAL: Failed to load PCG file '" << pcgFilePath << "'. Exiting." << std::endl;
        player.shutdown();
        return 1;
    }
    if (player.getCombiCount() == 0) {
        std::cout << "Warning: PCG file loaded, but no combis were found. The player might not be very useful." << std::endl;
        // player.shutdown(); // Optional: exit if no combis make the app useless.
        // return 1;
    }


    player.listCombis();

    if (player.getCombiCount() == 0) {
        std::cout << "No combis to select. Exiting." << std::endl;
        player.shutdown();
        return 0; // Not necessarily an error if file was valid but empty of combis
    }

    std::cout << "\nEnter the index of the combi you want to select: ";
    int combiIndex;
    while (!(std::cin >> combiIndex) || combiIndex < 0 || static_cast<size_t>(combiIndex) >= player.getCombiCount()) {
        std::cerr << "Invalid input. Please enter a number between 0 and " << player.getCombiCount() -1 << ": ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');


    if (!player.selectCombi(combiIndex)) {
        std::cerr << "FATAL: Failed to select combi " << combiIndex << ". Exiting." << std::endl;
        player.shutdown();
        return 1;
    }

    std::cout << "\nPlayer is now running. MIDI input is active for the selected combi." << std::endl;
    std::cout << "Arpeggiator is configured. Notes on the target channel will be arpeggiated." << std::endl;
    std::cout << "(Press Ctrl+C to exit)" << std::endl;

    while (g_signal_flag == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "\nSignal " << g_signal_flag << " received. Initiating shutdown..." << std::endl;
    player.shutdown();

    g_player_instance = nullptr; // Clear global pointer
    std::cout << "\nKronos Combi Player has shut down gracefully. Exiting." << std::endl;
    return 0;
}
