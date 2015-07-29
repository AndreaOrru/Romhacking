#pragma once
#include <string>
#include <vector>
#include "sentence.hpp"

class Block
{
  public:
    Block(const uint8_t* rom, uint16_t index, uint32_t addr, bool isText) :
        rom(rom), index(index), addr(addr), isText(isText) {};

    uint16_t get_index() const { return index; }
    uint32_t get_size()  const { return (data == nullptr) ? 0 : data->size(); }
    std::vector<Sentence>& get_sentences() { return sentences; }

    static std::vector<Block>* extract_blocks(const uint8_t* rom);
    static void reinsert_blocks(uint8_t* rom, std::vector<Block>* blocks, const std::string& text);

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
