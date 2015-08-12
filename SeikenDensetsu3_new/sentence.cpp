#include "block.hpp"
#include "sentence.hpp"

using namespace std;


const string Sentence::names[] = { "DURAN", "KEVIN", "HAWK", "ANGELA", "CARLIE", "LISE" };


void Sentence::insert_sentences(vector<Sentence>& sentences)
{
    Block* block = nullptr;
    int diff = 0;

    for (auto& sentence: sentences)
    {
        if (sentence.block != block)
        {
            block = sentence.block;
            diff = 0;
        }

        sentence.begin += diff;
        sentence.end   += diff;

        block->data.erase(block->data.begin() + sentence.begin,
                          block->data.begin() + sentence.end);

        vector<u8> data = sentence.unstringify();
        diff += data.size() - (sentence.end - sentence.begin);
        sentence.end = sentence.begin + data.size();

        block->data.insert(block->data.begin() + sentence.begin,
                           data.begin(), data.end());
    }
}

vector<Sentence> Sentence::extract_sentences(vector<Block>& blocks)
{
    vector<Sentence> sentences;

    for (auto& block: blocks)
    {
        u32 i = 0;
        while (i < block.data.size())
        {
            u8 b = block.data[i];
            if (b == 0x58 or b == 0x5E or b == 0x7B or b == 0xF2 or
                b == 0xF3 or b == 0xF8 or b == 0xFA)
            {
                u32 end = try_sentence(block.data, i);
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

bool Sentence::check(vector<u8>& data, u32 begin, u32 end)
{
    int printable = 0;
    for (u32 i = begin + 1; i < end; i++)
        if (data[i] == 0x17 or (data[i] >= 0x20 and data[i] <= 0x7E))
            printable++;
    if (printable == 0)
        return false;

    auto in = [=](vector<u8> seq)
        { return search(data.begin(), data.end(), seq.begin(), seq.end()) != data.end(); };

    if (data[begin] >= 0xF2 and (not in({0x14}) or in({0x12}) or in({0x58, 0x10}) or
                                     in({0x5E, 0x10}) or in({0x11, 0xFF, 0xFF})))
        return false;

    return true;
}

u32 Sentence::try_sentence(vector<u8>& data, u32 begin)
{
    int n = (data[begin] == 0x7B) ? 6 : 1;
    u32 i = begin;

    while (n > 0)
    {
        while (true)
        {
            if (not (i+1 < data.size()))
                return 0;
            if (data[i] == 0xFF and data[i+1] == 0xFF)
                break;
            i++;
        }

        i += 2;
        n--;
    }

    return check(data, begin, i) ? i : 0;
}

void Sentence::stringify()
{
    vector<u8>& data = block->data;
    u32 i = begin;

         if (data[i] == 0x58) { text = "<BOX>" ; i++; }
    else if (data[i] == 0x5E) { text = "<LINE>"; i++; }
    else if (data[i] == 0x7B) { text = "<ALT>" ; i++; }

    for (; i < end; i++)
    {
        if (i+2 < data.size() and data[i] == 0x19 and data[i+1] == 0xF8)
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
            text += s;
        }
    }
}

vector<u8> Sentence::unstringify()
{
    vector<u8> data;

    for (size_t i = 0; i < text.size(); i++)
    {
        if (not (text[i] == '<'))
        {
            if (text[i] == '\n')
                data.push_back(0x17);
            else if (!text.compare(i, 3, "..."))
            {
                data.push_back('_'); i += 2;
            }
            else
                data.push_back(text[i]);
        }
        else
        {
            i++;
            int sz = text.find('>', i) - i;
            bool nl = false;

                 if (!text.compare(i, sz, "BOX"))    data.push_back(0x58);
            else if (!text.compare(i, sz, "LINE"))   data.push_back(0x5E);
            else if (!text.compare(i, sz, "ALT"))    data.push_back(0x7B);
            else if (!text.compare(i, sz, "OPEN"))   data.push_back(0x10);
            else if (!text.compare(i, sz, "CLOSE"))  data.push_back(0x11);
            else if (!text.compare(i, sz, "PAGE")) { data.push_back(0x12); nl = true; }
            else if (!text.compare(i, sz, "OR"))     data.push_back(0x14);
            else if (!text.compare(i, sz, "WAIT"))   data.push_back(0x18);

            else if (!text.compare(i, sz, "END"))
            {
                data.push_back(0xFF); data.push_back(0xFF); nl = true;
            }
            else if (!text.compare(i, sz, "MULTI"))
            {
                data.push_back(0xF8); data.push_back(0x01);
            }
            else if (!text.compare(i, sz, "CHOICE"))
            {
                data.push_back(0xF3); data.push_back(0x00);
            }
            else
            {
                int n;
                for (n = 0; n < 6; n++)
                    if (!text.compare(i, sz, names[n]))
                        break;
                if (n < 6)
                {
                    data.push_back(0x19); data.push_back(0xF8); data.push_back(n);
                }
                else
                {
                    u8 v; sscanf(&text.c_str()[i], "%hhX", &v);
                    data.push_back(v);
                }
            }

            i += sz;
            if (nl and text.at(i+1) == '\n')
                i++;
        }
    }

    return data;
}
