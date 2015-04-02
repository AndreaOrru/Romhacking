#include <cstdio>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include "extractor.hpp"

using namespace std;

const uint8_t events[] = {3, 4, 4, 4, 4, 3, 2, 4, 7, 9, 3, 3, 2, 2, 3, 3, 3, 2, 4, 0, 2, 3, 3, 3, 2, 3, 3, 2, 3, 4, 5, 5, 5, 3, 2, 3, 5, 5, 4, 3, 2, 2, 5, 2, 5, 3, 10, 7, 11, 4, 5, 4, 3, 2, 1, 3, 1, 2, 2, 2, 2, 3, 3, 2, 3, 2, 2, 2, 5, 3, 1, 2, 2, 3, 3, 3, 4, 3, 2, 2, 3, 5, 3, 3, 3, 6, 5, 5, 5, 7, 2, 13, 2, 7, 7, 2, 2, 2, 2, 2, 8, 7, 8, 2, 9, 2, 1, 3, 3, 2, 3, 3, 8, 2, 9, 2, 2, 2, 1, 6, 6, 1, 6, 3, 2, 3, 3, 3, 2, 2, 5, 4, 1, 5, 1, 1, 2, 2, 2, 2, 3, 6, 2, 3, 7, 2, 3, 2, 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 3, 3, 1, 3, 3, 1, 3, 3, 3, 3, 1, 3, 3, 1, 3, 3, 3, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 5, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1, 6, 1, 1};

const string names[] = { "BYUU", "YOYO", "SALMANDO", "ICEDRAKE", "THUNDERH", "MOLTEN", "TWINHEAD", "MUNIMUNI", "PUPPY", "FARNHEIT" };

set<uint16_t>* parse(vector<uint8_t>::iterator begin, vector<uint8_t>::iterator it, vector<uint8_t>::iterator end, set<uint16_t>* pointers = NULL)
{
    if (pointers == NULL)
        pointers = new set<uint16_t>;

    while (it < end)
    {
        if (*it == 0x13)
        {
            while (*it != 0xFF and *it != 0xFE)
                it++;
        }

        else if (*it == 0x37 or *it == 0x09 or *it == 0x08)
            pointers->insert(it + 1 - begin);

        it += events[*it];
    }

    return pointers;
}

set<uint16_t>* get_pointers(vector<uint8_t>* data)
{
    auto it  = data->begin() + (*data)[0] + ((*data)[1] << 8);
    auto end = data->begin() + (*data)[2] + ((*data)[3] << 8);

    auto pointers = parse(data->begin(), it, end);

    it = data->begin() + (*data)[6] + ((*data)[7] << 8);
    while (not (*it == 0xFF and *(it+1) == 0xFF))
    {
        auto entry = data->begin() + (*it) + (*(it+1) << 8);
        parse(data->begin(), entry, data->end(), pointers);
        it += 2;
    }

    return pointers;
}

string format(uint16_t pointer, vector<uint8_t>::iterator it)
{
    char str[8];
    sprintf(str, "%.4X", pointer);

    string s = "<BEGIN $" + string(str) + ">\n";

    while (true)
    {
        uint8_t c = *(it++);

        switch (c)
        {
            case '$':
                s += "“"; break;
            case '"':
                s += "”"; break;
            case 0xF4:
                s += "<" + names[*(it++)] + ">"; break;
            case 0xFD:
                s += "\n<ENDQ>\n\n";
                return s;
            case 0xFE:
                s += '\n'; break;
            case 0xFF:
                s += "\n<END>\n\n";
                return s;
            default:
                if (isprint(c))
                    s += c;
                else
                {
                    sprintf(str, "%.2X", c);
                    s += "<$" + string(str) + ">";
                }
        }
    }

    return s;
}

string format(uint16_t ptr, uint16_t oldPtr)
{
    char ptr_str[8];
    char oldPtr_str[8];

    sprintf(ptr_str, "%.4X", ptr);
    sprintf(oldPtr_str, "%.4X", oldPtr);

    string s = "<BEGIN $" + string(ptr_str) + ">\n";
    s += "<REF $" + string(oldPtr_str) + ">";
    s += "\n<END>\n\n";

    return s;
}

string extract(vector<uint8_t>* data)
{
    string s;
    uint16_t textStart = 0xFFFF;

    map<uint16_t, uint16_t> sentence2ptr;
    auto pointers = get_pointers(data);

    for (uint16_t p : *pointers)
    {
        uint16_t sentence = (*data)[p] + ((*data)[p+1] << 8);
        if (not (*data)[sentence])
            continue;

        if (sentence < textStart)
            textStart = sentence;

        if (sentence2ptr.count(sentence) == 0)
            s.append(format(p, data->begin() + sentence));
        else
            s.append(format(p, sentence2ptr[sentence]));

        sentence2ptr.insert(pair<uint16_t, uint16_t>(sentence, p));
    }

    char tmp[8];
    sprintf(tmp, "%.4X", textStart);
    s.insert(0, "<TEXT $" + string(tmp) + ">\n\n");

    return s;
}
