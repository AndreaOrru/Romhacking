#include <cstdio>
#include "rom.hpp"

using namespace std;


namespace ROM
{

u8* rom;
int size;


void open(const char* fname)
{
    FILE* file = fopen(fname, "rb");

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);

    rom = new u8[size];
    fread(rom, 1, (size_t) size, file);
    fclose(file);
}

const u8* get_rom()
{
    return rom;
}

int get_size()
{
    return size;
}

u8 rd(int addr)
{
    return rom[addr];
}

u16 rd_w(int addr)
{
    return *((u16*) &rom[addr]);
}

}
