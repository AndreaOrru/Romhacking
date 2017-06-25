#ifndef ROM_HPP
#define ROM_HPP

#include <string>
#include "types.hpp"


namespace ROM
{

void open(const std::string file_name);  // Open the ROM.
void save();                             // Save the ROM.

u8   readByte(int address);              // Read a byte from the given (PC) address.
u16  readWord(int address);              // Read a word from the given (PC) address.

void writeByte(int address, u8 byte);    // Write a word to the given (PC) address.
void writeWord(int address, u16 word);   // Write a word to the given (PC) address.

}

#endif
