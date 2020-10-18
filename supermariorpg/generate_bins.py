#!/usr/bin/env python3

text = open("dict.txt").read().split("\n")

bin_dict = bytearray()
bin_ptrs = bytearray()

ptr = 0
for word in text:
    bin_dict.extend(word.encode("ascii"))
    bin_dict.append(0x00)

    bin_ptrs.append(ptr & 0xFF)
    bin_ptrs.append(ptr >> 8)

    ptr += len(word) + 1

with open("dict.bin", "wb") as f:
    f.write(bin_dict)
with open("ptrs.bin", "wb") as f:
    f.write(bin_ptrs)
