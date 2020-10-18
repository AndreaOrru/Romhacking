#!/usr/bin/env python3

text = [x for x in open("dict.txt").read().split("\n") if x]

table = [tuple(x.split("=", 1)) for x in open("smrpg.tbl").read().split("\n") if x]
table = {v: int(k, 16) for k, v in table}

bin_dict = bytearray()
bin_ptrs = bytearray()

ptr = 0
for word in text:
    bin_word = bytearray([table[x] for x in word])
    bin_dict.extend(bin_word)
    bin_dict.append(0x00)

    bin_ptrs.append(ptr & 0xFF)
    bin_ptrs.append(ptr >> 8)

    ptr += len(word) + 1

with open("dict.bin", "wb") as f:
    f.write(bin_dict)
with open("ptrs.bin", "wb") as f:
    f.write(bin_ptrs)
