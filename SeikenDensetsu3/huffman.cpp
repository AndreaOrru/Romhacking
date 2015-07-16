#include <cctype>
#include <cstdio>
#include <string>

using namespace std;

uint16_t xba(uint16_t v)
{
    uint8_t l =  v & 0x00FF;
    uint8_t h = (v & 0xFF00) >> 8;

    return (l << 8) | h;
}

uint16_t read16(uint8_t* rom, uint32_t addr)
{
    return *((uint16_t*) &rom[addr]);
}

string mte(uint8_t* rom, uint16_t v)
{
    if (v <= 0x100)
    {
        if (isprint((char) v))
            return string(1, (char) v);
        else
            return "[" + to_string(v) + "]";
    }

    v -= 0x101;

    string s1 = mte(rom, read16(rom, 0x3E6604 + v*4));
    string s2 = mte(rom, read16(rom, 0x3E6606 + v*4));

    return s1 + s2;
}

int main(int argc, char* argv[])
{
    auto fname = argv[1];
    auto room  = stoi(string(argv[2]));
    auto words = stoi(string(argv[3]));

    auto f = fopen(fname, "rb");
    fseek(f, 0, SEEK_END);
    auto size = ftell(f);
    rewind(f);

    auto rom = new uint8_t[size];
    fread(rom, 1, size, f);
    fclose(f);

    uint16_t offset = read16(rom, 0x3E2E00 + room*2);
    uint8_t  bank   = rom[0xEA91 + (rom[0x3E4E00 + room/2] & 0xF)];
    uint32_t addr   = ((bank << 16) + offset) - 0xC00000;
    printf("%x\n\n", addr, bank, offset);

    // Algorithm:
    uint16_t bits;
    uint8_t  n = 1;

    for (int i = 0; i < words; i++)
    {
        uint16_t y = read16(rom, 0x3E6600);

        while (true)
        {
            n--;
            if (n == 0)
            {
                bits = xba(read16(rom, addr));
                addr += 2;
                n = 16;
            }

            bool b = bits & 0x8000;
            bits <<= 1;

            if (b)
                y += 2;

            y = read16(rom, 0x3E6600 + y);
            if (y & 0x8000)
                break;
        }

        y &= 0x7FFF;

        printf("%s", mte(rom, y).c_str());
    }

    printf("\n");

    return 0;
}
