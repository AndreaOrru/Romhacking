#pragma once
#include <vector>

namespace Huffman
{

std::vector<uint8_t>* decompress(const uint16_t* comprData, uint32_t comprSize,
                                 const uint16_t* meta);

}
