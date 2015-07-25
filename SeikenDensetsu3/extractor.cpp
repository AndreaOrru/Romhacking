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

Script extract(vector<uint8_t>* data)
{
    auto sentences = new vector<Sentence>;

    uint32_t i = 0;
    while (i < data->size())
    {
        uint16_t pos = i;
        string text;
        bool end;

        if ((*data)[i] == 0x58 or (*data)[i] == 0x5E)
        {
            text += ((*data)[i++] == 0x58) ? "<BOX>" : "<LINE>";

            if ((*data)[i] != 0x10)
                continue;

            end = false;
            while (i < (data->size() - 1) and not(end = ((*data)[i] == 0xFF and (*data)[i+1] == 0xFF)))
                text += stringify((*data)[i++]);

            if (end and (i - pos) > 2)
                text += "<END>\n";
            else
                continue;
            i += 2;

            sentences->emplace_back(pos, text);
        }
        else if ((*data)[i] == 0x7B)
        {
            text += "<ALT>";
            i++;

            for (int n = 0; n < 6; n++)
            {
                end = false;
                while (i < (data->size() - 1) and not(end = ((*data)[i] == 0xFF and (*data)[i+1] == 0xFF)))
                    text += stringify((*data)[i++]);

                if (end)
                    text += "<END>\n";
                i += 2;
            }
            if (!end)
                continue;

            sentences->emplace_back(pos, text);
        }
        else
            i++;
    }

    return Script(data, sentences);
}
