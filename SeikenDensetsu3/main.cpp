#include <cstdio>
#include <iostream>
#include "block.hpp"

using namespace std;

uint8_t* read_file(FILE* file)
{
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    uint8_t* mem = new uint8_t[size];
    fread(mem, 1, size, file);
    fclose(file);

    return mem;
}

int main(int argc, const char* argv[])
{
    if (argc != 4 or (strcmp(argv[1], "-i") and strcmp(argv[1], "-e")))
    {
        cerr << "usage: sd3 [-ei] <romfile> <textfile>" << endl;
        cerr << "options:" << endl;
        cerr << "  -e   extract from ROM to text"    << endl;
        cerr << "  -i   (re)insert from text to ROM" << endl << endl;
        return 1;
    }
    string option = argv[1];
    auto romName  = argv[2];
    auto textName = argv[3];

    FILE* romFile = fopen(romName, "rb");
    if (romFile == NULL)
    {
        perror("romfile"); cerr << endl;
        return 1;
    }

    FILE* textFile = fopen(textName, (option == "-i") ? "rb" : "wb");
    if (textFile == NULL)
    {
        perror("textfile"); cerr << endl;
        return 1;
    }

    uint8_t* rom = read_file(romFile);
    auto* blocks = Block::extract_blocks(rom);

    if (option == "-e")
    {
        for (auto& block: *blocks)
            for (auto& sentence: block.get_sentences())
                fprintf(textFile, "%s\n", sentence.get_text().c_str());
    }

    else if (option == "-i")
    {
        string text((char*)read_file(textFile));

        //blocks->reinsert(text);
    }

    return 0;
}
