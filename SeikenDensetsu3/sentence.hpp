#pragma once
#include <string>
#include <vector>

struct Sentence
{
  public:
    Sentence(uint16_t block, uint16_t index, uint32_t pos,
             std::vector<uint8_t>::const_iterator begin,
             std::vector<uint8_t>::const_iterator end);

    Sentence(uint16_t block, uint16_t index, std::string content);

    uint16_t get_block() const { return block; }
    uint16_t get_index() const { return index; }
    uint32_t get_pos()   const { return pos; }
    uint32_t get_size()  const { return data.size(); }
    const std::string& get_text();
    static std::vector<Sentence>* extract_sentences(const std::string& text);

  private:
    static const std::string names[];
    uint16_t block;
    uint16_t index;
    uint32_t pos;
    std::vector<uint8_t> data;
    std::string text;

    static std::string stringify(uint8_t c);
    void format();
    void unformat(std::string& content);
};
