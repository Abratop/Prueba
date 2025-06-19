#include "pcg_parser.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <cstring> // For strncmp

PcgParser::PcgParser(const std::string& filename) : filename_(filename) {}

bool PcgParser::readChunkHeader(std::ifstream& file, ChunkHeader& header) {
    file.read(reinterpret_cast<char*>(&header.chunk_name), sizeof(header.chunk_name));
    if (file.gcount() != sizeof(header.chunk_name)) {
        return false; // Could be EOF or read error
    }
    file.read(reinterpret_cast<char*>(&header.chunk_length), sizeof(header.chunk_length));
    if (file.gcount() != sizeof(header.chunk_length)) {
        std::cerr << "Error: Could not read chunk length." << std::endl;
        return false;
    }
    return true;
}

bool PcgParser::parseDiv1Chunk(std::ifstream& file, uint32_t length) {
    if (length < sizeof(DIV1Chunk)) {
        std::cerr << "Error: DIV1 chunk length is too small. Expected " << sizeof(DIV1Chunk)
                  << ", got " << length << std::endl;
        file.seekg(length, std::ios_base::cur); // Skip the erroneous chunk
        return false;
    }
    file.read(reinterpret_cast<char*>(&div1_chunk_), sizeof(DIV1Chunk));
    if (file.gcount() != sizeof(DIV1Chunk)) {
        std::cerr << "Error: Failed to read DIV1 chunk data." << std::endl;
        return false;
    }
    // If DIV1Chunk was larger than expected, skip the remainder
    if (length > sizeof(DIV1Chunk)) {
        file.seekg(length - sizeof(DIV1Chunk), std::ios_base::cur);
    }
    return true;
}

bool PcgParser::parseCbk1Chunk(std::ifstream& file, uint32_t length) {
    CombiBankChunk bank_chunk_header;
    size_t expected_cbk1_header_size = sizeof(bank_chunk_header.ckb1_size_value_1) +
                                       sizeof(bank_chunk_header.ckb1_unknown_1) +
                                       sizeof(bank_chunk_header.number_of_combis) +
                                       sizeof(bank_chunk_header.ckb1_reserved_1) +
                                       sizeof(bank_chunk_header.size_of_combi_entry) +
                                       sizeof(bank_chunk_header.ckb1_reserved_2) +
                                       sizeof(bank_chunk_header.bank_id);

    if (length < expected_cbk1_header_size) {
        std::cerr << "Error: CBK1 chunk length is too small for its header. Expected at least "
                  << expected_cbk1_header_size << ", got " << length << std::endl;
        file.seekg(length, std::ios_base::cur);
        return false;
    }

    file.read(reinterpret_cast<char*>(&bank_chunk_header.ckb1_size_value_1), sizeof(bank_chunk_header.ckb1_size_value_1));
    file.read(reinterpret_cast<char*>(&bank_chunk_header.ckb1_unknown_1), sizeof(bank_chunk_header.ckb1_unknown_1));
    file.read(reinterpret_cast<char*>(&bank_chunk_header.number_of_combis), sizeof(bank_chunk_header.number_of_combis));
    file.read(reinterpret_cast<char*>(&bank_chunk_header.ckb1_reserved_1), sizeof(bank_chunk_header.ckb1_reserved_1));
    file.read(reinterpret_cast<char*>(&bank_chunk_header.size_of_combi_entry), sizeof(bank_chunk_header.size_of_combi_entry));
    file.read(reinterpret_cast<char*>(&bank_chunk_header.ckb1_reserved_2), sizeof(bank_chunk_header.ckb1_reserved_2));
    file.read(reinterpret_cast<char*>(&bank_chunk_header.bank_id), sizeof(bank_chunk_header.bank_id));

    if (file.fail()) {
        std::cerr << "Error: Failed to read CBK1 chunk header." << std::endl;
        // Attempt to skip the rest of the chunk based on the initial length
        // This might be inaccurate if the read failed mid-header
        return false;
    }

    combi_bank_chunks_.push_back(bank_chunk_header);

    if (bank_chunk_header.size_of_combi_entry != sizeof(CombiData)) {
        std::cerr << "Warning: Size of combi entry in CBK1 header (" << bank_chunk_header.size_of_combi_entry
                  << ") does not match expected CombiData size (" << sizeof(CombiData)
                  << "). Combi data might be misinterpreted." << std::endl;
        // We will still try to read based on size_of_combi_entry from header, but it might be risky.
        // For robustness, one might prefer to skip if sizes don't match an expected value.
    }

    uint32_t actual_combi_data_size_in_chunk = length - static_cast<uint32_t>(expected_cbk1_header_size);
    uint32_t expected_combi_data_size_in_chunk = static_cast<uint32_t>(bank_chunk_header.number_of_combis) * bank_chunk_header.size_of_combi_entry;

    if (actual_combi_data_size_in_chunk < expected_combi_data_size_in_chunk) {
         std::cerr << "Error: CBK1 chunk data size is too small. Expected " << expected_combi_data_size_in_chunk
                  << " for " << (int)bank_chunk_header.number_of_combis << " combis, got " << actual_combi_data_size_in_chunk << std::endl;
        // We cannot reliably read combis, so we return false. The file pointer is already past the header.
        // We should skip what the chunk header *claimed* was the rest of its data.
        file.seekg(actual_combi_data_size_in_chunk, std::ios_base::cur);
        return false;
    }


    for (int i = 0; i < bank_chunk_header.number_of_combis; ++i) {
        CombiData combi;
        // Read based on the size specified in the bank header
        file.read(reinterpret_cast<char*>(&combi), bank_chunk_header.size_of_combi_entry);
        if (file.gcount() != bank_chunk_header.size_of_combi_entry) {
            std::cerr << "Error: Failed to read CombiData entry " << i << " in CBK1 chunk." << std::endl;
            // Skip the rest of what this chunk claims to be its data.
            // Current position is after the failed read. Need to calculate remaining bytes from original length.
            // file.tellg() gives current pos.
            // Add logic here to seek past the end of this chunk if necessary.
            return false;
        }
        combis_.push_back(combi);
    }

    // If the declared chunk length was greater than header + (num_combis * size_combi_entry), skip padding.
    uint32_t total_read_for_bank = static_cast<uint32_t>(expected_cbk1_header_size) + (static_cast<uint32_t>(bank_chunk_header.number_of_combis) * bank_chunk_header.size_of_combi_entry);
    if (length > total_read_for_bank) {
        file.seekg(length - total_read_for_bank, std::ios_base::cur);
    }

    return true;
}

bool PcgParser::parse() {
    std::ifstream file(filename_, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename_ << std::endl;
        return false;
    }

    // Read PCGHeader
    file.read(reinterpret_cast<char*>(&header_), sizeof(PCGHeader));
    if (file.gcount() != sizeof(PCGHeader)) {
        std::cerr << "Error: Failed to read PCGHeader." << std::endl;
        return false;
    }

    // Validate PCGHeader magic
    if (strncmp(header_.magic, "KORG", 4) != 0) {
        std::cerr << "Error: Invalid PCG file magic. Expected 'KORG'." << std::endl;
        // continue parsing as KORG is not part of PCG1 but the overall file.
        // The first chunk *must* be PCG1 though.
    }
    // The PCGHeader structure already includes the first ChunkHeader for "PCG1"
    // So, we check that one specifically.
    if (strncmp(header_.pcg1_chunk_header.chunk_name, "PCG1", 4) != 0) {
        std::cerr << "Error: Expected 'PCG1' chunk after KORG header, but found '"
                  << std::string(header_.pcg1_chunk_header.chunk_name, 4) << "'." << std::endl;
        return false;
    }

    // The PCG1 chunk data starts immediately after the PCGHeader structure.
    // The length in header_.pcg1_chunk_header.chunk_length is for data *within* PCG1,
    // not including sub-chunks like DIV1 etc.
    // We need to position the file pointer to read the next actual chunk header
    // The PCG1 data itself (version, reserved, flags) are part of PCGHeader struct.
    // The pcg1_chunk_header.chunk_length refers to content *after* PCGHeader up to the next main chunk.
    // This is a bit confusing. Let's assume PCG1's length means all content it "owns",
    // which includes DIV1, CMB1, etc.
    // For now, we'll just start reading subsequent chunks directly.
    // The file pointer is currently right after the full PCGHeader.

    ChunkHeader current_chunk_header;
    while (readChunkHeader(file, current_chunk_header)) {
        std::string chunk_name_str(current_chunk_header.chunk_name, 4);
        // std::cout << "Found chunk: " << chunk_name_str << " with length: " << current_chunk_header.chunk_length << std::endl;

        if (chunk_name_str == "DIV1") {
            if (!parseDiv1Chunk(file, current_chunk_header.chunk_length)) {
                std::cerr << "Error parsing DIV1 chunk." << std::endl;
                // Decide if this is fatal. For now, let's try to continue.
            }
        } else if (chunk_name_str == "CMB1") {
            // CMB1 is a container chunk. Its length includes all CBK1 chunks within it.
            // We don't store CMB1 itself, but it signals that CBK1 chunks will follow.
            // The parsing loop will naturally pick up the CBK1s.
            // We could validate that the sum of CBK1 lengths matches CMB1 length, but that's complex.
            // For now, just log it.
            // std::cout << "Entering CMB1 container chunk." << std::endl;
            // The CMB1 chunk itself doesn't have data other than its children, so we don't seek.
            // The next readChunkHeader will read the first child (e.g. CBK1)
        } else if (chunk_name_str == "CBK1") {
            if (!parseCbk1Chunk(file, current_chunk_header.chunk_length)) {
                std::cerr << "Error parsing CBK1 chunk." << std::endl;
                // Potentially fatal for combi extraction.
            }
        } else if (chunk_name_str == "PRG1" || chunk_name_str == "SLS1" ||
                   chunk_name_str == "DKB1" || chunk_name_str == "WSB1" || /* other known but unhandled bank types */
                   chunk_name_str == "INDX" || chunk_name_str == "ARPG" ||
                   chunk_name_str == "GLB1" || chunk_name_str == "DPI1" ||
                   chunk_name_str == "SQS1" || chunk_name_str == "SQB1" ||
                   chunk_name_str == "MTR1" || chunk_name_str == "TPB1" ) {
            // std::cout << "Skipping known chunk: " << chunk_name_str << std::endl;
            file.seekg(current_chunk_header.chunk_length, std::ios_base::cur);
            if (file.fail()) {
                std::cerr << "Error seeking past chunk: " << chunk_name_str << std::endl;
                return false;
            }
        } else {
            std::cerr << "Warning: Found unknown chunk: '" << chunk_name_str
                      << "' with length " << current_chunk_header.chunk_length
                      << ". Skipping." << std::endl;
            file.seekg(current_chunk_header.chunk_length, std::ios_base::cur);
            if (file.fail()) {
                std::cerr << "Error seeking past unknown chunk: " << chunk_name_str << std::endl;
                return false;
            }
        }

        if (file.eof()) {
            // std::cout << "EOF reached after processing chunk " << chunk_name_str << std::endl;
            break;
        }
        if (file.fail()) {
            std::cerr << "File stream error after processing chunk " << chunk_name_str << std::endl;
            return false;
        }
    }

    if (file.eof() && !file.bad()) {
        // EOF is expected if we successfully read all chunks
        // std::cout << "Successfully reached EOF." << std::endl;
        parsed_successfully_ = true;
    } else if (file.bad()) {
        std::cerr << "Error: File stream is bad." << std::endl;
        return false;
    } else if (!file.eof()) {
        // This case means readChunkHeader returned false not due to EOF
        // std::cerr << "Stopped reading chunks, not at EOF. Possible incomplete file or read error." << std::endl;
        // It might be okay if it's just padding at the end of file.
        // For now, consider it successful if no other errors occurred.
        parsed_successfully_ = !combi_bank_chunks_.empty() || !combis_.empty(); // Heuristic for success
    }


    file.close();
    return parsed_successfully_;
}

// Getter implementations
const PCGHeader& PcgParser::getPcgHeader() const {
    return header_;
}

const DIV1Chunk& PcgParser::getDiv1Chunk() const {
    // Add check if DIV1 was actually parsed if it's optional
    return div1_chunk_;
}

const std::vector<CombiBankChunk>& PcgParser::getCombiBankChunks() const {
    return combi_bank_chunks_;
}

const std::vector<CombiData>& PcgParser::getCombis() const {
    return combis_;
}
