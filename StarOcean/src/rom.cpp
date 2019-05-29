#include "rom.hpp"
#include <cstdio>

ROM::ROM(const char *path) {
  FILE *file = fopen(path, "rb");

  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  fseek(file, 0, SEEK_SET);

  data = new u8[size];
  fread(data, size, 1, file);
  fclose(file);
}

ROM::~ROM() { delete data; }

u8 ROM::readByte(u24 address) const { return data[translateAddress(address)]; }

u16 ROM::readWord(u24 address) const {
  u8 lo = readByte(address);
  u8 hi = readByte(address + 1);
  return (hi << 8) | lo;
}

u24 ROM::readAddress(u24 address) const {
  u16 lo = readWord(address);
  u8 hi = readByte(address + 2);
  return (hi << 16) | lo;
}

u24 ROM::translateAddress(u24 address) const { return address & ~0xC00000; }
