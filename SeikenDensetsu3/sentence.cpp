#include <cctype>
#include <cstdio>
#include <regex>
#include "sentence.hpp"

using namespace std;

const string Sentence::names[] = { "DURAN", "KEVIN", "HAWK", "ANGELA", "CARLIE", "LISE" };

Sentence::Sentence(uint16_t block, uint16_t index, uint32_t pos,
                   vector<uint8_t>::const_iterator begin,
                   vector<uint8_t>::const_iterator end)
{
    this->block = block;
    this->index = index;
    this->pos   = pos;
    data.assign(begin, end);

    format();
}

Sentence::Sentence(uint16_t block, uint16_t index, string content)
{
    this->block = block;
    this->index = index;

    unformat(content);
}

vector<Sentence>* Sentence::extract_sentences(const string& text)
{
    auto* sentences = new vector<Sentence>;

    regex sentenceRegex("\\[Sentence \\$([0-9A-F]+):([0-9a-f]+)\\]\n((.|\n)*?)\n\\[/Sentence\\]");
    auto begin = sregex_iterator(text.begin(), text.end(), sentenceRegex);
    auto   end = sregex_iterator();
    smatch m;

    for (auto i = begin; i != end; i++)
    {
        m = *i;

        uint16_t block = stoi(m[1], 0, 16);
        uint16_t index = stoi(m[2], 0, 16);
        sentences->emplace_back(block, index, m[3]);
    }

    return sentences;
}

string Sentence::stringify(uint8_t c)
{
    switch (c)
    {
        case 0x10:  return "<OPEN>";
        case 0x11:  return "<CLOSE>";
        case 0x12:  return "<PAGE>\n";
        case 0x14:  return "<OR>";
        case 0x17:  return "\n";
        case 0x18:  return "<WAIT>";
        case  '_':  return "...";
        default:
            if (isprint(c))
                return string(1, c);
            else
            {
                char s[5];
                sprintf(s, "<%.2X>", c);
                return string(s);
            }
    }
}

void Sentence::format()
{
    for (auto c: data)
        text += stringify(c);

    for (int i = 0; i < 6; i++)
        text = regex_replace(text, regex("<19><F8><0"+ to_string(i) +">"), "<"+ names[i] +">");

    text = regex_replace(text, regex("<FF><FF>"),                  "<END>\n");
    text = regex_replace(text, regex("<F8><01>"),                  "<MULTI>");
    text = regex_replace(text, regex("<F3><00>"),                  "<CHOICE>");

    text = regex_replace(text, regex("\\{((((.|\n)*?)<END>){6})"), "<ALTERN>$1");
    text = regex_replace(text, regex("(X|\\^)((.|\n)*?)<END>"),    "<$1>$2<END>");

    text = regex_replace(text, regex("<X>"),                       "<BOX>");
    text = regex_replace(text, regex("<\\^>"),                     "<LINE>");

    char header[32];
    sprintf(header, "[Sentence $%X:%x]\n", block, index);
    text.insert(0, header);
    text.append("[/Sentence]\n");
}

void Sentence::unformat(string& content)
{
    content = regex_replace(content, regex("<LINE>"),    "^");
    content = regex_replace(content, regex("<BOX>"),     "X");
    content = regex_replace(content, regex("<ALTERN>"),  "{");
    content = regex_replace(content, regex("<CHOICE>"),  "<F3><00>");
    content = regex_replace(content, regex("<MULTI>"),   "<F8><01>");
    content = regex_replace(content, regex("<END>"),     "<FF><FF>");

    for (int i = 0; i < 6; i++)
        content = regex_replace(content, regex("<"+ names[i] +">"), "<19><F8>0"+ to_string(i) +">");

    content = regex_replace(content, regex("<OPEN>"),    "<10>");
    content = regex_replace(content, regex("<CLOSE>"),   "<11>");
    content = regex_replace(content, regex("<PAGE>\n?"), "<12>");
    content = regex_replace(content, regex("<OR>"),      "<14>");
    content = regex_replace(content, regex("<WAIT>"),    "<18>");
    content = regex_replace(content, regex("_"),         "...");
    content = regex_replace(content, regex("\n"),        "<17>");

    for (auto it = content.begin(); it < content.end(); it++)
        if (*it != '<')
            data.push_back(*it);
        else
        {
            // Assume (it+2) is in range.
            data.push_back(stoi(string(it+1, it+2), 0, 16));
            it += 3;
        }
}

const string& Sentence::get_text()
{
    if (text.empty())
        format();

    return text;
}
