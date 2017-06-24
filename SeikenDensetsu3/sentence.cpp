#include "sentence.hpp"

using namespace std;


// Default names of main characters.
const string Sentence::names[] = {
    "DURAN",
    "KEVIN",
    "HAWK",
    "ANGELA",
    "CARLIE",
    "LISE",
};


// Extract all sentences from the given blocks.
vector<Sentence> Sentence::extractAll(vector<Block>& blocks)
{
    vector<Sentence> sentences;

    // Iterate through the blocks.
    for (Block& block: blocks)
    {
        // Iterate through the block's bytes.
        size_t i = 0;
        while (i < block.data.size())
        {
            // Check for dialogue event codes.
            u8 b = block.data[i];
            if (b == 0x58 || b == 0x5E || b == 0x7B || b == 0xF2 ||
                b == 0xF3 || b == 0xF8 || b == 0xFA)
            {
                // Find where the sentence ends - or if it is a sentence at all.
                int end = trySentence(block.data, i);
                if (end)
                {
                    // Create the new sentence and add it to the list.
                    Sentence sentence(&block, i, end);
                    sentence.stringify();
                    sentences.push_back(sentence);

                    // Restart searching from the end of the sentence.
                    i = end;
                    continue;
                }
            }
            i += 1;
        }
    }

    return sentences;
}

// Search for a sentence in data starting at begin.
// Return 0 if it's not a sentence.
int Sentence::trySentence(vector<u8>& data, int begin)
{
    // Check for sentences spoken by multiple characters.
    int n = (data[begin] == 0x7B) ? 6 : 1;
    size_t i = begin;

    while (n > 0)
    {
        while (true)
        {
            // End of the block.
            if (i+1 >= data.size())
                return 0;
            // End of the sentence.
            if (data[i] == 0xFF and data[i+1] == 0xFF)
                break;
            i++;
        }

        i += 2;
        n--;
    }

    // Check whether this looks like a sentence at all.
    if (check(data.begin() + begin, data.begin() + i))
        return i;
    else
        return 0;
}

// Check whether the given slice of bytes is a sentence or not.
bool Sentence::check(vector<u8>::iterator begin, vector<u8>::iterator end)
{
    // NOTE: we use begin+1 because the first byte signals the opening of the dialogue box.

    // Count the number of printable characters in the string.
    int printable = 0;
    for (auto it = begin+1; it < end; it++)
        if (*it == 0x17 or (*it >= 0x20 and *it <= 0x7E))
            printable++;
    // With no printable characters it's certainly not a sentence.
    if (printable == 0)
        return false;

    // Check whether the given sequence is part of the sentence.
    auto in = [=](vector<u8> seq)
              { return search(begin+1, end, seq.begin(), seq.end()) != end; };

    if (in({0x58, 0x10}) or in({0x5E, 0x10}))
        return false;
    if (*begin >= 0xF2 and (!in({0x14}) or in({0x12}) or in({0x11, 0xFF, 0xFF})))
        return false;

    return true;
}

// Convert the raw bytes into a readable script format.
void Sentence::stringify()
{
    vector<u8>& data = block->data;
    size_t i = begin;

         if (data[i] == 0x58) { text = "<BOX>" ; i++; }
    else if (data[i] == 0x5E) { text = "<LINE>"; i++; }
    else if (data[i] == 0x7B) { text = "<ALT>" ; i++; }

    for (; i < (size_t) end; i++)
    {
        // Name of one of the main characters.
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

        // Printable characters (exclude the ones we use for our tags).
        else if (data[i] >= 0x20 and data[i] <= 0x7E and
                 data[i] != '['  and data[i] != ']'  and
                 data[i] != '<'  and data[i] != '>')
        {
            text += data[i];
        }
        // Non-printable bytes.
        else
        {
            char s[5];
            sprintf(s, "<%.2X>", data[i]);
            text.append(s);
        }
    }
}
