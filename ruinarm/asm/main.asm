architecture wdc65816
include "instructions.asm"

output "../rom/ruinarm-en.sfc", create
insert "../rom/ruinarm-jp.sfc"
tracker enable

// LoROM / FastROM.
macro seek(variable offset) {
  origin ((offset & $7F0000) >> 1) | (offset & $7FFF)
  base offset | $800000
}

seek($80EAE8)
include "dialogue.asm"
include "font.asm"

if pc() > $810000 {
  error "bank $80 exhausted"
}

seek($BB90B8)
db $01,$fc,$af,$00,$bc,$00,$b3,$00,$b5,00,$ab,$00,$00,$f4,$46,$f4,$c1,$fa,$00,$f6
db $ba,$00,$d9,$00,$d4,$00,$d4,$00,$c5,$00,$00,$00,$ca,$00,$d3,$00,$d6,$00,$d8,$00,$c9,$00,$00,$f6
db $ba,$00,$d9,$00,$d4,$00,$d4,$00,$c5,$00,$00,$00,$ca,$00,$d3,$00,$d6,$00,$d8,$00,$c9,$00,$00,$f6
db $ba,$00,$d9,$00,$d4,$00,$d4,$00,$c5,$00,$00,$00,$ca,$00,$d3,$00,$d6,$00,$d8,$00,$c9,$00,$00,$00
db $ba,$00,$d9,$00,$d4,$00,$d4,$00,$c5,$00,$00,$00,$ca,$00,$d3,$00,$d6,$00,$d8,$00,$c9,$00
db $00,$f7
