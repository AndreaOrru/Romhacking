#pragma once
#include "types.hpp"
#include <vector>

class ROM;

class Block {
public:
  u8 index;
  u24 address;
  u16 size;

  Block(const ROM *, u8);
  void decompressString(u8);

private:
  const ROM *rom;
  std::vector<u8> data;

  void initialize();
  u8 readByte(u16) const;
  u16 readWord(u16) const;
  u24 readAddress(u16) const;
  void pushByte(u8);
  void pushWord(u16);
  void pushAddress(u24);
};
