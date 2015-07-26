#include <cctype>
#include <cstdio>
#include <regex>
#include "sentence.hpp"

using namespace std;

const string Sentence::names[] = { "DURAN", "KEVIN", "HAWK", "ANGELA", "CARLIE", "LISE" };

Sentence::Sentence(uint32_t pos, vector<uint8_t>::const_iterator begin,
                                 vector<uint8_t>::const_iterator end)
{
    this->pos = pos;
    data.assign(begin, end);
}

const string& Sentence::getText()
{
    if (text.empty())
        format();

    return text;
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

    text = regex_replace(text, regex("\\{((((.|\n)*?)<END>){6})"), "<ALT>$1");
    text = regex_replace(text, regex("(X|\\^)((.|\n)*?)<END>"),    "<$1>$2<END>");

    text = regex_replace(text, regex("<X>"),                       "<BOX>");
    text = regex_replace(text, regex("<\\^>"),                     "<LINE>");
}
