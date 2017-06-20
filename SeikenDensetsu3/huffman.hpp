#ifndef HUFFMAN_HPP
#define HUFFMAN_HPP

#include <vector>
#include "types.hpp"


namespace Huffman
{

// Decompress a block.
std::vector<u8> decompress(int begin, int end);

}

#endif
