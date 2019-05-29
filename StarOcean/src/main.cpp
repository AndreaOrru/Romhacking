#include "rom.hpp"
#include <cassert>
#include <fmt/core.h>

int main(int argc, char *argv[]) {
  assert(argc >= 2);

  char *rom_path = argv[1];
  auto rom = ROM(rom_path);

  auto blocks = rom.blocks();
  for (auto block : blocks) {
    fmt::print("{:06X}\n", block);
  }

  return 0;
}
