#pragma once
#include <vector>

std::vector<uint8_t>* decompress(uint8_t* data);
std::vector<uint8_t>* decompress(std::vector<uint8_t>* data);
std::vector<uint8_t>*   compress(std::vector<uint8_t>* data);
