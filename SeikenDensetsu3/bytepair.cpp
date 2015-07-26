#include <list>
#include <map>
#include "bytepair.hpp"

using namespace std;

namespace BytePair
{

void replace_pair(list<uint16_t>& block, uint32_t p, uint16_t sym)
{
    uint16_t p1 =  p        & 0xFFFF;
    uint16_t p2 = (p >> 16) & 0xFFFF;

    auto it = block.begin();
    while (it != prev(block.end()))
    {
        if (*it == p1 and *next(it) == p2)
        {
            block.erase(next(it));
            *it = sym;
        }
        it++;
    }
}

vector<uint32_t>* compress(vector< list<uint16_t> >& blocks)
{
    auto* dict = new vector<uint32_t>;
    map<uint32_t,int> occ;
    uint16_t nextSym = 0x0101;

    while (nextSym < 0x0580)
    {
        uint32_t maxPair = 0;
        int max = 0;
        occ.clear();

        for (auto& block: blocks)
            for (auto it = block.begin(); it != prev(block.end()); it++)
            {
                uint32_t p = (*next(it) << 16) | *it;
                if (occ.count(p) == 0)
                    occ[p] = 1;
                else
                    occ[p]++;

                if (occ[p] > max)
                {
                    max = occ[p];
                    maxPair = p;
                }
            }

        if (!maxPair)
            break;

        for (auto& block: blocks)
            replace_pair(block, maxPair, nextSym);
        dict->push_back(maxPair);
        nextSym++;
    }

    return dict;
}

vector<uint8_t> decompress(uint16_t v, const uint16_t* meta)
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
