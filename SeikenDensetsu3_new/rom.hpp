#pragma once
#include "types.hpp"


namespace ROM
{

void open(const char* fname);
const u8* get_rom();
size_t get_size();

u8  rd  (u32 addr);
u16 rd_w(u32 addr);

}
