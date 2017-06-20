#include <cstdio>
#include "rom.hpp"

using namespace std;


namespace ROM
{

u8* rom;
int size;


// Open the ROM.
void open(const string file_name)
{
    FILE* file = fopen(file_name.c_str(), "rb");

    // Get file size.
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);

    // Load all the data from the ROM.
    rom = new u8[size];
    fread(rom, 1, (size_t) size, file);
    fclose(file);
}

// Read a byte from the given (PC) address.
u8 readByte(int address)
{
    return rom[address];
}

// Read a word from the given (PC) address.
u16 readWord(int address)
{
    return (rom[address + 1] << 8) |
            rom[address];
}

}
