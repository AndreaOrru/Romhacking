#include <cstdlib>
#include <iostream>
#include <regex>
#include "reinserter.hpp"

using namespace std;

const string names[] = { "BYUU", "YOYO", "SALMANDO", "ICEDRAKE", "THUNDERH", "MOLTEN", "TWINHEAD", "MUNIMUNI", "PUPPY", "FARNHEIT" };

void reinsert(vector<uint8_t>* data, string text)
{
    regex textRegex("<TEXT \\$([0-9A-F]+)>");
    smatch m;
    regex_search(text, m, textRegex);
    uint16_t textStart = strtoul(m[1].str().c_str(), NULL, 16);

    regex sentenceRegex("<BEGIN \\$([0-9A-F]+)>\n((.|\n)*?)\n<END(Q?)>");

    auto begin = sregex_iterator(text.begin(), text.end(), sentenceRegex);
    auto   end = sregex_iterator();

    auto output = data->begin() + textStart;
    for (auto i = begin; i != end; i++)
    {
        m = *i;

        uint16_t pointer = strtoul(m[1].str().c_str(), NULL, 16);
        string  sentence = m[2];
        bool          fd = m[4] == "Q";

        if (regex_match(sentence, m, regex("<REF \\$([0-9A-F]+)>")))
        {
            uint16_t ref = strtoul(m[1].str().c_str(), NULL, 16);
            (*data)[pointer  ] = (*data)[ref  ];
            (*data)[pointer+1] = (*data)[ref+1];
            continue;
        }

        // Substitute <NAMES>
        // Substitute <$xx>

        (*data)[pointer  ] =  (output - data->begin())       & 0xFF;
        (*data)[pointer+1] = ((output - data->begin()) >> 8) & 0xFF;

        if (output - data->begin() + sentence.size() > data->size())
            data->resize(output - data->begin() + sentence.size());

        output = copy(sentence.begin(), sentence.end(), output);
        *(output++) = fd ? 0xFD : 0xFF;
    }

    data->resize(output - data->begin());

    for (auto i: *data)
        cout << i;
    cout << endl;
}
