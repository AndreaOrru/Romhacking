#pragma once
#include <vector>
#include "types.hpp"


struct Block
{
    Block(int index, u32 begin, u32 end = 0) : index(index), begin(begin), end(end) {};
    inline bool operator< (const Block& other) const { return begin < other.begin; }

  public:
    int index;
    u32 begin;
    u32 end;
    std::vector<u8> data;

    static void insert_blocks(std::vector<Block>& blocks, const char* outname);
    static std::vector<Block> extract_blocks();
};
