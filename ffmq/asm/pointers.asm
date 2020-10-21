;;;;
;; Expand ROM to 1MB.
;;
norom

;; Signal 8Mbit.
org $7FD7
    db $0A

;; Fill with zeroes till 1MB.
!address = $80000
while !address < $100000
    org !address
        fillbyte $00 : fill $10000
    !address #= !address+$10000
endif

arch 65816
lorom


;;;;
;; Add control byte $F0 (function 1).
;;
org $009DBD
LoadDialogueByte_1:
    ;; lda [$17]
    ;; inc $17
    jml CheckF0_1
    ;; and #$00FF
    nop
    nop
    nop
Check80_1:
    ;; cmp #$0080


;;;;
;; Add control byte $F0 (function 2).
;;
org $00E076
LoadDialogueByte_2:
    ;; lda [$17]
    ;; inc $17
    jml CheckF0_2
    ;; and #$00FF
    nop
    nop
    nop
Check80_2:
    ;; cmp #$0080


;;;;
;; Implement byte F0 - jump to pointer.
;;
macro CheckF0(n)
    lda [$17]                   ; Load byte.
    inc $17                     ; Advance pointer.
    and #$00FF
    ;; Check against $F0.
    cmp #$00F0
    bne .check80

.isF0:
    ;; Save state.
    php
    phx

    ;; Load index of dialogue.
    lda [$17]

    ;; X = offset of the pointer in the table.
    asl
    clc
    adc [$17]                   ; Multiply by 2, add 1.
    tax

    ;; Copy least significant 16-bits of pointer into $17.
    lda.l PointerTable,x        ; Load 16-bits offset.
    sta $17                     ; Save into dialogue pointer.

    ;; Copy most significant 8-bits of pointer into $17 + 2.
    sep #$20                    ; 8-bit accumulator.
    lda.l PointerTable+2,x      ; Load bank byte.
    sta $19                     ; Save into dialogue pointer (bank).

    ;; Restore state.
    plx
    plp
    if <n> == 1
        jml LoadDialogueByte_1
    else
        jml LoadDialogueByte_2
    endif

.check80:
    if <n> == 1
        jml Check80_1
    else
        jml Check80_2
    endif
endmacro

org $108000
CheckF0_1:
    %CheckF0(1)
CheckF0_2:
    %CheckF0(2)


;;;;
;; Data.
;;
org $109000
PointerTable:
    dl $10A000
    dl $03F87B


org $03F830
FirstSentenceOld:
    db $F0
    dw $0000


org $10A000
FirstSentenceNew:
    db $1A
    db $00
    db $AB,$C2,$B5,$B8,$C5,$C7,$C2,$FF,$C3,$C8,$C3,$C3,$B4,$CE

    db $F0
    dw $0001
