#pragma once
#include "types.hpp"
#include <string>
#include <vector>

class Sentence {
public:
  std::vector<u8> data;

  Sentence(std::vector<u8> &&data);
  std::string format() const;
};
