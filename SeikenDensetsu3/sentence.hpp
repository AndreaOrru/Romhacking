#pragma once
#include <string>
#include <vector>

struct Sentence
{
  public:
    Sentence(uint32_t pos, std::vector<uint8_t>::const_iterator begin,
                           std::vector<uint8_t>::const_iterator end);
    uint32_t getPos() const { return pos; }
    uint32_t getSize() const { return data.size(); }
    const std::string& getText() const { return text; }

  private:
    static const std::string names[];
    uint32_t pos;
    std::vector<uint8_t> data;
    std::string text;

    void format();
    std::string stringify(uint8_t c);
};
