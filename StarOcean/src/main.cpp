#include "block.hpp"
#include "rom.hpp"
#include "sentence.hpp"
#include <cassert>
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
  assert(argc >= 2);

  char *rom_path = argv[1];
  auto rom = ROM(rom_path);

  for (Block &block : rom.blocks()) {
    if (block.type == TEXT) {
      for (auto &sentence : block.extract()) {
        cout << sentence.format() << endl << endl;
      }
    }
  }

  return 0;
}
