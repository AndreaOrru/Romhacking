#include "sentence.hpp"
#include <fmt/core.h>

using namespace std;

Sentence::Sentence(vector<u8> &&data) : data(data) {}

string Sentence::format() const {
  string output;

  for (auto c : data) {
    switch (c) {
    case 0x10 ... 0x29:
      // 'A' - 'Z'
      output += (c + 0x31);
      break;

    case 0x2A ... 0x43:
      // 'a' - 'z'
      output += (c + 0x37);
      break;

    case 0x44:
      output += ' ';
      break;
    case 0x46:
      output += '!';
      break;
    case 0x47:
      output += '?';
      break;
    case 0x55:
      output += ',';
      break;
    case 0x56:
      output += '.';
      break;
    case 0x57:
      output += ':';
      break;
    case 0x5A:
      output += '\'';
      break;

    default:
      output += fmt::format("<{:02X}>", c);
      break;
    }
  }

  return output;
}
