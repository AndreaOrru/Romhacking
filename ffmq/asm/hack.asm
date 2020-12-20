;;;;
;; Expand ROM to 1MB.
;;
norom

;; Signal 8Mbit.
org $7FD7
    db $0A

;; Fill with zeroes till 1MB.
;!address = $80000
;while !address < $100000
;    org !address
;        fillbyte $00 : fill $10000
;    !address #= !address+$10000
;endif

arch 65816
lorom


;; Add accented characters.
org $078242
    incbin "font.bin"


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
    ;; bcc lessThan80
    ;; eor $1D
org $009DCB
    ;; sta [$1A]
    ;; inc $1A
    jsl AccentedFont
    ;; inc $1A


org $00DE0F
OriginalLoadFontProperties:
    ;; ...
org $00DE28
    ;; and #$0FFE
    ;; sta $0D
    jsl LoadAccentedFontPointer
    nop
org $00DE40
    ;; and #$000F
    ;; sta $13
    ;; sep #$20
    ;; lda $E552,x
    ;; sta $4202
    jsl RepositionAccentedFont
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
org $00DEB6
    rtl

org $07F850
LoadAccentedFontPointer:
    and #$0FFE
    phx
    pha
    lda $7FA3FE
    beq .normalChar
.accentedFont:
    sec
    sbc #$00DE
    asl
    tax
    pla
    lda.l .table,x
    sta $0d
    plx
    rtl
.normalChar:
    pla
    plx
    sta $0d
    rtl
.table:
    dw $0212                    ; È
    dw $0254                    ; à
    dw $0256                    ; é
    dw $0258                    ; è
    dw $025A                    ; ì
    dw $025C                    ; ò
    dw $025E                    ; ù

RepositionAccentedFont:
    and #$000F
    sta $13
    sep #$20
    lda $E552,x
    sta $4202

    lda $7FA3FE
    cmp #$DE
    beq .adjustE
    cmp #$E2
    beq .adjustI
    rtl
.adjustE:
    stz $4202
    rtl
.adjustI:
    inc $13
    lda #$01
    sta $4202
    rtl


org $00E03F
    jsl HandleNewline
    rts


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
    ;; bcc lessThan80
org $00E082
    ;; pha
    ;; jsr OriginalLoadFontProperties
    jsl LoadFontProperties
    ;; pla
    nop


;;;;
;; Implement byte F0 - jump to pointer.
;;
macro CheckF0(n)
    ;; Check bank.
    lda $19
    and #$00FF
    cmp #$0010
    bcs .checkF0
    ;; Check if we're inside menu.
    lda $7F0000
    cmp #$2CFE
    bne .checkF0
.checkF0F1:
    lda [$17]
    cmp #$F1F0
    bne .check80Load
    inc $17
    inc $17
    bra .isF0

.checkF0:
    ;; Check against $F0.
    lda [$17]
    inc $17
    and #$00FF
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

.check80Load:
    lda [$17]
    inc $17
    and #$00FF
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

AccentedFont:
.check_mode:
    pha
    lda $7F0000
    cmp #$2CFE
    beq .write
    pla
    pha
.check_accented:
    cmp #$00DE
    bcc .write
    cmp #$00E5
    bcs .write
.accented:
    sec
    sbc #$00B5
.write:
    pla
    sta [$1A]
    inc $1A
    rtl

LoadFontProperties:
.check_range:
    cmp #$00DE
    bcc .normalChar
    cmp #$00E5
    bcs .normalChar
.check_ram:
    pha
    lda $17
    cmp #$1FFF
    pla
    bcc .normalChar
.accentedChar:
    sta $7FA3FE
    sec
    sbc #$00DE
    asl
    tax
    lda.l .table,x

    jsl OriginalLoadFontProperties

    lda #$0000
    sta $7FA3FE
    lda #$00B5
    rtl
.normalChar:
    pha
    jsl $DE0F
    pla
    rtl
.table:
    dw $00AA                    ; È
    dw $009A                    ; à
    dw $00B5                    ; é
    dw $00B5                    ; è
    dw $00BC                    ; ì
    dw $00B5                    ; ò
    dw $00B5                    ; ù

HandleNewline:
    pha
    tay
    lda $7FA3FE
    bne .accentedChar
.normalChar:
    tya
.accentedChar:
    tay
    lda #$0080
    and $00dd
    bne .return
    tya
    ldx $00ca
    sta $7e35d9,x
    inc $00ca
.return:
    pla
    rtl


;;;;
;; Data.
;;
org $109000
PointerTable:
