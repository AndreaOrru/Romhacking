#include <iostream>
#include <cstdio>
#include "block.hpp"

using namespace std;

int main(int argc, char* argv[])
{
    auto romName = argv[1];

    auto romFile = fopen(romName, "rb");
    fseek(romFile, 0, SEEK_END);
    auto romSize = ftell(romFile);
    rewind(romFile);

    auto rom = new uint8_t[romSize];
    fread(rom, 1, romSize, romFile);
    fclose(romFile);

    auto blocks = getBlocks(rom);
    for (auto& b: *blocks)
    {
        b.extract();
        for (int i = 0; i < b.sentences.size(); i++)
        {
            auto& s = b.sentences[i];
            printf("[ Sentence %X:%X (%X-%lX) ]\n%s\n",
                   b.index, i, s.pos, s.pos + s.data.size(), s.text.c_str());
        }
    }

    return 0;
}
