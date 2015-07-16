#include <cstdio>
#include <iostream>
#include "extractor.hpp"
#include "lzss.hpp"
#include "reinserter.hpp"

using namespace std;

struct Address
{
    uintptr_t address : 24;

    uintptr_t addr() { return address - 0xC00000; }
} __attribute__((packed));

int main(int argc, char* argv[])
{
    auto fname = argv[1];
    auto room  = stoi(string(argv[2]));

    auto f = fopen(fname, "rb");
    fseek(f, 0, SEEK_END);
    auto size = ftell(f);
    rewind(f);

    auto rom = new uint8_t[size];
    fread(rom, 1, size, f);
    fclose(f);

    auto rooms = (Address*) &rom[0x1A8000];
    auto data = decompress(rom + rooms[room].addr());

    string text = extract(data);
    reinsert(data, text);

    auto comp = compress(data);

    return 0;
}
