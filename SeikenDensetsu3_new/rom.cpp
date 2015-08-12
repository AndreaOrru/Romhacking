#include <cstdio>
#include "rom.hpp"

using namespace std;


namespace ROM
{

u8* rom;
size_t size;


void open(const char* fname)
{
    FILE* file = fopen(fname, "rb");

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);

    rom = new u8[size];
    fread(rom, 1, size, file);
    fclose(file);
}

const u8* get_rom()
{
    return rom;
}

size_t get_size()
{
    return size;
}

u8 rd(u32 addr)
{
    return rom[addr];
}

u16 rd_w(u32 addr)
{
    return *((u16*) &rom[addr]);
}

}
