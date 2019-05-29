#pragma once
#include "types.hpp"

class ROM;

class Block {
public:
  const u8 index;
  const u24 address;

  Block(const ROM *, u8, u24);
  u24 size() const;

private:
  const ROM *rom;
};
