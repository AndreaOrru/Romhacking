#pragma once
#include <vector>

namespace BytePair
{

std::vector<uint32_t>* compress(std::vector<std::vector<uint8_t>*>& blocksData);
std::vector<uint8_t> decompress(uint16_t v, const uint16_t* meta);

}
