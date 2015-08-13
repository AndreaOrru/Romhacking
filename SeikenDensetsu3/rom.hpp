#pragma once
#include "types.hpp"


namespace ROM
{

void open(const char* fname);
const u8* get_rom();
int get_size();

u8  rd  (int addr);
u16 rd_w(int addr);

}
