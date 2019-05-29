#include "block.hpp"
#include "rom.hpp"

using namespace std;

Block::Block(const ROM *rom, u8 index) : index(index), rom(rom) {
  address = rom->readAddress(0xE40000 + (index * 3));

  initialize();
}

u24 Block::size() const { return rom->readWord(address + 9) / 8; }

void Block::initialize() {
  data.clear();

  u24 start = address + 9;
  for (u24 i = 0; i < size(); i++) {
    u8 byte = rom->readByte(start + i);
    pushByte(byte);
  }

  pushWord(rom->readWord(address + 1));
  pushAddress(start);
  pushByte(0x00);
}

void Block::pushByte(u8 value) { data.push_back(value); }

void Block::pushWord(u16 value) {
  pushByte(value & 0xFF);
  pushByte(value >> 8);
}

void Block::pushAddress(u24 value) {
  pushByte(value & 0xFF);
  pushWord(value >> 8);
}
