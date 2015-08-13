#include <algorithm>
#include <iterator>
#include <map>
#include "block.hpp"
#include "bytepair.hpp"
#include "huffman.hpp"
#include "rom.hpp"

using namespace std;


void Block::insert_blocks(vector<Block>& blocks, const char* outname)
{
    vector<vector<u8>> datas = Huffman::compress(blocks);

    FILE* out = fopen(outname, "wb");
    fwrite(ROM::get_rom(), 1, (size_t) ROM::get_size(), out);

    int diff = 0;
    for (size_t i = 0; i < blocks.size(); i++)
    {
        auto& b = blocks[i];
        size_t old_size = b.end - b.begin;

        b.begin += diff;
        b.end    = b.begin + datas[i].size();
        diff += datas[i].size() - old_size;

        for (auto index: b.indexes)
            if (index < 0x1000)
            {
                fseek(out, 0x3E2E00 + index*2, SEEK_SET);
                fputc( b.begin &       0xFF, out);
                fputc((b.begin >> 8) & 0xFF, out);

                fseek(out, 0x3E4E00 + index/2, SEEK_SET);
                fputc((ROM::rd(0x3E4E00 + index/2) & 0xF0) |
                      (((b.begin >> 16) - 0x38) & 0xF), out);
            }
            else
            {
                fseek(out, 0x4BF7C + 2*(index - 0x1000), SEEK_SET);
                fputc( b.begin &       0xFF, out);
                fputc((b.begin >> 8) & 0xFF, out);
            }

        fseek(out, b.begin, SEEK_SET);
        fwrite(datas[i].data(), 1, datas[i].size(), out);
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
    map<int,Block> blocks;

    auto insert = [&](int i, int addr)
    {
        auto it = blocks.find(addr);
        if (it != blocks.end())
            it->second.indexes.push_back(i);
        else
            blocks.emplace(addr, Block(i, addr));
    };

    for (int i = 0; i < 0x1000; i++)
    {
        u8    bank = ROM::rd(0xEA91 + (ROM::rd(0x3E4E00 + i/2) & 0xF));
        u16 offset = ROM::rd_w(0x3E2E00 + i*2);
        int   addr = ((bank << 16) | offset) - 0xC00000;

        if (i != 0x827 and i != 0x855)
            insert(i, addr);
    }
    for (int i = 0; i < 0x1A9; i++)
        insert(i + 0x1000, 0x3B0000 | ROM::rd_w(0x4BF7C + i*2));

    vector<Block> blocks_vec;
    transform(blocks.begin(), blocks.end(), back_inserter(blocks_vec), [](pair<const int,Block> &x) { return x.second; });

    for (auto it = blocks_vec.begin(); it != blocks_vec.end(); it++)
    {
        it->end  = (next(it) != blocks_vec.end()) ? next(it)->begin : 0x3C0000;
        it->data = Huffman::decompress(*it);
    }

    return blocks_vec;
}
