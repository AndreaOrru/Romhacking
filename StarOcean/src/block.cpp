#include "block.hpp"
#include "rom.hpp"

using namespace std;

Block::Block(const ROM *rom, u8 index) : index(index), rom(rom) {
  address = rom->readAddress(0xE40000 + (index * 3));
  size = rom->readWord(address + 9) / 8;

  initialize();
}

vector<u8> Block::decompressString(u8 string_index) {
  vector<u8> buffer;

  u16 decrypt_start = size + 6;
  u24 rom_data_start = readAddress(size + 2);

  u16 string_offset = readWord(string_index * 2);
  u16 next_string_offset = readWord((string_index + 1) * 2);
  u16 limit = (next_string_offset / 8) + 1;

  u16 byte_index = string_offset / 8;
  u8 bit_index = string_offset % 8;

  while (byte_index < limit) {
    u16 decompressed_data = 0x00FE;

    while (!(decompressed_data & 0x8000)) {
      u8 byte = rom->readByte(rom_data_start + byte_index);

      u16 huffman_index = decompressed_data * 4;
      if (byte & (1 << bit_index)) {
        huffman_index += 2;
      }

      decompressed_data = rom->readWord(0xE8C9CA + huffman_index);

      if (bit_index == 7) {
        byte_index += 1;
        bit_index = 0;
      } else {
        bit_index += 1;
      }
    }
    pushByte(buffer, decompressed_data & 0xFF);
  }

  return buffer;
}

void Block::initialize() {
  data.clear();

  u24 start = address + 9;
  for (u24 i = 0; i < size; i++) {
    u8 byte = rom->readByte(start + i);
    pushByte(data, byte);
  }

  pushWord(data, rom->readWord(address + 1));
  pushAddress(data, start);
  pushByte(data, 0x00);
}

u8 Block::readByte(u16 index) const { return data[index]; }

u16 Block::readWord(u16 index) const {
  u8 lo = readByte(index);
  u8 hi = readByte(index + 1);
  return (hi << 8) | lo;
}

u24 Block::readAddress(u16 index) const {
  u16 lo = readWord(index);
  u8 hi = readByte(index + 2);
  return (hi << 16) | lo;
}

void Block::pushByte(vector<u8> &vec, u8 val) { vec.push_back(val); }

void Block::pushWord(vector<u8> &vec, u16 val) {
  pushByte(vec, val & 0xFF);
  pushByte(vec, val >> 8);
}

void Block::pushAddress(vector<u8> &vec, u24 val) {
  pushByte(vec, val & 0xFF);
  pushWord(vec, val >> 8);
}
