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

        for (auto& ptr: b.pointers)
        {
            if (ptr.first >= 0)
            {
                fseek(out, ptr.first, SEEK_SET);
                fputc((ROM::rd(ptr.first) & 0xF0) | (((b.begin >> 16) - 0x38) & 0xF), out);
            }
            fseek(out, ptr.second, SEEK_SET);
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
    map<int, Block> blocks;

    auto insert = [&](int bank_ptr, int offset_ptr) {
        u8 bank = (u8) ((bank_ptr < 0) ? 0x3B : ((ROM::rd(bank_ptr) & 0xF) + 0x38));
        u16 offset = ROM::rd_w(offset_ptr);
        int begin = (bank << 16) | offset;

        auto it = blocks.find(begin);
        if (it != blocks.end())
            it->second.pointers.emplace_back(bank_ptr, offset_ptr);
        else
            blocks.emplace(begin, Block(begin, bank_ptr, offset_ptr));
    };

    for (int i = 0; i < 0x1000; i++)
        if (i != 0x827 and i != 0x855)
            insert(0x3E4E00 + i / 2, 0x3E2E00 + i * 2);

    for (int i = 0; i < 0x36; i++)
    {
        int seq = 0x40000 + ROM::rd_w(0x4BF10 + i*2);
        while (ROM::rd(seq) != 0)
        {
            insert(-1, seq + 3);
            seq += 5;
        }
    }

    vector<Block> blocks_vec;
    transform(blocks.begin(), blocks.end(), back_inserter(blocks_vec), [](pair<const int,Block> &x) { return x.second; });

    for (auto it = blocks_vec.begin(); it != blocks_vec.end(); it++)
    {
        it->end  = (next(it) != blocks_vec.end()) ? next(it)->begin : 0x3C0000;
        it->data = Huffman::decompress(*it);
    }

    return blocks_vec;
}
