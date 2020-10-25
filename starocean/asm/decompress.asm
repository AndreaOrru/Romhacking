;;;;
;; Decompress a string of dialogue.
;;
;; IN:       A = Dialogue string index * 2.
;;
;; OUT:      A = Offset of decompressed string inside buffer.
;;
;; DISCARDS: X
;;
org !decompress_addr
DecompressText:
  ;; Preserve value of Y.
  phy
  ;; Preserve RAM values.
  pei ($00)
  pei ($02)
  ;; Change to a new stack and save previous one.
  tsx
  txy
  ldx #$1CE0
  txs
  phy

  ;; X = Offset in bits of the dialogue data inside block content.
  tay
  lda $B873,y
  sec
  sbc $B873
  tax
  ;; $00..$02 = Address of the block content in ROM.
  lda $B873
  lsr
  lsr
  lsr
  sta $00
  inc
  inc
  tay
  lda $B873,y
  clc
  adc $00
  sta $00
  iny
  iny
  lda $B873,y
  sta $02
  ;; Y = Offset of the decompression buffer.
  iny
  iny
  sty $03D5                     ; $03D5 = beginning of decompression buffer.

.loop:
  jsr FetchSymbol
  jsr ExpandBytepair
  cmp #$FFFF
  bne .loop

.done:
  ;; Restore stack.
  ply
  tyx
  txs
  ;; Restore RAM values.
  pla
  sta $02
  pla
  sta $00
  ;; Restore value of Y.
  ply

  ;; Return offset of decompressed string.
  lda $03D5
  rtl


;;;;
;; Expand bytepair value.
;;
;; IN:   A = Bytepair value.
;;       Y = Index inside decompression buffer.
;;
;; OUT:  A = Value of the last expanded symbol.
;;
ExpandBytepair:
  phx

  cmp #$0100
  beq .returnFFFF               ; (A == 0x100) ==> 0xFFFF
  bcc .returnChar               ; (A  < 0x100) ==> A

  ;; Expand bytepair.
  sbc #$0101
  asl
  asl
  tax

  lda $!DICTIONARY,x
  jsr ExpandBytepair

  lda $!DICTIONARY+2,x
  jsr ExpandBytepair

  bra .return

.returnFFFF:
  lda #$FFFF
.returnChar:
  sta $B873,y
  iny
.return:
  plx
  rts


;;;;
;; Increment X by 12.
;;
;; IN:       X = Value.
;;
;; OUT:      X = Value incremented by 12.
;;
;; DISCARDS: A
;;
macro inx12()
  txa
  clc
  adc #$000C
  tax
endmacro


;;;;
;; Fetch a 12-bit symbol.
;;
;; IN:       X = Symbol offset in bits.
;;
;; OUT:      A = Value of the symbol.
;;
FetchSymbol:
  phy

  ;; Y = (Bits / 8) = Byte index.
  txa
  lsr
  lsr
  lsr
  tay
  ;; ((Bits % 8) == 0) ==> Shift.
  txa
  and #$0007
  beq .shift

.noshift:
  %inx12()
  lda [$00],y
  xba
  and #$0FFF
  bra .return

.shift:
  %inx12()
  lda [$00],y
  xba
  lsr
  lsr
  lsr
  lsr

.return:
  ply
  rts


;; Bitmasks had to be moved from the original position.
;; Make sure references to it are correct.
bitmasks:
  db $01
  db $02
  db $04
  db $08
  db $10
  db $20
  db $40
  db $80
