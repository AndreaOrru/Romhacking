#include <algorithm>
#include "huffman.hpp"

using namespace std;

uint16_t xba(uint16_t v)
{
    uint8_t l =  v & 0x00FF;
    uint8_t h = (v & 0xFF00) >> 8;

    return (l << 8) | h;
}

vector<uint8_t> sub_mte(uint16_t* meta, uint16_t v)
{
    if (v == 0x100)
        return { 0xFF, 0xFF };
    else if (v < 0x100)
        return { (uint8_t) v };

    v -= 0x101;

    auto s  = sub_mte(meta, meta[2 + v*2]);
    auto s2 = sub_mte(meta, meta[2 + v*2 + 1]);
    s.insert(s.end(), s2.begin(), s2.end());

    return s;
}

vector<uint8_t>* decompress(uint16_t* data, uint32_t size, uint16_t* meta)
{
    auto out = new vector<uint8_t>;

    uint16_t bits;
    uint32_t dataPtr = 0;
    uint8_t  i = 1;

    while (dataPtr <= size)
    {
        uint16_t metaPtr = meta[0];

        while (not (metaPtr & 0x8000))
        {
            if (--i == 0)
            {
                bits = xba(data[dataPtr / 2]);
                dataPtr += 2;
                i = 16;
            }

            bool b = bits & 0x8000;
            bits <<= 1;

            if (b) metaPtr += 2;

            metaPtr = meta[metaPtr / 2];
        }

        auto s = sub_mte(meta, metaPtr & 0x7FFF);
        out->insert(out->end(), s.begin(), s.end());
    }

    vector<uint8_t> end { 0xFF, 0xFF };

    auto it = find_end(out->begin(), out->end(), end.begin(), end.end());
    if (it != out->end())
        out->resize(distance(out->begin(), it) + end.size());

    return out;
}
