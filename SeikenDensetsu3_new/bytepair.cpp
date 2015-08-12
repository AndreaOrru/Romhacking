#include <array>
#include "rom.hpp"
#include "bytepair.hpp"

#include <iostream>

using namespace std;

namespace BytePair
{

vector<vector<u8>> cache;
vector<u32> dict;


void replace_pair(list<u16>& data, u32 pair, u16 symbol)
{
    for (auto it = data.begin(); it != prev(data.end()); it++)
        if (*it == (pair & 0xFFFF) and *next(it) == (pair >> 16))
        {
            *it = symbol;
            data.erase(next(it));
        }
}

const vector<u32>& get_dict()
{
    return dict;
}

vector<list<u16>> compress(const vector<Block>& blocks)
{
    vector<list<u16>> buffers;
    array<array<int,0x580>, 0x580> occ;

    for (auto& block: blocks)
        buffers.emplace_back(block.data.begin(), block.data.end());

    for (auto& buffer: buffers)
        replace_pair(buffer, 0x00FF00FF, 0x100);

    for (u16 symbol = 0x101; symbol < 0x580; symbol++)
    {
        u32 max_pair = 0;
        int max_occ  = 0;
        for (auto& arr: occ)
            arr.fill(0);

        for (auto& buffer: buffers)
            for (auto it = buffer.begin(); it != prev(buffer.end()); it++)
            {
                int curr_occ = ++occ[*it][*next(it)];

                if (curr_occ > max_occ)
                {
                    max_occ  = curr_occ;
                    max_pair = (*next(it) << 16) | *it;
                }
            }

        if (max_occ < 2)
            break;

        for (auto& buffer: buffers)
            replace_pair(buffer, max_pair, symbol);
        dict.push_back(max_pair);
    }
    return buffers;
}

vector<u8> expand(u16 w)
{
    if (w == 0x100)
        return { 0xFF, 0xFF };
    else if (w < 0x100)
        return { (u8) w };
    w -= 0x101;

    vector<u8> s  = expand(ROM::rd_w(0x3E6604 + w*4));
    vector<u8> s2 = expand(ROM::rd_w(0x3E6604 + w*4 + 2));
    s.insert(s.end(), s2.begin(), s2.end());

    return s;
}

void init()
{
    for (int i = 0; i < 0x580; i++)
        cache.push_back(expand(i));
}

const vector<u8>& get(u16 w)
{
    if (cache.empty())
        init();

    return cache[w];
}

}
