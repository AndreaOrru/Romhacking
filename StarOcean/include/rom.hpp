#pragma once
#include "types.hpp"

class ROM {
public:
  ROM(const char *path);
  ~ROM();

  u8 readByte(u24 address) const;
  u16 readWord(u24 address) const;
  u24 readAddress(u24 address) const;

private:
  u8 *data;

  u24 translateAddress(u24 address) const;
};
