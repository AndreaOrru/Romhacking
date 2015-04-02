#include <algorithm>
#include <iostream>
#include "lzss.hpp"

using namespace std;

union Recovery
{
    struct
    {
        uint8_t low;
        uint8_t high;
    } __attribute__((packed));

    struct
    {
        unsigned jump : 12;
        unsigned size : 4;
    } __attribute__((packed));
} __attribute__((packed));

Recovery get_match(vector<uint8_t>::iterator begin,
                   vector<uint8_t>::iterator curr,
                   vector<uint8_t>::iterator end)
{
    uint16_t maxSize = min(18L, end - curr);
    uint8_t size = 3;
    Recovery rec = { 0, 0 };

    while (size <= maxSize)
    {
        auto match = search(begin, curr, curr, curr + size);

        if (match != curr)
        {
            rec.jump = curr - match;
            rec.size = size - 3;
            size++;
        }
        else
            break;
    }

    return rec;
}

vector<uint8_t>* compress(vector<uint8_t>* data)
{
    auto out = new vector<uint8_t>;
    out->push_back(0x00);
    out->push_back(0x00);

    uint16_t block = out->size();
    uint8_t byte = 0;

    auto curr = data->begin();
    while (curr != data->end())
    {
        if (byte == 0)
        {
            block = out->size();
            out->push_back(0x00);
        }

        int distance = min(curr - data->begin(), 4095L);
        auto rec = get_match(curr - distance, curr, data->end());

        if (rec.jump == 0)
        {
            out->push_back(*curr);
            curr++;
        }
        else
        {
            out->push_back(rec.low);
            out->push_back(rec.high);
            curr += rec.size + 3;
            (*out)[block] |= (1 << byte);
        }

        byte = (byte + 1) % 8;
    }

    (*out)[0] =  (out->size() - 2)       & 0xFF;
    (*out)[1] = ((out->size() - 2) >> 8) & 0xFF;
    out->push_back(0x00);

    return out;
}

vector<uint8_t>* decompress(vector<uint8_t>* data)
{
    return decompress(data->data());
}

vector<uint8_t>* decompress(uint8_t* data)
{
    auto out = new vector<uint8_t>;

    uint16_t size = ((uint16_t*) data)[0] + 2;
    uint16_t i = 2;

    while (true)
    {
        if (i > size)
            break;
        else if ((i == size) && (data[i] != 0))
        {
            size = *((uint16_t*) &data[++i]);
            i += 2;
        }

        uint8_t info = data[i++];

        for (uint8_t j = 0; j < 8; j++)
        {
            if (i >= size)
                break;

            if ((info >> j) & 1)
            {
                auto rec = (Recovery*) &data[i];
                out->insert(out->end(), out->end() - rec->jump,
                                        out->end() - rec->jump + rec->size + 3);
                i += 2;
            }
            else
                out->push_back(data[i++]);
        }
    }

    return out;
}
