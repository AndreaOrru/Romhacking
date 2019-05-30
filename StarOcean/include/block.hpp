#pragma once
#include "types.hpp"
#include <vector>

class ROM;

class Block {
public:
  u8 index;
  u24 address;
  u16 size;
  u8 type;

  Block(const ROM *, u8);
  std::vector<u8> decompressString(u8);

private:
  const ROM *rom;
  std::vector<u8> data;

  void initialize();
  std::vector<u8> huffman(u24, u16, u16);
  u8 readByte(u16) const;
  u16 readWord(u16) const;
  u24 readAddress(u16) const;
  static void pushByte(std::vector<u8> &, u8);
  static void pushWord(std::vector<u8> &, u16);
  static void pushAddress(std::vector<u8> &, u24);
  static void specialPush(std::vector<u8> &, u8);
};
