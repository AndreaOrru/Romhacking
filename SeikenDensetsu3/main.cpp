#include <cstdio>
#include <iostream>
#include <map>
#include "extractor.hpp"
#include "huffman.hpp"

using namespace std;

int main(int argc, char* argv[])
{
    auto fname = argv[1];

    auto f = fopen(fname, "rb");
    fseek(f, 0, SEEK_END);
    auto fsize = ftell(f);
    rewind(f);

    auto rom = new uint8_t[fsize];
    fread(rom, 1, fsize, f);
    fclose(f);

    map<uint32_t,uint16_t> roomsMap;
    for (int i = 0; i < 0x1000; i++)
    {
        uint16_t offset = ((uint16_t*) &rom[0x3E2E00])[i];
        uint8_t  bank   = rom[0xEA91 + (rom[0x3E4E00 + i/2] & 0xF)];
        uint32_t addr   = ((bank << 16) + offset) - 0xC00000;

        if (addr < 0x3BD079)
            roomsMap[addr] = i;
    }
    roomsMap.erase(0x390055);
    roomsMap.erase(0x394C2D);

    vector< pair<uint32_t,uint16_t> > rooms(roomsMap.begin(), roomsMap.end());
    uint16_t* meta = (uint16_t*) (rom + 0x3E6600);

    for (int i = 0; i < rooms.size() - 1; i++)
    {
        uint16_t* data = (uint16_t*) (rom + rooms[i].first);
        uint32_t  size = rooms[i+1].first - rooms[i].first;

        auto out = decompress(data, size, meta);
        auto sentences = extract(out);

        if (!sentences->empty())
        {
            printf("[ Script %X ]\n", rooms[i].second);

            for (auto it: *sentences)
                cout << it.second << endl;
            cout << endl;
        }
    }

    return 0;
}
