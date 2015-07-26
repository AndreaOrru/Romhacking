#pragma once
#include <vector>

namespace BytePair
{

std::vector<uint8_t> decompress(uint16_t v, const uint16_t* meta);

}
