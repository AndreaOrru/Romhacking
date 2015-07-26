#include <cstdio>
#include <iostream>
#include "block.hpp"

using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        cerr << "usage: sd3 <file>" << endl << endl;
        return 1;
    }
    auto romName = argv[1];

    auto romFile = fopen(romName, "rb");
    if (romFile == NULL)
    {
        perror("error"); cerr << endl;
        return 1;
    }
    fseek(romFile, 0, SEEK_END);
    auto romSize = ftell(romFile);
    rewind(romFile);

    auto* rom = new uint8_t[romSize];
    fread(rom, 1, romSize, romFile);
    fclose(romFile);

    auto* blocks = Block::getBlocks(rom);
    for (auto& b: *blocks)
        for (int i = 0; i < b.getSentences().size(); i++)
        {
            auto& s = b.getSentences()[i];
            printf("[ Sentence %X:%X (%X-%X) ]\n%s\n",
                   b.getIndex(), i, s.getPos(), s.getPos() + s.getSize(), s.getText().c_str());
        }

    return 0;
}
