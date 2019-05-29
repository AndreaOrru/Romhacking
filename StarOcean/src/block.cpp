#include "block.hpp"
#include "rom.hpp"

Block::Block(const ROM *rom, u8 index, u24 address)
    : index(index), address(address), rom(rom) {}

u24 Block::size() const { return rom->readWord(address + 9) / 8; }
