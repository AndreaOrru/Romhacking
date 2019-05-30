#pragma once
#include "types.hpp"
#include <vector>

class ROM;
class Sentence;

enum BlockType : u8 {
  TEXT = 0x85,
  EVENTS = 0x86,
  OTHER = 0x80,
};

class Block {
public:
  u8 index;
  u24 address;
  u16 size;
  BlockType type;

  Block(const ROM *, u8);
  std::vector<Sentence> extract();

private:
  const ROM *rom;
  std::vector<u8> data;

  std::vector<u8> decompressString(u8);
  void initialize();
  u8 readByte(u16) const;
  u16 readWord(u16) const;
  u24 readAddress(u16) const;
  static void pushByte(std::vector<u8> &, u8);
  static void pushWord(std::vector<u8> &, u16);
  static void pushAddress(std::vector<u8> &, u24);
  static void specialPush(std::vector<u8> &, u8);
  std::vector<u8> huffman(u24, u16, u16);
  std::vector<u8> decode(const std::vector<u8> &);
};
