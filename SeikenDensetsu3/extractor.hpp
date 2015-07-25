#pragma once
#include <string>
#include <vector>

struct Sentence
{
    uint32_t pos;
    std::string text;

    Sentence(uint32_t pos, std::string text) : pos(pos), text(text) {};
};

struct Script
{
    std::vector<uint8_t>* data;
    std::vector<Sentence>* sentences;

    Script(std::vector<uint8_t>* data, std::vector<Sentence>* sentences) :
        data(data), sentences(sentences) {};
};

Script extract(std::vector<uint8_t>* data);
