namespace font {
  enqueue pc
  seek($9F8000); insert "../gfx/f8000_font_vwf.bin"
  dequeue pc

  widthTable: {
    db $09  // 0
    db $04  // 1
    db $09  // 2
    db $08  // 3
    db $09  // 4
    db $08  // 5
    db $08  // 6
    db $09  // 7
    db $08  // 8
    db $08  // 9

    db $0B  // A
    db $07  // B
    db $0A  // C
    db $09  // D
    db $07  // E
    db $07  // F
    db $0B  // G
    db $09  // H
    db $03  // I
    db $05  // J
    db $0A  // K
    db $07  // L
    db $0F  // M
    db $0C  // N
    db $0D  // O
    db $07  // P
    db $0D  // Q
    db $08  // R
    db $07  // S
    db $0B  // T
    db $0A  // U
    db $0D  // V
    db $0F  // W
    db $0B  // X
    db $09  // Y
    db $0A  // Z

    db $08  // a
    db $07  // b
    db $07  // c
    db $07  // d
    db $07  // e
    db $06  // f
    db $08  // g
    db $07  // h
    db $03  // i
    db $04  // j
    db $07  // k
    db $03  // l
    db $0B  // m
    db $07  // n
    db $08  // o
    db $07  // p
    db $07  // q
    db $06  // r
    db $06  // s
    db $06  // t
    db $07  // u
    db $09  // v
    db $0B  // w
    db $09  // x
    db $09  // y
    db $07  // z
  }
}
