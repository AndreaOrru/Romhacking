#ifndef SENTENCE_HPP
#define SENTENCE_HPP

#include <map>
#include <string>
#include <utility>
#include <vector>
#include "block.hpp"
#include "types.hpp"


class Sentence
{
    static const std::string names[];

    static int trySentence(std::vector<u8>& data, int begin);
    static bool check(std::vector<u8>::iterator begin, std::vector<u8>::iterator end);
    void stringify();
    std::vector<u8> unstringify();

  public:
    Block* block;      // Parent block.
    int begin;         // Beginning of the sentence inside the block.
    int end;           // End of the sentence inside the block.
    std::string text;  // Sentence's text.

    // Constructor.
    Sentence(Block* block, int begin, int end) : block(block), begin(begin), end(end) {};

    // Extract all sentences from the given blocks.
    static std::map<std::pair<int,int>, Sentence*> extractAll(std::vector<Block>& blocks);
    static void insertAll(std::map<std::pair<int,int>, Sentence*>& sentences);
};

#endif
