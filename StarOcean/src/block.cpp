#include "block.hpp"
#include "rom.hpp"
#include "sentence.hpp"
#include <cassert>

using namespace std;

Block::Block(const ROM *rom, u8 index) : index(index), rom(rom) {
  address = rom->readAddress(0xE40000 + (index * 3));
  size = rom->readWord(address + 9) / 8;
  type = {rom->readByte(address)};
}

vector<Sentence> Block::extract() {
  assert(type == TEXT);
  initialize();

  vector<Sentence> sentences;
  for (int i = 0; i < (size / 2) - 1; i++) {
    sentences.emplace_back(decompressString(i));
  }
  return sentences;
}

vector<u8> Block::decompressString(u8 string_index) const {
  u24 rom_data_start = readAddress(size + 2);
  u16 string_offset = readWord(string_index * 2);
  u16 next_string_offset = readWord((string_index + 1) * 2);
  u16 string_limit = (next_string_offset / 8) + 1;

  auto first_step = huffman(rom_data_start, string_offset, string_limit);
  auto second_step = decode(first_step);

  return second_step;
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

void Block::specialPush(vector<u8> &vec, u8 val) {
  pushByte(vec, val);
  if (val >= 0x80) {
    pushByte(vec, 0x01);
  }
}

vector<u8> Block::huffman(u24 data_start, u16 offset, u16 limit) const {
  vector<u8> buffer;

  u16 byte_index = offset / 8;
  u8 bit_index = offset % 8;

  while (byte_index < limit) {
    u16 decompressed_data = 0x00FE;

    while (!(decompressed_data & 0x8000)) {
      u8 byte = rom->readByte(data_start + byte_index);

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

vector<u8> Block::decode(const vector<u8> &input) const {
  vector<u8> output;

  bool done;
  u8 byte;
  u16 index, index2, type_index, word, fragment_start;

  int i = 0;
  while (true) {
    u8 byte_type = rom->readByte(0xE8F9CE + input[i]);
    switch (byte_type) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 7:
      specialPush(output, input[i++]);
      break;

    case 5:
      pushWord(output, 0xFFFF);
      return output;

    case 6:
      pushByte(output, input[i++]);
    case 9:
      pushByte(output, input[i++]);
    case 8:
      pushByte(output, input[i++]);
      pushByte(output, input[i++]);
      break;

    case 0x10 ... 0x4F:
      byte = byte_type - 0x10;

      index = input[i + 1];
      index += (byte_type & 0x0F) << 8;
      index += 0x6E;
      index *= 2;
      index = rom->readWord(0xFF503E + index);
      i += 2;

      fragment_start = output.size();
      done = false;

      while (true) {
        index2 = rom->readByte(0xE8CDC6 + index) * 2;
        index++;
        if (index2 == 0) {
          if (byte & 0x20) {
            output[fragment_start] -= 0x1A;
            assert(output[fragment_start + 1] != 0);
          }
          if (byte & 0x10) {
            specialPush(output, 0x44);
          }
          done = true;
          break;
        } else {
          word = rom->readWord(0xE8F7CE + index2);
          specialPush(output, word & 0xFF);

          word >>= 8;
          if (word == 5) {
            if (byte & 0x20) {
              output[fragment_start] -= 0x1A;
              assert(output[fragment_start + 1] != 0);
            }
            if (byte & 0x10) {
              specialPush(output, 0x44);
            }
            done = true;
            break;
          } else if (word != 1) {
            specialPush(output, word);
          }
        }
      }
      if (done) {
        continue;
      }
      break;

    case 0x50 ... 0xFF:
      type_index = (byte_type - 0x50) * 2;
      index = rom->readWord(0xFF503E + type_index);

      while (true) {
        index2 = rom->readByte(0xE8CDC6 + index) * 2;
        index++;
        if (index2 == 0) {
          break;
        }

        word = rom->readWord(0xE8F7CE + index2);
        specialPush(output, word & 0xFF);

        word >>= 8;
        if (word == 5) {
          break;
        } else if (word != 1) {
          specialPush(output, word);
        }
      }
      i++;
      break;

    default:
      assert(false);
      break;
    }
  }

  return output;
}
