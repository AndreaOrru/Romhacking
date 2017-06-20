#include "rom.hpp"
#include "bytepair.hpp"

using namespace std;


namespace BytePair
{

// Cache of expanded bytepairs.
vector<vector<u8>> cache;


// Recursively expand a bytepair.
vector<u8> recExpand(u16 word)
{
    // Special case.
    if (word == 0x100)
        return { 0xFF, 0xFF };
    // Uncompressed data.
    else if (word < 0x100)
        return { (u8) word };
    // Index into bytepair dictionary.
    int i = word - 0x101;

    // Expand first and second word and concatenate.
    vector<u8> s1 = recExpand(ROM::readWord(0x3E6604 + i*4));
    vector<u8> s2 = recExpand(ROM::readWord(0x3E6604 + i*4 + 2));
    s1.insert(s1.end(), s2.begin(), s2.end());

    return s1;
}

// Build a cache with all the expanded bytepairs.
void init()
{
    for (u16 i = 0; i < 0x580; i++)
        cache.push_back(recExpand(i));
}

// Expand a bytepair.
const vector<u8>& expand(u16 word)
{
    // Build the cache the first time.
    if (cache.empty())
        init();

    return cache[word];
}

}
