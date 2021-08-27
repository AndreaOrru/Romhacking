#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

typedef unsigned char bool8;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

#include "comp/SDD1comp.cpp"
#include "decomp/SDD1emu.cpp"

using namespace std;

const int IN_BUF_SIZE = 0x8000;
const int OUT_BUF_SIZE = IN_BUF_SIZE * 4;

vector<uint8> readBinaryFile(const string &path) {
  FILE *file = fopen(path.c_str(), "rb");

  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  fseek(file, 0, SEEK_SET);

  vector<uint8> buffer(size);
  fread(buffer.data(), size, 1, file);

  fclose(file);
  return buffer;
}

int decompress(vector<uint8> &rom, int address, int size, char *out_filename) {
  uint8 in_buf[IN_BUF_SIZE];
  uint8 out_buf[OUT_BUF_SIZE];
  SDD1emu SDD1;

  int i = 0;
  for (; i < size; i++) {
    in_buf[i] = rom[address + i];
  }
  for (; i < IN_BUF_SIZE; i++) {
    in_buf[i] = 0;
  }

  int comp_size = SDD1.decompress(in_buf, size, out_buf);

  FILE *out_file = fopen(out_filename, "wb");
  for (i = 0; i < size; i++) {
    fputc(out_buf[i], out_file);
  }
  fclose(out_file);

  return comp_size;
}

int compress(char *rom_filename, int address, int max_size,
             vector<uint8> &data) {
  SDD1comp SDD1encoder;
  uint8 out_buf[OUT_BUF_SIZE];
  uint32 out_len;

  SDD1encoder.compress(data.size(), data.data(), &out_len, out_buf);

  if (out_len > max_size) {
    fprintf(stderr, "Maximum size exceeded: 0x%X > 0x%X\n", out_len, max_size);
    return -1;
  }

  FILE *rom_file = fopen(rom_filename, "wb");
  fseek(rom_file, address, SEEK_SET);

  for (int i = 0; i < out_len; i++) {
    fputc(out_buf[i], rom_file);
  }
  fclose(rom_file);

  return out_len;
}

int main(int argc, char **argv) {
  string command = string(argv[1]);

  if (command == "-d") {
    auto rom = readBinaryFile(string(argv[2]));
    int address = std::stoi(string(argv[3]), nullptr, 16);
    int size = std::stoi(string(argv[4]), nullptr, 16);
    char *out_filename = argv[5];

    int comp_size = decompress(rom, address, size, out_filename);
    printf("Compressed size: 0x%X bytes\n", comp_size);
  } else if (command == "-c") {
    char *rom_filename = argv[2];
    int address = std::stoi(string(argv[3]), nullptr, 16);
    int max_size = std::stoi(string(argv[4]), nullptr, 16);
    auto data = readBinaryFile(string(argv[5]));

    int comp_size = compress(rom_filename, address, max_size, data);
    if (comp_size >= 0) {
      printf("Compressed size: 0x%X bytes\n", comp_size);
    } else {
      return -1;
    }
  } else {
    fprintf(stderr, "Unknown command\n");
    return -1;
  }

  return 0;
}
