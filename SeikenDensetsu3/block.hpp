#pragma once
#include <utility>
#include <vector>
#include "types.hpp"


struct Block
{
    Block(int begin, int end) : begin(begin), end(end) {}
    Block(int begin, int bank_ptr, int offset_ptr) : begin(begin) { pointers.emplace_back(bank_ptr, offset_ptr); }
    inline bool operator< (const Block& other) const { return begin < other.begin; }

  public:
    std::vector<std::pair<int,int>> pointers;
    int begin;
    int end = 0;
    std::vector<u8> data;

    static void insert_blocks(std::vector<Block>& blocks, const char* outname);
    static std::vector<Block> extract_blocks();
};
