#pragma once
#include <map>
#include <string>
#include <vector>

struct Sentence
{
  public:
    Sentence(uint16_t block, uint16_t index, uint32_t pos,
             std::vector<uint8_t>::const_iterator begin,
             std::vector<uint8_t>::const_iterator end);

    uint16_t get_block() const { return block; }
    uint16_t get_index() const { return index; }
    uint32_t get_pos()   const { return pos; }
    uint32_t get_size()  const { return (data == nullptr) ? 0 : data->size(); }
    std::vector<uint8_t>* get_data() const { return data; }
    void set_data(std::vector<uint8_t>* data);
    void set_pos(uint32_t pos) { this->pos = pos; }
    const std::string& get_text();
    static std::map<std::pair<uint16_t,uint16_t>,std::vector<uint8_t>*>* extract_sentences(const std::string& text);

  private:
    static const std::string names[];
    uint16_t block;
    uint16_t index;
    uint32_t pos;
    std::vector<uint8_t>* data;
    std::string text;

    static std::string stringify(uint8_t c);
    void format();
    static std::vector<uint8_t>* unformat(std::string& content);
};
