#pragma once
#include <vector>

namespace Huffman
{


std::vector<uint16_t>* compress(std::vector<std::vector<uint16_t>*>& blocksData);
std::vector<uint8_t>* decompress(const uint16_t* comprData, uint32_t comprSize,
                                 const uint16_t* meta);

}
