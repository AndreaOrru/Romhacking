#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

typedef unsigned char bool8;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

#include "decomp/SDD1emu.cpp"

using namespace std;

const int IN_BUF_SIZE = 0x8000;
const int OUT_BUF_SIZE = IN_BUF_SIZE * 4;

vector<char> readBinaryFile(const string &path) {
  FILE *file = fopen(path.c_str(), "rb");

  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  fseek(file, 0, SEEK_SET);

  vector<char> buffer(size);
  fread(buffer.data(), size, 1, file);

  fclose(file);
  return buffer;
}

int main(int argc, char **argv) {
  uint8 in_buf[IN_BUF_SIZE];
  uint8 out_buf[OUT_BUF_SIZE];
  SDD1emu SDD1;

  auto rom = readBinaryFile(string(argv[1]));
  int address = std::stoi(string(argv[2]), nullptr, 16);
  int size = std::stoi(string(argv[3]), nullptr, 16);
  char *out_filename = argv[4];

  int i = 0;
  for (; i < size; i++) {
    in_buf[i] = rom[address + i];
  }
  for (; i < IN_BUF_SIZE; i++) {
    in_buf[i] = 0;
  }

  SDD1.decompress(in_buf, size, out_buf);

  FILE *out_file = fopen(out_filename, "wb");
  for (i = 0; i < size; i++) {
    fputc(out_buf[i], out_file);
  }
  fclose(out_file);

  return 0;
}
