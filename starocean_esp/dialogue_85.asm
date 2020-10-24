arch 65816
exhirom


;;------------------------------------------------------------------------------
org $C04984
LoadBlockInRAM:
    jml HandleDeJapBlock        ; Take care of the case in which type = $85.
    ;; ...

org $C04997
LoadDialogueBlockMagno:
    ;; The rest of the cases.
    ;;     cmp #$81
    ;;     ...


;;------------------------------------------------------------------------------
org $C9FC90
HandleDeJapBlock:
    php
    sep #$20
    stz $B1
    stz $B2
    sta $B3
    stz $A7
    stz $A5
    stz $B7
    sta $B8
    lda [$B1],y                 ; Load header's type.
    cmp #$85                    ; $85 = DeJap compression.
    beq LoadDialogueBlockDeJap
.isMagno:
    jml LoadDialogueBlockMagno

LoadDialogueBlockDeJap:
    ;; Copy only the initial part of the block (pointers).
    lda $B3
    pha
    rep #$20
    iny
    lda [$B1],y
    sta $A2
    tya
    clc
    adc #$0008
    tay
    pha
    lda [$B1],y
    lsr a
    lsr a
    lsr a
    lsr a
    sta $A5
.loc_C06C47:
    lda [$B1],y
    sta $0000,x
    iny
    iny
    inx
    inx
    dec $A5
    bne .loc_C06C47
    lda $A2
    asl a
    asl a
    asl a
    sta $0000,x
    inx
    inx
    pla
    sta $0000,x
    inx
    inx
    sep #$20
    pla
    rep #$20
    and #$00FF
    sta $0000,x
    plp
    rtl
