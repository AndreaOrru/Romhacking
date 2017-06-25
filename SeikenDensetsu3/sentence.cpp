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
map<pair<int,int>, Sentence*> Sentence::extractAll(vector<Block>& blocks)
{
    map<pair<int,int>, Sentence*> sentences;

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
                    auto sentence = new Sentence(&block, i, end);
                    sentence->stringify();
                    sentences[make_pair(block.begin, i)] = sentence;

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

// Reinsert the sentences into the blocks.
void Sentence::insertAll(map<pair<int,int>, Sentence*>& sentences)
{
    Block* block = nullptr;  // The block currently being considered.
    int diff;  // Difference in position of the sentences compared to original.

    // Iterate through the sentences in block and position order.
    for (auto& pair: sentences)
    {
        auto sentence = pair.second;
        // New block?
        if (sentence->block != block)
        {
            block = sentence->block;
            diff = 0;
        }

        // Update the sentence position inside the block.
        sentence->begin += diff;
        sentence->end   += diff;

        // Erase the old sentence.
        block->data.erase(block->data.begin() + sentence->begin,
                          block->data.begin() + sentence->end);

        // Serialize back the data into the ROM format.
        auto data = sentence->unstringify();
        // Calculate the difference in size and add it to the global count.
        diff += data.size() - (sentence->end - sentence->begin);
        // Update the new end of the sentence.
        sentence->end = sentence->begin + data.size();

        // Insert the new sentence into the block.
        block->data.insert(block->data.begin() + sentence->begin,
                           data.begin(), data.end());
    }
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

// Convert the script format into encoded bytes.
vector<u8> Sentence::unstringify()
{
    vector<u8> data;

    // Iterate through the text.
    for (size_t i = 0; i < text.size(); i++)
    {
        // Normal characters, not tags.
        if (text[i] != '<')
        {
            // New line.
            if (text[i] == '\n')
                data.push_back(0x17);
            // Ellipsis.
            else if (!text.compare(i, 3, "..."))
            {
                data.push_back('_'); i += 2;
            }
            // Any other printable character.
            else
                data.push_back((u8) text[i]);
        }
        // Tags.
        else
        {
            // Find the end of the tag.
            i++;
            unsigned sz = text.find('>', i) - i;
            bool nl = false;  // Is there a newline right after the tag?

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
                // It might be one of the main character names.
                u8 n;
                for (n = 0; n < 6; n++)
                    if (!text.compare(i, sz, names[n]))
                        break;
                // It is.
                if (n < 6)
                {
                    data.push_back(0x19); data.push_back(0xF8); data.push_back(n);
                }
                // It's not, it's just some weird byte.
                else
                {
                    u8 v; sscanf(&text.c_str()[i], "%hhX", &v);
                    data.push_back(v);
                }
            }

            i += sz;
            // Ignore the newline if it's part of the tag.
            if (nl and text.at(i+1) == '\n')
                i++;
        }
    }

    return data;
}
