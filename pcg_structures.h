#ifndef PCG_STRUCTURES_H
#define PCG_STRUCTURES_H

#include <cstdint>

// Standard chunk header found in Korg PCG files
struct ChunkHeader {
    char chunk_name[4];      // Four-character chunk identifier (e.g., "PCG1", "DIV1", "CBK1")
    uint32_t chunk_length;   // Length of the chunk data following this header
};

// Main header for the PCG file
struct PCGHeader {
    char magic[4];              // File magic identifier (e.g., "KORG")
    uint8_t product_id;         // Identifier for the Korg product
    uint8_t file_format;        // File format type (0x00 for PCG)
    uint8_t main_version;       // Main version number of the file format
    uint8_t minor_version;      // Minor version number of the file format
    uint8_t checksum_flag;      // Flag indicating checksum presence (00 for no checksum)
    char reserved_intro[7];     // Reserved bytes, often part of a longer "KORG INC." string
    ChunkHeader pcg1_chunk_header; // Header for the "PCG1" chunk
    uint16_t pcg1_version;      // Version of the PCG1 data structure
    uint8_t pcg1_reserved;      // Reserved byte in PCG1 header
    uint8_t pcg1_flags;         // Flags related to PCG1 data
};

// Structure for the "DIV1" chunk, containing various bank flags
struct DIV1Chunk {
    uint8_t div1_data_unknown_1[8]; // Unknown data, observed as 00 01 00 00 00 00 00 00
    uint8_t program_banks_flags_1[4]; // Flags for program banks (e.g., FF FF 00 15)
    uint8_t program_banks_flags_2[4]; // Additional flags for program banks (e.g., 00 0F 00 15)
    uint8_t combi_banks_flags_1[4];   // Flags for combi banks (e.g., 3F FF 00 0E)
    uint8_t combi_banks_flags_2[4];   // Additional flags for combi banks (e.g., 00 00 00 0E)
    uint8_t drumkit_banks_flags_1[4]; // Flags for drumkit banks
    uint8_t drumkit_banks_flags_2[4]; // Additional flags for drumkit banks
    uint8_t waveseq_banks_flags_1[4]; // Flags for wavesequence banks
    uint8_t waveseq_banks_flags_2[4]; // Additional flags for wavesequence banks
    uint8_t misc_flags[4];            // Miscellaneous flags (DPI, Set list, Global)
    uint8_t reserved_div1[4];         // Reserved bytes in DIV1 chunk
};

// Structure for individual timbre data within a Combi
struct TimbreData {
    uint8_t program_bank_msb; // MSB of the program bank (e.g., 07 from 07 00)
    uint8_t program_lsb;      // LSB of the program number (e.g., 00 from 07 00)
    uint8_t status;           // Timbre status (e.g., 00 for Off, 21 for Int)
    uint8_t reserved_timbre[1]; // Reserved/padding byte for alignment or future use
};

// Structure for a single Combi entry
struct CombiData {
    char name[24];                                  // Name of the Combi (null-terminated if shorter)
    unsigned char other_combi_params[7722];         // Placeholder for other parameters within a combi structure.
                                                    // Calculated as: total_combi_size (7810) - name_size (24) - timbres_size (16 * 4)
    TimbreData timbres[16];                         // Array of 16 timbres
};

// Structure for the "CBK1" chunk, which is a Combi bank
struct CombiBankChunk {
    uint32_t ckb1_size_value_1;     // Size-related value (e.g., 0F 41 0C)
    uint32_t ckb1_unknown_1;        // Unknown data (e.g., 03 00 6D)
    uint8_t number_of_combis;       // Number of combis in this bank (e.g., 80 hex = 128 decimal)
    uint8_t ckb1_reserved_1[3];     // Reserved/padding bytes
    uint16_t size_of_combi_entry;   // Size of a single CombiData entry (e.g., 1E 82 hex = 7810 decimal)
    uint16_t ckb1_reserved_2;       // Reserved/padding bytes
    uint32_t bank_id;               // Bank ID (e.g., 0 for Bank A)
    // CombiData entries follow this structure in the file
};

#endif // PCG_STRUCTURES_H
