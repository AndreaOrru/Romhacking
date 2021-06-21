#include <iostream>

typedef unsigned char bool8;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

#include "SDD1comp.cpp"

int main(void) {

  uint8 in_buf[0x2000];
  uint32 out_len;
  uint8 out_buf[0x3000];
  SDD1comp SDD1encoder;


  uint32 i, j;
  for (i=0; (j=std::cin.get())!=EOF; i++) 
    in_buf[i] = char(j);
  for (; i<0x2000; i++) in_buf[i]=0;

  SDD1encoder.compress(0x2000, in_buf, &out_len, out_buf);

  for (i=0; i < out_len; i++) std::cout << out_buf[i];

  return 0;

}
