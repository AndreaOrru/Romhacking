#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <vector>
#include "types.hpp"


// Event script block.
class Block
{
    static int fetchBlockAddress(int i);

  public:
    std::vector<u8>  data;     // Decompressed data.
    std::vector<u16> indexes;  // The indexes this block is referenced by.
    int begin;    // The beginning address of the block.
    int end = 0;  // The ending address of the block.

    // Constructor.
    Block(int index, int address) : begin(address)
    {
        indexes.push_back(index);  // The first index.
    }

    // Define a precedence operator between blocks (needed for std::map).
    inline bool operator< (const Block& other) const { return begin < other.begin; }

    static std::vector<Block> extractAll();             // Extract all blocks.
    static void insertAll(std::vector<Block>& blocks);  // Reinsert all the blocks.
    void decompress();  // Decompress the block's data.

};

#endif
