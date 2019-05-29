#include "block.hpp"
#include "rom.hpp"
#include <cassert>
#include <fmt/core.h>

int main(int argc, char *argv[]) {
  assert(argc >= 2);

  char *rom_path = argv[1];
  auto rom = ROM(rom_path);

  // auto blocks = rom.blocks();
  // for (Block &block : blocks) {
  //   fmt::print("block {:02X}, address {:06X}, size {:04X}\n", block.index,
  //              block.address, block.size);
  // }

  auto block = Block(&rom, 0x0b);
  auto data = block.decompressString(0x24);
  for (auto c : data) {
    fmt::print("{:02X} ", c);
  }
  fmt::print("\n");

  return 0;
}
