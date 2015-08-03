#pragma once
#include <cstdint>

struct Flags
{
    bool n = 0;
    bool v = 0;
    bool m = 0;
    bool x = 0;
    bool d = 0;
    bool i = 0;
    bool z = 0;
    bool c = 0;

    bool e = 0;

    inline operator unsigned() const
    {
        return (n << 7) + (v << 6) + (m << 5) + (x << 4)
             + (d << 3) + (i << 2) + (z << 1) + (c << 0);
    }

    inline auto operator=(uint8_t data) -> unsigned
    {
        n = data & 0x80; v = data & 0x40; m = data & 0x20; x = data & 0x10;
        d = data & 0x08; i = data & 0x04; z = data & 0x02; c = data & 0x01;
        return data;
    }
};

struct Register
{
    union
    {
        uint16_t w = 0;
        struct { uint8_t l, h; };
    };

    inline operator unsigned() const { return w; }
    inline auto operator   = (unsigned i) -> unsigned { return w   = i; }
    inline auto operator  |= (unsigned i) -> unsigned { return w  |= i; }
    inline auto operator  ^= (unsigned i) -> unsigned { return w  ^= i; }
    inline auto operator  &= (unsigned i) -> unsigned { return w  &= i; }
    inline auto operator <<= (unsigned i) -> unsigned { return w <<= i; }
    inline auto operator >>= (unsigned i) -> unsigned { return w >>= i; }
    inline auto operator  += (unsigned i) -> unsigned { return w  += i; }
    inline auto operator  -= (unsigned i) -> unsigned { return w  -= i; }
    inline auto operator  *= (unsigned i) -> unsigned { return w  *= i; }
    inline auto operator  /= (unsigned i) -> unsigned { return w  /= i; }
    inline auto operator  %= (unsigned i) -> unsigned { return w  %= i; }
};

Register A, X, Y, S, D;
uint32_t B = 0;
Flags P;
uint8_t mem[0x100000];

inline uint8_t  rd_b(uint32_t i) { return mem[i]; }
inline uint16_t rd_w(uint32_t i) { return *((uint16_t*) &mem[i]); }
inline uint32_t rd_l(uint32_t i) { return *((uint32_t*) &mem[i]) & 0xFFFFFF; }
inline void wr_b(uint32_t i, uint8_t  v) { mem[i] = v; }
inline void wr_w(uint32_t i, uint16_t v) { *((uint16_t*) &mem[i]) = v; }

void ADC_imm_b(uint8_t v)
{
    int result;

    if (!P.d)
        result = A.l + v + P.c;
    else
    {
        result = (A.l & 0x0F) + (v & 0x0F) + (P.c << 0);
        if (result > 0x09) result += 0x06;
        P.c = result > 0x0F;
        result = (A.l & 0xF0) + (v & 0xF0) + (P.c << 4) + (result & 0x0F);
    }

    P.v = ~(A.l ^ v) & (A.l ^ result) & 0x80;
    if (P.d && result > 0x9F) result += 0x60;
    P.c = result > 0xFF;
    P.n = result & 0x80;
    P.z = (uint8_t)result == 0;

    A.l = result;
}
void ADC_b(uint32_t i) { ADC_imm_b(rd_b(i)); }

void ADC_imm_w(uint16_t v)
{
    int result;

    if (!P.d)
        result = A.w + v + P.c;
    else
    {
        result = (A.w & 0x000F) + (v & 0x000F) + (P.c <<  0);
        if (result > 0x0009) result += 0x0006;
        P.c = result > 0x000F;
        result = (A.w & 0x00F0) + (v & 0x00F0) + (P.c <<  4) + (result & 0x000F);
        if (result > 0x009F) result += 0x0060;
        P.c = result > 0x00FF;
        result = (A.w & 0x0F00) + (v & 0x0F00) + (P.c <<  8) + (result & 0x00FF);
        if (result > 0x09FF) result += 0x0600;
        P.c = result > 0x0FFF;
        result = (A.w & 0xF000) + (v & 0xF000) + (P.c << 12) + (result & 0x0FFF);
    }

    P.v = ~(A.w ^ v) & (A.w ^ result) & 0x8000;
    if (P.d && result > 0x9FFF) result += 0x6000;
    P.c = result > 0xFFFF;
    P.n = result & 0x8000;
    P.z = (uint16_t)result == 0;

    A.w = result;
}
void ADC_w(uint32_t i) { ADC_imm_w(rd_w(i)); }

void AND_imm_b(uint8_t v)
{
    A.l &= v;
    P.n = A.l & 0x80;
    P.z = A.l == 0;
}
void AND_b(uint32_t i) { AND_imm_b(rd_b(i)); }

void AND_imm_w(uint16_t v)
{
    A.w &= v;
    P.n = A.w & 0x8000;
    P.z = A.w == 0;
}
void AND_w(uint32_t i) { AND_imm_w(rd_w(i)); }

void BIT_imm_b(uint8_t v)
{
    P.n = v & 0x80;
    P.v = v & 0x40;
    P.z = (v & A.l) == 0;
}
void BIT_b(uint32_t i) { BIT_imm_b(rd_b(i)); }

void BIT_imm_w(uint16_t v)
{
    P.n = v & 0x8000;
    P.v = v & 0x4000;
    P.z = (v & A.w) == 0;
}
void BIT_w(uint32_t i) { BIT_imm_w(rd_w(i)); }

void CMP_imm_b(uint8_t v)
{
    int r = A.l - v;
    P.n = r & 0x80;
    P.z = (uint8_t)r == 0;
    P.c = r >= 0;
}
void CMP_b(uint32_t i) { CMP_imm_b(rd_b(i)); }

void CMP_imm_w(uint16_t v)
{
    int r = A.w - v;
    P.n = r & 0x8000;
    P.z = (uint16_t)r == 0;
    P.c = r >= 0;
}
void CMP_w(uint32_t i) { CMP_imm_w(rd_w(i)); }

void CPX_imm_b(uint8_t v)
{
    int r = X.l - v;
    P.n = r & 0x80;
    P.z = (uint8_t)r == 0;
    P.c = r >= 0;
}
void CPX_b(uint32_t i) { CPX_imm_b(rd_b(i)); }

void CPX_imm_w(uint16_t v)
{
    int r = X.w - v;
    P.n = r & 0x8000;
    P.z = (uint16_t)r == 0;
    P.c = r >= 0;
}
void CPX_w(uint32_t i) { CPX_imm_w(rd_w(i)); }

void CPY_imm_b(uint8_t v)
{
    int r = Y.l - v;
    P.n = r & 0x80;
    P.z = (uint8_t)r == 0;
    P.c = r >= 0;
}
void CPY_b(uint32_t i) { CPY_imm_b(rd_b(i)); }

void CPY_imm_w(uint16_t v)
{
    int r = Y.w - v;
    P.n = r & 0x8000;
    P.z = (uint16_t)r == 0;
    P.c = r >= 0;
}
void CPY_w(uint32_t i) { CPY_imm_w(rd_w(i)); }

void EOR_imm_b(uint8_t v)
{
    A.l ^= v;
    P.n = A.l & 0x80;
    P.z = A.l == 0;
}
void EOR_b(uint32_t i) { EOR_imm_b(rd_b(i)); }

void EOR_imm_w(uint16_t v)
{
    A.w ^= v;
    P.n = A.w & 0x8000;
    P.z = A.w == 0;
}
void EOR_w(uint32_t i) { EOR_imm_w(rd_w(i)); }

void LDA_imm_b(uint8_t v)
{
    A.l = v;
    P.n = A.l & 0x80;
    P.z = A.l == 0;
}
void LDA_b(uint32_t i) { LDA_imm_b(rd_b(i)); }

void LDA_imm_w(uint16_t v)
{
    A.w = v;
    P.n = A.w & 0x8000;
    P.z = A.w == 0;
}
void LDA_w(uint32_t i) { LDA_imm_w(rd_w(i)); }

void LDX_imm_b(uint8_t v)
{
    X.l = v;
    P.n = X.l & 0x80;
    P.z = X.l == 0;
}
void LDX_b(uint32_t i) { LDX_imm_b(rd_b(i)); }

void LDX_imm_w(uint16_t v)
{
    X.w = v;
    P.n = X.w & 0x8000;
    P.z = X.w == 0;
}
void LDX_w(uint32_t i) { LDX_imm_w(rd_w(i)); }

void LDY_imm_b(uint8_t v)
{
    Y.l = v;
    P.n = Y.l & 0x80;
    P.z = Y.l == 0;
}
void LDY_b(uint32_t i) { LDY_imm_b(rd_b(i)); }

void LDY_imm_w(uint16_t v)
{
    Y.w = v;
    P.n = Y.w & 0x8000;
    P.z = Y.w == 0;
}
void LDY_w(uint32_t i) { LDY_imm_w(rd_w(i)); }

void ORA_imm_b(uint8_t v)
{
    A.l |= v;
    P.n = A.l & 0x80;
    P.z = A.l == 0;
}
void ORA_b(uint32_t i) { ORA_imm_b(rd_b(i)); }

void ORA_imm_w(uint16_t v)
{
    A.w |= v;
    P.n = A.w & 0x8000;
    P.z = A.w == 0;
}
void ORA_w(uint32_t i) { ORA_imm_w(rd_w(i)); }

void SBC_imm_b(uint8_t v)
{
    int result;
    v ^= 0xFF;

    if (!P.d)
        result = A.l + v + P.c;
    else
    {
        result = (A.l & 0x0F) + (v & 0x0F) + (P.c << 0);
        if (result <= 0x0F) result -= 0x06;
        P.c = result > 0x0F;
        result = (A.l & 0xF0) + (v & 0xF0) + (P.c << 4) + (result & 0x0F);
    }

    P.v = ~(A.l ^ v) & (A.l ^ result) & 0x80;
    if (P.d && result <= 0xFF) result -= 0x60;
    P.c = result > 0xFF;
    P.n = result & 0x80;
    P.z = (uint8_t)result == 0;

    A.l = result;
}
void SBC_b(uint32_t i) { SBC_imm_b(rd_b(i)); }

void SBC_imm_w(uint16_t v)
{
    int result;
    v ^= 0xFFFF;

    if (!P.d)
        result = A.w + v + P.c;
    else
    {
        result = (A.w & 0x000F) + (v & 0x000F) + (P.c <<  0);
        if (result <= 0x000F) result -= 0x0006;
        P.c = result > 0x000F;
        result = (A.w & 0x00F0) + (v & 0x00F0) + (P.c <<  4) + (result & 0x000F);
        if (result <= 0x00FF) result -= 0x0060;
        P.c = result > 0x00FF;
        result = (A.w & 0x0F00) + (v & 0x0F00) + (P.c <<  8) + (result & 0x00FF);
        if (result <= 0x0FFF) result -= 0x0600;
        P.c = result > 0x0FFF;
        result = (A.w & 0xF000) + (v & 0xF000) + (P.c << 12) + (result & 0x0FFF);
    }

    P.v = ~(A.w ^ v) & (A.w ^ result) & 0x8000;
    if (P.d && result <= 0xFFFF) result -= 0x6000;
    P.c = result > 0xFFFF;
    P.n = result & 0x8000;
    P.z = (uint16_t)result == 0;

    A.w = result;
}
void SBC_w(uint32_t i) { SBC_imm_w(rd_w(i)); }

void INC_imm_b(uint8_t v)
{
    v++;
    P.n = v & 0x80;
    P.z = v == 0;
}
void INC_b(uint32_t i) { INC_imm_b(rd_b(i)); }

void INC_imm_w(uint16_t v)
{
    v++;
    P.n = v & 0x8000;
    P.z = v == 0;
}
void INC_w(uint32_t i) { INC_imm_w(rd_w(i)); }

void DEC_imm_b(uint8_t v)
{
    v--;
    P.n = v & 0x80;
    P.z = v == 0;
}
void DEC_b(uint32_t i) { DEC_imm_b(rd_b(i)); }

void DEC_imm_w(uint16_t v)
{
    v--;
    P.n = v & 0x8000;
    P.z = v == 0;
}
void DEC_w(uint32_t i) { DEC_imm_w(rd_w(i)); }

void ASL_imm_b(uint8_t v)
{
    P.c = v & 0x80;
    v <<= 1;
    P.n = v & 0x80;
    P.z = v == 0;
}
void ASL_b(uint32_t i) { ASL_imm_b(rd_b(i)); }

void ASL_imm_w(uint16_t v)
{
    P.c = v & 0x8000;
    v <<= 1;
    P.n = v & 0x8000;
    P.z = v == 0;
}
void ASL_w(uint32_t i) { ASL_imm_w(rd_w(i)); }

void LSR_imm_b(uint8_t v)
{
    P.c = v & 1;
    v >>= 1;
    P.n = v & 0x80;
    P.z = v == 0;
}
void LSR_b(uint32_t i) { LSR_imm_b(rd_b(i)); }

void LSR_imm_w(uint16_t v)
{
    P.c = v & 1;
    v >>= 1;
    P.n = v & 0x8000;
    P.z = v == 0;
}
void LSR_w(uint32_t i) { LSR_imm_w(rd_w(i)); }

void ROL_imm_b(uint8_t v)
{
    unsigned carry = (unsigned)P.c;
    P.c = v & 0x80;
    v = (v << 1) | carry;
    P.n = v & 0x80;
    P.z = v == 0;
}
void ROL_b(uint32_t i) { ROL_imm_b(rd_b(i)); }

void ROL_imm_w(uint16_t v)
{
    unsigned carry = (unsigned)P.c;
    P.c = v & 0x8000;
    v = (v << 1) | carry;
    P.n = v & 0x8000;
    P.z = v == 0;
}
void ROL_w(uint32_t i) { ROL_imm_w(rd_w(i)); }

void ROR_imm_b(uint8_t v)
{
    unsigned carry = (unsigned)P.c << 7;
    P.c = v & 1;
    v = carry | (v >> 1);
    P.n = v & 0x80;
    P.z = v == 0;
}
void ROR_b(uint32_t i) { ROR_imm_b(rd_b(i)); }

void ROR_imm_w(uint16_t v)
{
    unsigned carry = (unsigned)P.c << 15;
    P.c = v & 1;
    v = carry | (v >> 1);
    P.n = v & 0x8000;
    P.z = v == 0;
}
void ROR_w(uint32_t i) { ROR_imm_w(rd_w(i)); }

void TRB_imm_b(uint8_t v)
{
    P.z = (v & A.l) == 0;
    v &= ~A.l;
}
void TRB_b(uint32_t i) { TRB_imm_b(rd_b(i)); }

void TRB_imm_w(uint16_t v)
{
    P.z = (v & A.w) == 0;
    v &= ~A.w;
}
void TRB_w(uint32_t i) { TRB_imm_w(rd_w(i)); }

void TSB_imm_b(uint8_t v)
{
    P.z = (v & A.l) == 0;
    v |= A.l;
}
void TSB_b(uint32_t i) { TSB_imm_b(rd_b(i)); }

void TSB_imm_w(uint16_t v)
{
    P.z = (v & A.w) == 0;
    v |= A.w;
}
void TSB_w(uint32_t i) { TSB_imm_w(rd_w(i)); }

void NOP() {}

void XBA()
{
    A.l ^= A.h;
    A.h ^= A.l;
    A.l ^= A.h;
    P.n = (A.l & 0x80);
    P.z = (A.l == 0);
}

void STP()
{
    while (true);
}

void XCE()
{
    bool carry = P.c;
    P.c = P.e;
    P.e = carry;
    if (P.e)
    {
        P.m = 1;
        P.x = 1;
        X.h = 0x00;
        Y.h = 0x00;
        S.h = 0x01;
    }
}

void TCS()
{
    S.w = A.w;
}

void TSX_imm_b(uint8_t v)
{
    X.l = S.l;
    P.n = (X.l & 0x80);
    P.z = (X.l == 0);
}
void TSX_b(uint32_t i) { TSX_imm_b(rd_b(i)); }

void TSX_imm_w(uint16_t v)
{
    X.w = S.w;
    P.n = (X.w & 0x8000);
    P.z = (X.w == 0);
}
void TSX_w(uint32_t i) { TSX_imm_w(rd_w(i)); }

void TXS()
{
    S.w = X.w;
}

void PHD()
{
    wr_b(S.w--, D.h);
    wr_b(S.w--, D.l);
}

void PHB()
{
    wr_b(S.w--, B >> 16);
}

void PHK(uint8_t b)
{
    wr_b(S.w--, b);
}

void PHP()
{
    wr_b(S.w--, P);
}

void PLD()
{
    D.l = rd_b(++S.w);
    D.h = rd_b(++S.w);
    P.n = (D.w & 0x8000);
    P.z = (D.w == 0);
}

void PLB()
{
    B = rd_b(++S.w) << 16;
    P.n = (B & 0x800000);
    P.z = (B == 0);
}

void PLP()
{
    P = rd_b(++S.w);
    if (P.x)
    {
        X.h = 0x00;
        Y.h = 0x00;
    }
}

void PEA(uint16_t i)
{
    wr_b(S.w--, i >> 8);
    wr_b(S.w--, i & 0xFF);
}

void PEI(uint8_t i)
{
    wr_b(S.w--, rd_b(D + i + 1));
    wr_b(S.w--, rd_b(D + i));
}
