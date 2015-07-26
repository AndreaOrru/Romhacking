#include "bytepair.hpp"

using namespace std;

namespace BytePair
{

vector<uint8_t> decompress(uint16_t v, uint16_t* meta)
{
    if (v == 0x100)
        return { 0xFF, 0xFF };
    else if (v < 0x100)
        return { (uint8_t) v };

    v -= 0x101;

    auto s  = decompress(meta[2 + v*2]    , meta);
    auto s2 = decompress(meta[2 + v*2 + 1], meta);
    s.insert(s.end(), s2.begin(), s2.end());

    return s;
}

}
