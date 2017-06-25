#include <cstdio>
#include "rom.hpp"

using namespace std;


namespace ROM
{

u8* rom;
int size;
string rom_path;


// Open the ROM.
void open(const string file_name)
{
    rom_path = file_name;
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

// Save the ROM.
void save()
{
    FILE* file = fopen(rom_path.c_str(), "wb");
    fwrite(rom, sizeof(u8), size, file);
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

// Write a byte to the given (PC) address.
void writeByte(int address, u8 byte)
{
    rom[address] = byte;
}

// Read a word from the given (PC) address.
void writeWord(int address, u16 word)
{
    rom[address]     = word & 0xFF;
    rom[address + 1] = word >> 8;
}

}
