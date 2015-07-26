#pragma once
#include <vector>

namespace Huffman
{

std::vector<uint8_t>* decompress(uint16_t* comprData, uint32_t comprSize, uint16_t* meta);

}
