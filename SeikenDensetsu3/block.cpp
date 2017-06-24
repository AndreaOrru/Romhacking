#include <map>
#include "huffman.hpp"
#include "rom.hpp"
#include "block.hpp"

using namespace std;


// Extract all the blocks.
vector<Block> Block::extractAll()
{
    // block address => block object.
    map<int, Block> blocks;

    // Iterate through all the blocks.
    for (int i = 0; i < 0x1000; i++)
    {
        int address = fetchBlockAddress(i);
        // Check if we already encountered this block.
        auto it = blocks.find(address);
        if (it != blocks.end())
            // We did, keep track of this block index as well.
            it->second.indexes.push_back(i);
        else
            // We didn't, create a new block.
            blocks.emplace(address, Block(i, address));
    }

    // Build a vector of the blocks, ordered by address.
    vector<Block> blocks_vec;
    transform(blocks.begin(), blocks.end(), back_inserter(blocks_vec),
              [](pair<const int, Block> &x) { return x.second; });

    // Decompress each block's data.
    for (auto it = blocks_vec.begin(); it != blocks_vec.end(); it++)
    {
        // A block begins where the next one ends, or at the end of all the blocks.
        it->end = (next(it) == blocks_vec.end()) ? 0x3BD079 : next(it)->begin;
        // Block can't cross bank boundaries.
        if ((it->begin >> 16) != (it->end >> 16))
            it->end = (it->begin & 0xFF0000) + 0x10000;  // Keep it in the beginning bank.
        it->decompress();
    }

    return blocks_vec;
}

// Get the address of the block of index i.
int Block::fetchBlockAddress(int i)
{
    // Fetch bank index.
    u8 bank_index = ROM::readByte(0x3E4E00 + i/2);
    if (i % 2) bank_index >>= 4;
    bank_index &= 0x0F;

    // Fetch the full address of the block.
    u8  bank    = ROM::readByte(0xEA91 + bank_index) - 0xC0;
    u16 offset  = ROM::readWord(0x3E2E00 + i*2);
    int address = (bank << 16) | offset;

    return address;
}

// Decompress the data of the block.
void Block::decompress()
{
    this->data = Huffman::decompress(this->begin, this->end);
}
