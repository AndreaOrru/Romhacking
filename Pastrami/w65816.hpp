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

inline  uint8_t& mem_b(uint32_t i) { return mem[i]; }
inline uint16_t& mem_w(uint32_t i) { return *((uint16_t*) &mem[i]); }
inline uint24_t& mem_l(uint32_t i) { return *((uint24_t*) &mem[i]); }

inline void ADC_imm_b(uint8_t v)
{
  int result;

  if (!P.d) {
    result = A.l + v + P.c;
  } else {
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
inline void ADC_b(uint32_t i) { ADC_imm_b(mem_b(i)); }

inline void ADC_imm_w(uint16_t v)
{
  int result;

  if (!P.d) {
    result = A.w + v + P.c;
  } else {
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
inline void ADC_w(uint32_t i) { ADC_imm_w(mem_w(i)); }

inline void AND_imm_b(uint8_t v)
{
  A.l &= v;
  P.n = A.l & 0x80;
  P.z = A.l == 0;
}
inline void AND_b(uint32_t i) { AND_imm_b(mem_b(i)); }

inline void AND_imm_w(uint16_t v)
{
  A.w &= v;
  P.n = A.w & 0x8000;
  P.z = A.w == 0;
}
inline void AND_w(uint32_t i) { AND_imm_w(mem_w(i)); }

inline void BIT_imm_b(uint8_t v)
{
  P.n = v & 0x80;
  P.v = v & 0x40;
  P.z = (v & A.l) == 0;
}
inline void BIT_b(uint32_t i) { BIT_imm_b(mem_b(i)); }

inline void BIT_imm_w(uint16_t v)
{
  P.n = v & 0x8000;
  P.v = v & 0x4000;
  P.z = (v & A.w) == 0;
}
inline void BIT_w(uint32_t i) { BIT_imm_w(mem_w(i)); }

inline void CMP_imm_b(uint8_t v)
{
  int r = A.l - v;
  P.n = r & 0x80;
  P.z = (uint8)r == 0;
  P.c = r >= 0;
}
inline void CMP_b(uint32_t i) { CMP_imm_b(mem_b(i)); }

inline void CMP_imm_w(uint16_t v)
{
  int r = A.w - v;
  P.n = r & 0x8000;
  P.z = (uint16)r == 0;
  P.c = r >= 0;
}
inline void CMP_w(uint32_t i) { CMP_imm_w(mem_w(i)); }

inline void CPX_imm_b(uint8_t v)
{
  int r = X.l - v;
  P.n = r & 0x80;
  P.z = (uint8)r == 0;
  P.c = r >= 0;
}
inline void CPX_b(uint32_t i) { CPX_imm_b(mem_b(i)); }

inline void CPX_imm_w(uint16_t v)
{
  int r = X.w - v;
  P.n = r & 0x8000;
  P.z = (uint16)r == 0;
  P.c = r >= 0;
}
inline void CPX_w(uint32_t i) { CPX_imm_w(mem_w(i)); }

inline void CPY_imm_b(uint8_t v)
{
  int r = Y.l - v;
  P.n = r & 0x80;
  P.z = (uint8)r == 0;
  P.c = r >= 0;
}
inline void CPY_b(uint32_t i) { CPY_imm_b(mem_b(i)); }

inline void CPY_imm_w(uint16_t v)
{
  int r = Y.w - v;
  P.n = r & 0x8000;
  P.z = (uint16)r == 0;
  P.c = r >= 0;
}
inline void CPY_w(uint32_t i) { CPY_imm_w(mem_w(i)); }

inline void EOR_imm_b(uint8_t v)
{
  A.l ^= v;
  P.n = A.l & 0x80;
  P.z = A.l == 0;
}
inline void EOR_b(uint32_t i) { EOR_imm_b(mem_b(i)); }

inline void EOR_imm_w(uint16_t v)
{
  A.w ^= v;
  P.n = A.w & 0x8000;
  P.z = A.w == 0;
}
inline void EOR_w(uint32_t i) { EOR_imm_w(mem_w(i)); }

inline void LDA_imm_b(uint8_t v)
{
  A.l = v;
  P.n = A.l & 0x80;
  P.z = A.l == 0;
}
inline void LDA_b(uint32_t i) { LDA_imm_b(mem_b(i)); }

inline void LDA_imm_w(uint16_t v)
{
  A.w = v;
  P.n = A.w & 0x8000;
  P.z = A.w == 0;
}
inline void LDA_w(uint32_t i) { LDA_imm_w(mem_w(i)); }

inline void LDX_imm_b(uint8_t v)
{
  X.l = v;
  P.n = X.l & 0x80;
  P.z = X.l == 0;
}
inline void LDX_b(uint32_t i) { LDX_imm_b(mem_b(i)); }

inline void LDX_imm_w(uint16_t v)
{
  X.w = v;
  P.n = X.w & 0x8000;
  P.z = X.w == 0;
}
inline void LDX_w(uint32_t i) { LDX_imm_w(mem_w(i)); }

inline void LDY_imm_b(uint8_t v)
{
  Y.l = v;
  P.n = Y.l & 0x80;
  P.z = Y.l == 0;
}
inline void LDY_b(uint32_t i) { LDY_imm_b(mem_b(i)); }

inline void LDY_imm_w(uint16_t v)
{
  Y.w = v;
  P.n = Y.w & 0x8000;
  P.z = Y.w == 0;
}
inline void LDY_w(uint32_t i) { LDY_imm_w(mem_w(i)); }

inline void ORA_imm_b(uint8_t v)
{
  A.l |= v;
  P.n = A.l & 0x80;
  P.z = A.l == 0;
}
inline void ORA_b(uint32_t i) { ORA_imm_b(mem_b(i)); }

inline void ORA_imm_w(uint16_t v)
{
  A.w |= v;
  P.n = A.w & 0x8000;
  P.z = A.w == 0;
}
inline void ORA_w(uint32_t i) { ORA_imm_w(mem_w(i)); }

inline void SBC_b(uint8_t& v)
{
  int result;
  v ^= 0xFF;

  if (!P.d) {
    result = A.l + v + P.c;
  } else {
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
inline void SBC_b(uint32_t i) { SBC_b(mem_b(i)); }

inline void SBC_w(uint16_t& v)
{
  int result;
  v ^= 0xFFFF;

  if (!P.d) {
    result = A.w + v + P.c;
  } else {
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
inline void SBC_w(uint32_t i) { SBC_w(mem_w(i)); }

inline void INC_b(uint8_t& v)
{
  v++;
  P.n = v & 0x80;
  P.z = v == 0;
}
inline void INC_b(uint32_t i) { INC_b(mem_b(i)); }

inline void INC_w(uint16_t& v)
{
  v++;
  P.n = v & 0x8000;
  P.z = v == 0;
}
inline void INC_w(uint32_t i) { INC_w(mem_w(i)); }

inline void DEC_b(uint8_t& v)
{
  v--;
  P.n = v & 0x80;
  P.z = v == 0;
}
inline void DEC_b(uint32_t i) { DEC_b(mem_b(i)); }

inline void DEC_w(uint16_t& v)
{
  v--;
  P.n = v & 0x8000;
  P.z = v == 0;
}
inline void DEC_w(uint32_t i) { DEC_w(mem_w(i)); }

inline void ASL_b(uint8_t& v)
{
  P.c = v & 0x80;
  v <<= 1;
  P.n = v & 0x80;
  P.z = v == 0;
}
inline void ASL_b(uint32_t i) { ASL_b(mem_b(i)); }

inline void ASL_w(uint16_t& v)
{
  P.c = v & 0x8000;
  v <<= 1;
  P.n = v & 0x8000;
  P.z = v == 0;
}
inline void ASL_w(uint32_t i) { ASL_w(mem_w(i)); }

inline void LSR_b(uint8_t& v)
{
  P.c = v & 1;
  v >>= 1;
  P.n = v & 0x80;
  P.z = v == 0;
}
inline void LSR_b(uint32_t i) { LSR_b(mem_b(i)); }

inline void LSR_w(uint16_t& v)
{
  P.c = v & 1;
  v >>= 1;
  P.n = v & 0x8000;
  P.z = v == 0;
}
inline void LSR_w(uint32_t i) { LSR_w(mem_w(i)); }

inline void ROL_b(uint8_t& v)
{
  unsigned carry = (unsigned)P.c;
  P.c = v & 0x80;
  v = (v << 1) | carry;
  P.n = v & 0x80;
  P.z = v == 0;
}
inline void ROL_b(uint32_t i) { ROL_b(mem_b(i)); }

inline void ROL_w(uint16_t& v)
{
  unsigned carry = (unsigned)P.c;
  P.c = v & 0x8000;
  v = (v << 1) | carry;
  P.n = v & 0x8000;
  P.z = v == 0;
}
inline void ROL_w(uint32_t i) { ROL_w(mem_w(i)); }

inline void ROR_b(uint8_t& v)
{
  unsigned carry = (unsigned)P.c << 7;
  P.c = v & 1;
  v = carry | (v >> 1);
  P.n = v & 0x80;
  P.z = v == 0;
}
inline void ROR_b(uint32_t i) { ROR_b(mem_b(i)); }

inline void ROR_w(uint16_t& v)
{
  unsigned carry = (unsigned)P.c << 15;
  P.c = v & 1;
  v = carry | (v >> 1);
  P.n = v & 0x8000;
  P.z = v == 0;
}
inline void ROR_w(uint32_t i) { ROR_w(mem_w(i)); }

inline void TRB_b(uint8_t& v)
{
  P.z = (v & A.l) == 0;
  v &= ~A.l;
}
inline void TRB_b(uint32_t i) { TRB_b(mem_b(i)); }

inline void TRB_w(uint16_t& v)
{
  P.z = (v & A.w) == 0;
  v &= ~A.w;
}
inline void TRB_w(uint32_t i) { TRB_w(mem_w(i)); }

inline void TSB_b(uint8_t& v)
{
  P.z = (v & A.l) == 0;
  v |= A.l;
}
inline void TSB_b(uint32_t i) { TSB_b(mem_b(i)); }

inline void TSB_w(uint16_t& v)
{
  P.z = (v & A.w) == 0;
  v |= A.w;
}
inline void TSB_w(uint32_t i) { TSB_w(mem_w(i)); }
