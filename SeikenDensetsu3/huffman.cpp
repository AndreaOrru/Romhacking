#include "bytepair.hpp"
#include "rom.hpp"
#include "huffman.hpp"

using namespace std;


namespace Huffman
{

// Decompress a block.
vector<u8> decompress(int begin, int end)
{
    vector<u8> data;

    int i = begin;
    u8 bits = 0;  // 8 bits of Huffman stream.
    int b = 0;    // Number of bits still available.

    while (i < end)
    {
        // Start from the root of the tree.
        u16 node = ROM::readWord(0x3E6600);

        // Loop until we reach a leaf.
        while (!(node & 0x8000))
        {
            // If we don't have any more bits, load a new byte.
            if (b == 0)
            {
                bits = ROM::readByte(i++);
                b = 8;
            }

            // When MSB == 0, left child, otherwise right.
            if (bits & 0x80)
                node += 2;

            // Load the child node.
            node = ROM::readWord(0x3E6600 + node);
            // Shift to the next bit.
            bits <<= 1;
            b--;
        }

        // Exclude the most significant bit.
        u16 nodeValue = node & 0x7FFF;
        // Expand the bytepair and append the result to the buffer.
        auto s = BytePair::expand(nodeValue);
        data.insert(data.end(), s.begin(), s.end());
    }

    return data;
}

}
