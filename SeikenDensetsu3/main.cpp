#include <cstdio>
#include <iostream>
#include "block.hpp"

using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 4 or (strcmp(argv[1], "-i") and strcmp(argv[1], "-e")))
    {
        cerr << "usage: sd3 [-ei] <romfile> <textfile>" << endl;
        cerr << "options:"         << endl;
        cerr << "  -e   extract from ROM to text"    << endl;
        cerr << "  -i   (re)insert from text to ROM" << endl << endl;
        return 1;
    }
    string option = argv[1];
    auto romName  = argv[2];
    auto textName = argv[3];

    auto romFile = fopen(romName, "rb");
    if (romFile == NULL)
    {
        perror("romfile"); cerr << endl;
        return 1;
    }
    fseek(romFile, 0, SEEK_END);
    auto romSize = ftell(romFile);
    rewind(romFile);

    auto textFile = fopen(textName, "wb");
    if (textFile == NULL)
    {
        perror("textfile"); cerr << endl;
        return 1;
    }

    auto* rom = new uint8_t[romSize];
    fread(rom, 1, romSize, romFile);
    fclose(romFile);

    if (option == "-e")
    {
        auto* blocks = Block::extractBlocks(rom);
        for (auto& b: *blocks)
        {
            auto& s = b.getSentences();
            for (int i = 0; i < s.size(); i++)
                fprintf(textFile, "[ Sentence $%X:%x ]\n%s\n", b.getIndex(), i, s[i].getText().c_str());
        }
    }

    return 0;
}
