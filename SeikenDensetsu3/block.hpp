#pragma once
#include <string>
#include <vector>
#include "sentence.hpp"

struct Block
{
    uint8_t* rom;

    uint16_t index;
    uint32_t addr;
    bool isText;

    uint16_t* comprData;
    uint32_t  comprSize;

    std::vector<uint8_t>* data = nullptr;
    std::vector<Sentence> sentences;

    Block(uint8_t* rom, uint16_t index, uint32_t addr, bool isText) :
        rom(rom), index(index), addr(addr), isText(isText) {};

    void init(uint32_t nextAddr);
    void decompress();
    void extract();

    bool check(std::vector<uint8_t>::iterator begin, std::vector<uint8_t>::iterator end);
};

std::vector<Block>* getBlocks(uint8_t* rom);
