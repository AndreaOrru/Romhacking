#include <iostream>

typedef unsigned char bool8;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

#include "SDD1emu.cpp"

int main(void) {

  uint8 in_buf[0x4000];
  uint8 out_buf[0x2000];
  SDD1emu SDD1;


  uint32 i, j;
  for (i=0; (j=std::cin.get())!=EOF; i++) 
    in_buf[i] = char(j);
  for (; i<0x4000; i++) in_buf[i]=0;

  SDD1.decompress(in_buf, 0x2000, out_buf);

  for (i=0; i < 0x2000; i++) std::cout << out_buf[i];

  return 0;

}
