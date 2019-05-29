#pragma once
#include "types.hpp"
#include <vector>

class ROM;

class Block {
public:
  const u8 index;
  u24 address;

  Block(const ROM *, u8);
  u24 size() const;

private:
  const ROM *rom;
  std::vector<u8> data;

  void initialize();
  void pushByte(u8);
  void pushWord(u16);
  void pushAddress(u24);
};
