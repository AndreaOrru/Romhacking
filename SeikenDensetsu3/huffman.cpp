#include "bytepair.hpp"
#include "huffman.hpp"

using namespace std;

namespace Huffman
{

uint16_t xba(uint16_t v)
{
    uint8_t l =  v & 0x00FF;
    uint8_t h = (v & 0xFF00) >> 8;

    return (l << 8) | h;
}

vector<uint8_t>* decompress(const uint16_t* comprData, uint32_t comprSize, const uint16_t* meta)
{
    auto* data = new vector<uint8_t>;

    uint32_t comprPtr = 0;
    uint16_t bits;
    uint8_t i = 1;

    while (comprPtr <= comprSize)
    {
        uint16_t metaPtr = meta[0];

        while (not (metaPtr & 0x8000))
        {
            if (--i == 0)
            {
                bits = xba(comprData[comprPtr / 2]);
                comprPtr += 2;
                i = 16;
            }

            bool b = bits & 0x8000;
            bits <<= 1;

            if (b)
                metaPtr += 2;

            metaPtr = meta[metaPtr / 2];
        }

        auto s = BytePair::decompress(metaPtr & 0x7FFF, meta);
        data->insert(data->end(), s.begin(), s.end());
    }

    vector<uint8_t> end = { 0xFF, 0xFF };
    auto occ = find_end(data->begin(), data->end(), end.begin(), end.end());
    if (occ != data->end())
        data->resize(distance(data->begin(), occ) + end.size());

    return data;
}

}
