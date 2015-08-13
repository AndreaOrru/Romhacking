#pragma once
#include <vector>
#include "block.hpp"


namespace Huffman
{

const std::vector<u16>& get_encoded_tree();
std::vector<u8> decompress(const Block& block);
std::vector<std::vector<u8>> compress(const std::vector<Block>& blocks);

}
