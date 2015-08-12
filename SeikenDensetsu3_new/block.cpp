#include <algorithm>
#include "bytepair.hpp"
#include "huffman.hpp"
#include "rom.hpp"
#include "block.hpp"


#include <iostream>

using namespace std;


void Block::insert_blocks(vector<Block>& blocks, const char* outname)
{
    vector<vector<u16>> datas = Huffman::compress(blocks);

    FILE* out = fopen(outname, "wb");
    fwrite(ROM::get_rom(), 1, ROM::get_size(), out);

    int diff = 0;
    for (size_t i = 0; i < blocks.size(); i++)
    {
        auto& b = blocks[i];
        size_t old_size = b.end - b.begin;

        b.begin += diff;
        b.end    = b.begin + 2*datas[i].size();

        diff += 2*datas[i].size() - old_size;

        if (b.index < 0x1000)
        {
            fseek(out, 0x3E2E00 + b.index*2, SEEK_SET);
            fprintf(out, "%hu", (u16)(b.begin & 0xFFFF));
            fseek(out, 0x3E4E00 + b.index/2, SEEK_SET);
            fputc((ROM::rd(0x3E4E00 + b.index/2) & 0xF0) |
                  (((b.begin >> 16) - 0x38) & 0xF), out);
        }
        else
        {
            fseek(out, 0x4BF7C + (b.index - 0x1000)*2, SEEK_SET);
            fprintf(out, "%hu", (u16)(b.begin & 0xFFFF));
        }

        fseek(out, b.begin, SEEK_SET);
        fwrite(b.data.data(), 1, b.data.size(), out);
    }

    auto& dict = BytePair::get_dict();
    auto& tree = Huffman::get_encoded_tree();

    fseek(out, 0x3E6604, SEEK_SET);
    fwrite(dict.data(), 4, dict.size(), out);
    fwrite(tree.data(), 2, tree.size(), out);

    fclose(out);
}

vector<Block> Block::extract_blocks()
{
    vector<Block> blocks;

    for (int i = 0; i < 0x1000; i++)
    {
        u8    bank = ROM::rd(0xEA91 + (ROM::rd(0x3E4E00 + i/2) & 0xF));
        u16 offset = ROM::rd_w(0x3E2E00 + i*2);
        u32   addr = ((bank << 16) | offset) - 0xC00000;
        if (i != 0x827 and i != 0x855)
            blocks.push_back(Block(i, addr));
    }
    for (int i = 0; i < 0x1A9; i++)
    {
        u16 offset = ROM::rd_w(0x4BF7C + i*2);
        u32   addr = 0x3B0000 | offset;
        blocks.push_back(Block(i + 0x1000, addr));
    }
    blocks.push_back(Block(blocks.size(), 0x3C0000));
    sort(blocks.begin(), blocks.end());

    for (size_t i = 0; i < blocks.size() - 1; i++)
    {
        blocks[i].end  = blocks[i+1].begin;
        blocks[i].data = Huffman::decompress(blocks[i]);
    }
    blocks.pop_back();

    return blocks;
}