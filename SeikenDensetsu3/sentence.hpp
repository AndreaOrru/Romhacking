#pragma once
#include <string>
#include <vector>
#include "block.hpp"
#include "types.hpp"


class Sentence
{
    static const std::string names[];

    static int try_sentence(std::vector<u8>& data, int begin);
    static bool check(std::vector<u8>::iterator begin, std::vector<u8>::iterator end);
    void stringify();
    std::vector<u8> unstringify();

  public:
    Block* block;
    int begin;
    int end;
    std::string text;

    Sentence(Block* block, int begin, int end) : block(block), begin(begin), end(end) {};
    static std::vector<Sentence> extract_sentences(std::vector<Block>& blocks);
    static void insert_sentences(std::vector<Sentence>& sentences);
};
