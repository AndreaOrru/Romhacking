#include <map>
#include "block.hpp"
#include "bytepair.hpp"
#include "huffman.hpp"

using namespace std;

vector<Block>* Block::extract_blocks(const uint8_t* rom)
{
    map<uint32_t,uint16_t> blocksMap;
    for (int i = 0; i < 0x1000; i++)
    {
        uint8_t    bank = rom[0xEA91 + (rom[0x3E4E00 + i/2] & 0xF)];
        uint16_t offset = ((uint16_t*) &rom[0x3E2E00])[i];
        uint32_t   addr = ((bank << 16) + offset) - 0xC00000;

        blocksMap[addr] = i;
    }
    for (int i = 0; i < 0x1A9; i++)
    {
        uint8_t    bank = 0xFB;
        uint16_t offset = ((uint16_t*) &rom[0x4BF7C])[i];
        uint32_t   addr = ((bank << 16) + offset) - 0xC00000;

        blocksMap[addr] = i + 0x1000;
    }
    blocksMap.erase(0x390055);
    blocksMap.erase(0x394C2D);
//    blocksMap.erase(0x3B0003);

    auto* blocks = new vector<Block>;
    for (auto& block: blocksMap)
    {
        uint16_t    i = block.second;
        uint32_t addr = block.first;
        bool   isText = addr < 0x3BD079 and !(addr >= 0x3B0000 and addr < 0x3B4C2E);
        if (block.second >= 0x1000)
            isText = false;

        blocks->emplace_back(rom, i, addr, isText);
    }

    for (int i = 0; i < blocks->size() - 1; i++)
        blocks->at(i).init(blocks->at(i+1).addr);
    blocks->back().init(0x3C0000);

    return blocks;
}

void Block::reinsert_blocks(uint8_t* rom, std::vector<Block>* blocks, const std::string& text)
{
    auto* newSentences = Sentence::extract_sentences(text);

    vector<vector<uint16_t>*> blocksData;
    for (auto& b: *blocks)
    {
        auto* blockData = new vector<uint16_t>;
        auto last = b.data->begin();

        for (auto& s: b.get_sentences())
        {
            auto* newData = newSentences->at(make_pair(b.index, s.get_index()));

            auto curr = b.data->begin() + s.get_pos();
            blockData->insert(blockData->end(), last, curr);
            last = curr + s.get_size();

            blockData->insert(blockData->end(), newData->begin(), newData->end());
        }
        blockData->insert(blockData->end(), last, b.data->end());
        blocksData.push_back(blockData);
    }

    auto* dict = BytePair::compress(blocksData);
    auto* tree = Huffman::compress(blocksData);

    uint32_t last = 0x390000;
    for (int i = 0; i < blocks->size(); i++)
    {
        auto& b = blocks->at(i);
        b.addr = last;
        b.comprData = (uint16_t*)(rom + b.addr);
        b.comprSize = blocksData.at(i)->size() * 2;

        if (b.index < 0x1000)
        {
            uint8_t*    bank = &rom[0x3E4E00 + b.index/2];
            uint16_t* offset = &((uint16_t*) &rom[0x3E2E00])[b.index];
            *bank   = (*bank & 0xF0) | (((b.addr >> 16) - 0x38) & 0xF);
            *offset = (b.addr & 0xFFFF);
        }
        else
        {
            uint16_t* offset = &((uint16_t*) &rom[0x4BF7C])[b.index - 0x1000];
            *offset = (b.addr & 0xFFFF);
        }

        copy(blocksData.at(i)->begin(), blocksData.at(i)->end(), b.comprData);
        last += b.comprSize;
    }

    copy(dict->begin(), dict->end(), (uint32_t*)(&rom[0x3E6604]));
    copy(tree->begin(), tree->end(), (uint16_t*)(&rom[0x3E7800]));
}

void Block::init(uint32_t nextAddr)
{
    this->comprData = (uint16_t*)(rom + addr);
    this->comprSize = nextAddr  - this->addr;

    this->decompress();
    this->extract();
}

void Block::decompress()
{
    data = Huffman::decompress(comprData, comprSize, (uint16_t*)(rom + 0x3E6600));
}

bool Block::check(vector<uint8_t>::const_iterator begin, vector<uint8_t>::const_iterator end)
{
    if (*begin == 0xF2)
        return *(begin+1) == 0x0E and find(begin, end, 0x14) != end;

    if (*begin == 0xF3)
        return *(begin+1) == 0x00 and find(begin, end, 0x14) != end;

    if (*begin == 0xF8)
        return (*(begin+1) == 0x01 and *(begin+2) >= 0xF0) or
               (*(begin+1) == 0x03 and *(begin+2) == 0x0A);

    if (*begin == 0xFA)
        return (*(begin+1) == 0xFB and *(begin+2) == 0xFC) or
               (*(begin+1) == 0x03 and *(begin+2) == 0x0A);

    if (distance(begin, end) < 7)
        return false;

    if (*begin != 0x7B and not ((*(begin+1) >= 0x10 and *(begin+1) <= 0x1F) or
                                (*(begin+1) >= 0xC0 and *(begin+1) <= 0xCF)))
        return false;

    if ((*begin == 0x58 or *begin == 0x5E) and (*(begin+1) == 0x18 and *(begin+2) == 0xF4 and
                                                *(begin+3) == 0x00))
        return false;

    return true;
}

void Block::extract()
{
    if (!isText)
        return;

    auto it = data->begin();
    while (it < data->end())
    {
        if (*it == 0x58 or *it == 0x5E or *it == 0x7B or *it == 0xF2 or *it == 0xF3 or *it == 0xF8 or *it == 0xFA)
        {
            auto begin = it;

            int n = (*it == 0x7B) ? 6 : 1;
            while (n > 0)
            {
                while (it+1 < data->end() and !(*it == 0xFF and *(it+1) == 0xFF))
                    it++;
                if (it+1 >= data->end())
                    break;
                it += 2;
                n--;
            }

            if (n > 0 or !check(begin, it))
                it = begin + 1;
            else
                sentences.emplace_back(index, sentences.size(),
                                       distance(data->begin(), begin),
                                       begin, it);
        }
        else
            it++;
    }
}
