#include "block.hpp"
#include "rom.hpp"
#include <cassert>
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
  assert(argc >= 2);

  char *rom_path = argv[1];
  auto rom = ROM(rom_path);

  auto blocks = rom.blocks();
  for (Block &block : blocks) {
    if (block.type == 0x85) {
      for (int i = 0; i < block.size / 2 - 1; i++) {
        auto data = block.decompressString(i);
        for (auto c : data) {
          cout << c;
        }
      }
    }
  }

  return 0;
}
