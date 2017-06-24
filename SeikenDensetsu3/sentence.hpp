#ifndef SENTENCE_HPP
#define SENTENCE_HPP

#include <string>
#include <vector>
#include "block.hpp"
#include "types.hpp"


class Sentence
{
    static const std::string names[];

    static int trySentence(std::vector<u8>& data, int begin);
    static bool check(std::vector<u8>::iterator begin, std::vector<u8>::iterator end);
    void stringify();

  public:
    Block* block;      // Parent block.
    int begin;         // Beginning of the sentence inside the block.
    int end;           // End of the sentence inside the block.
    std::string text;  // Sentence's text.

    // Constructor.
    Sentence(Block* block, int begin, int end) : block(block), begin(begin), end(end) {};

    // Extract all sentences from the given blocks.
    static std::vector<Sentence> extractAll(std::vector<Block>& blocks);
};

#endif
