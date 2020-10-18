arch 65816
sa1rom

;; Original MTE routine.
org $C060EF
  ;; $C060EF  plb
  ;; $C060F0  sty $021A
  jml extended_mte


;; MTE tables.
org $B5ED60
mte_pointers:
  incbin "ptrs.bin"
;; Space for 256 entries.

org $B5EF80
mtes:
  incbin "dict.bin"
;; Space for ~4k of text.


org $FF9C40
extended_mte:
  ;; $26 = $13 * 2 = Special byte for extended MTE.
  cmp #$26
  bne .no_extended

.extended:
  ;; Load second byte.
  lda $0000,y
  iny
  ;; Original incipit of the function.
  plb
  sty $021A

  ;; X = Address of the MTE pointer in the table.
  rep #$20
  and #$00FF
  asl
  tax

  ;; Y =  Address of the MTE content.
  lda $B5ED60,x
  clc
  adc #$EF80
  sta $021C                     ; $021C = Address of MTE content.
  tay                           ; Y = Address of MTE content.

  sep #$20
  lda #$80
  sta $0221

  ;; Bank from which to read.
  lda #$B5
  sta $021F
  phb
  pha
  plb

  ;; Read the content of the MTE.
  jml $C05F13

.no_extended:
  ;; Original incipit of the function.
  plb
  sty $021A
  ;; Jump back to the rest of the function.
  jml $C060F3
