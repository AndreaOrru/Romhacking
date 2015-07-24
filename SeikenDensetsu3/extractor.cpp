#include <cctype>
#include <cstdio>
#include "extractor.hpp"

using namespace std;

string stringify(uint8_t c)
{
    switch (c)
    {
        case 0x10:
            return "<OPEN>";
        case 0x11:
            return "<CLOSE>";
        case 0x12:
            return "<PAGE>\n";
        case 0x17:
            return "\n";
        case 0x18:
            return "<WAIT>";
        case '_':
            return "...";
        default:
            if (isprint(c))
                return string(1, c);
            else
            {
                char s[6];
                sprintf(s, "<%.2X>", c);
                return string(s);
            }
    }
}

vector< pair< uint16_t,string> >* extract(vector<uint8_t>* data)
{
    auto sentences = new vector< pair<uint16_t,string> >;

    uint32_t i = 0;
    while (i < data->size())
    {
        uint16_t pos;
        string sentence;

        if ((*data)[i] == 0x58 or (*data)[i] == 0x5E)
        {
            pos = i;
            sentence += ((*data)[i++] == 0x58) ? "<BOX>" : "<LINE>";

            if ((*data)[i] != 0x10)
                continue;

            bool end = false;
            while (i < (data->size() - 1) and not(end = ((*data)[i] == 0xFF and (*data)[i+1] == 0xFF)))
                sentence += stringify((*data)[i++]);

            if (end and (i - pos) > 2)
                sentence += "<END>\n";
            else
                continue;

            i += 2;

            sentences->push_back(make_pair(pos, sentence));
        }
        else
            i++;
    }

    return sentences;
}
