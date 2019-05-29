#include "rom.hpp"
#include <cassert>

int main(int argc, char *argv[]) {
  assert(argc >= 2);

  char *rom_path = argv[1];
  auto rom = ROM(rom_path);

  return 0;
}
