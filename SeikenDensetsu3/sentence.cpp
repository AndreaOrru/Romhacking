#include "sentence.hpp"

using namespace std;


// Default names of main characters.
const string Sentence::names[] = {
    "DURAN",
    "KEVIN",
    "HAWK",
    "ANGELA",
    "CARLIE",
    "LISE"
};


// Extract all sentences from the given blocks.
vector<Sentence> Sentence::extractAll(vector<Block>& blocks)
{
    vector<Sentence> sentences;

    for (auto& block: blocks)
    {
        size_t i = 0;
        while (i < block.data.size())
        {
            u8 b = block.data[i];
            if (b == 0x58 or b == 0x5E or b == 0x7B or b == 0xF2 or
                b == 0xF3 or b == 0xF8 or b == 0xFA)
            {
                int end = try_sentence(block.data, i);
                if (end)
                {
                    sentences.emplace_back(&block, i, end);
                    sentences.back().stringify();
                    i = end;
                    continue;
                }
            }
            i += 1;
        }
    }

    return sentences;
}

bool Sentence::check(vector<u8>::iterator begin, vector<u8>::iterator end)
{
    int printable = 0;
    for (auto it = begin+1; it < end; it++)
        if (*it == 0x17 or (*it >= 0x20 and *it <= 0x7E))
            printable++;
    if (printable == 0)
        return false;

    auto in = [=](vector<u8> seq)
        { return search(begin+1, end, seq.begin(), seq.end()) != end; };

    if (in({0x58, 0x10}) or in({0x5E, 0x10}))
        return false;
    if (*begin >= 0xF2 and (!in({0x14}) or in({0x12}) or in({0x11, 0xFF, 0xFF})))
        return false;

    return true;
}

int Sentence::try_sentence(vector<u8>& data, int begin)
{
    int n = (data[begin] == 0x7B) ? 6 : 1;
    size_t i = begin;

    while (n > 0)
    {
        while (true)
        {
            if (i+1 >= data.size())
                return 0;
            if (data[i] == 0xFF and data[i+1] == 0xFF)
                break;
            i++;
        }

        i += 2;
        n--;
    }

    return check(data.begin() + begin, data.begin() + i) ? i : 0;
}

void Sentence::stringify()
{
    vector<u8>& data = block->data;
    size_t i = begin;

         if (data[i] == 0x58) { text = "<BOX>" ; i++; }
    else if (data[i] == 0x5E) { text = "<LINE>"; i++; }
    else if (data[i] == 0x7B) { text = "<ALT>" ; i++; }

    for (; i < (size_t) end; i++)
    {
        if (i+2 < data.size() and data[i] == 0x19 and data[i+1] == 0xF8 and data[i+2] < 6)
        {
            text += "<" + names[data[i+2]] + ">"; i += 2;
        }

        else if (data[i] == 0x10) text += "<OPEN>";
        else if (data[i] == 0x11) text += "<CLOSE>";
        else if (data[i] == 0x12) text += "<PAGE>\n";
        else if (data[i] == 0x14) text += "<OR>";
        else if (data[i] == 0x17) text += '\n';
        else if (data[i] == 0x18) text += "<WAIT>";
        else if (data[i] == '_' ) text += "...";

        else if (i+1 < data.size() and data[i] == 0xFF and data[i+1] == 0xFF)
        {
            text += "<END>\n"; i++;
        }
        else if (i+1 < data.size() and data[i] == 0xF8 and data[i+1] == 0x01)
        {
            text += "<MULTI>"; i++;
        }
        else if (i+1 < data.size() and data[i] == 0xF3 and data[i+1] == 0x00)
        {
            text += "<CHOICE>"; i++;
        }

        else if (data[i] >= 0x20 and data[i] <= 0x7E and
                 data[i] != '['  and data[i] != ']'  and
                 data[i] != '<'  and data[i] != '>')
        {
            text += data[i];
        }
        else
        {
            char s[5];
            sprintf(s, "<%.2X>", data[i]);
            text.append(s);
        }
    }
}
