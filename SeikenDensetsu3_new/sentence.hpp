#pragma once
#include <string>
#include <vector>
#include "block.hpp"
#include "types.hpp"


class Sentence
{
    static const std::string names[];

    static u32 try_sentence(std::vector<u8>& data, u32 begin);
    static bool check(std::vector<u8>& data, u32 begin, u32 end);
    void stringify();
    std::vector<u8> unstringify();

  public:
    Block* block;
    u32 begin;
    u32 end;
    std::string text;

    Sentence(Block* block, u32 begin, u32 end) : block(block), begin(begin), end(end) {};
    static std::vector<Sentence> extract_sentences(std::vector<Block>& blocks);
    static void insert_sentences(std::vector<Sentence>& sentences);
};
