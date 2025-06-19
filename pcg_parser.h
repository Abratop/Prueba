#ifndef PCG_PARSER_H
#define PCG_PARSER_H

#include "pcg_structures.h"
#include <string>
#include <vector>
#include <fstream>
#include <iostream> // For std::cerr

class PcgParser {
public:
    PcgParser(const std::string& filename);
    bool parse();

    // Getter methods
    const PCGHeader& getPcgHeader() const;
    const DIV1Chunk& getDiv1Chunk() const; // Assuming DIV1 is unique and important enough to store
    const std::vector<CombiBankChunk>& getCombiBankChunks() const;
    const std::vector<CombiData>& getCombis() const;

private:
    std::string filename_;
    PCGHeader header_{}; // Initialize with default values
    DIV1Chunk div1_chunk_{}; // Initialize with default values
    std::vector<CombiBankChunk> combi_bank_chunks_;
    std::vector<CombiData> combis_;
    bool parsed_successfully_ = false;

    // Helper methods
    bool readChunkHeader(std::ifstream& file, ChunkHeader& header);
    bool parseDiv1Chunk(std::ifstream& file, uint32_t length);
    bool parseCbk1Chunk(std::ifstream& file, uint32_t length);
    // CMB1 is just a container, might not need a dedicated parse method if it has no data itself other than child chunks
};

#endif // PCG_PARSER_H
