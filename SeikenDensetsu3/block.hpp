#pragma once
#include <string>
#include <vector>
#include "sentence.hpp"

class Block
{
  public:
    Block(const uint8_t* rom, uint16_t index, uint32_t addr, bool isText) :
        rom(rom), index(index), addr(addr), isText(isText) {};

    static std::vector<Block>* extractBlocks(const uint8_t* rom);
    uint16_t getIndex() const { return index; }
    uint32_t getSize() const { return (data == nullptr) ? 0 : data->size(); }
    std::vector<Sentence>& getSentences() { return sentences; }

  private:
    const uint8_t* rom;

    uint16_t index;
    uint32_t addr;
    bool isText;

    uint16_t* comprData;
    uint32_t  comprSize;

    std::vector<uint8_t>* data = nullptr;
    std::vector<Sentence> sentences;

    void init(uint32_t nextAddr);
    void decompress();
    void extract();
    bool check(std::vector<uint8_t>::const_iterator begin,
               std::vector<uint8_t>::const_iterator end);
};
