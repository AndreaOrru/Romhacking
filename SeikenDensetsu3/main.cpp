#include <cstdio>
#include <iostream>
#include <string>
#include "huffman.hpp"

using namespace std;

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

    uint16_t offset = ((uint16_t*) &rom[0x3E2E00])[room];
    uint8_t  bank   = rom[0xEA91 + (rom[0x3E4E00 + room/2] & 0xF)];

    uint16_t* data = (uint16_t*) (rom + ((bank << 16) + offset) - 0xC00000);
    uint16_t* meta = (uint16_t*) (rom + 0x3E6600);

    auto out = decompress(data, meta);

    for (auto it: *out)
        cout << it;
    cout << endl;

    return 0;
}
