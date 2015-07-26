#pragma once
#include <string>
#include <vector>

struct Sentence
{
    std::string names[6] = { "DURAN", "KEVIN", "HAWK", "ANGELA", "CARLIE", "LISE" };

    uint32_t pos;
    std::vector<uint8_t> data;
    std::string text;

    Sentence(uint32_t pos, std::vector<uint8_t>::iterator begin, std::vector<uint8_t>::iterator end);
    void format();
    std::string stringify(uint8_t c);
};
