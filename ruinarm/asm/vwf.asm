arch 65816
lorom


;;------------------------------------------------
;;                    VARIABLES
;;------------------------------------------------
!LineLength         = $0CB3
!RenderBufferPtr    = $0CB1
!FontColor          = $0CBE
!RenderBufferPtrTmp = $02
!CharRows           = $04
!Zero               = $7E1CC0
!Width              = $7E1CC2
!RightShifts        = $7E1CC4
!LeftShifts         = $7E1CC6
!FontPtr            = $7E1CC8


;;------------------------------------------------
;;                      FONT
;;------------------------------------------------
org $9F8000
incbin "../gfx/f8000_font_vwf.bin"


org $BB90B8
db $01,$fc,$af,$00,$bc,$00,$b3,$00,$b5,00,$ab,$00,$00,$f4,$46,$f4,$c1,$fa,$00,$f6
db $ba,$00,$d9,$00,$d4,$00,$d4,$00,$c5,$00,$00,$00,$ca,$00,$d3,$00,$d6,$00,$d8,$00,$c9,$00,$00,$f6
db $ba,$00,$d9,$00,$d4,$00,$d4,$00,$c5,$00,$00,$00,$ca,$00,$d3,$00,$d6,$00,$d8,$00,$c9,$00,$00,$f6
db $ba,$00,$d9,$00,$d4,$00,$d4,$00,$c5,$00,$00,$00,$ca,$00,$d3,$00,$d6,$00,$d8,$00,$c9,$00,$00,$00
db $ba,$00,$d9,$00,$d4,$00,$d4,$00,$c5,$00,$00,$00,$ca,$00,$d3,$00,$d6,$00,$d8,$00,$c9,$00
db $00,$f7


;;------------------------------------------------
;;                EXISTING ROUTINES
;;------------------------------------------------
org $81E17F
RenderCharParent:
    ;; ...
org $81E1CC
    tax
    lda.w !RenderBufferPtr
    sta.w !RenderBufferPtrTmp
    jsl RenderVWFChar
    ldx.w !LineLength
    ;; Check if we're moving to the next tile.
    lda.w !RenderBufferPtr
    cmp.w !RenderBufferPtrTmp
    beq .continue
    jml AdvanceTile
org $81E1FB
.restAdvanceTile:
org $81E227
.continue:
    ;; ...

org $81E929
RenderChar:
    ;; ...

org $81E5FE
NewLine:
    jsl NewLineVWF
    plp
    rts

org $81EAF1
SetDialoguePalettes:
    php
    jsl SetDialoguePalettesVWF
    plp
    rtl


;;------------------------------------------------
;;                  NEW ROUTINES
;;------------------------------------------------

!LEFT   = 0
!RIGHT  = 1
!WHITE  = 2
!YELLOW = 4
!RED    = 8

macro RenderRow(base, color)
    if <color> == !WHITE
        ora <base>,y
        sta <base>,y
    elseif <color> == !YELLOW
        ora <base>+1,y
        sta <base>+1,y
    else
        pha
        ora <base>,y
        sta <base>,y
        pla
        ora <base>+1,y
        sta <base>+1,y
    endif
endmacro

macro RenderingLoop(shiftDirection, color)
?renderingLoop:
    ;; LOAD  - Row 0+y, Column 0-7
    lda $9F8000,x
    xba
    ;; LOAD  - Row 0+y, Column 8-15
    lda $9F8008,x
    ;; SHIFT - Row 0+y
    if <shiftDirection> == !LEFT
        jsr ShiftLeft
    else
        jsr ShiftRight
    endif
    ;; STORE - Row 0+y
    %RenderRow($8010, <color>)
    xba
    %RenderRow($8000, <color>)

    ;; LOAD - Row 7+y, Column 0-7
    lda $9F8080,x
    xba
    ;; LOAD - Row 7+y, Column 8-15
    lda $9F8088,x
    ;; SHIFT - Row 0+y
    if <shiftDirection> == !LEFT
        jsr ShiftLeft
    else
        jsr ShiftRight
    endif
    ;; STORE - Row 7+y
    %RenderRow($8030, <color>)
    xba
    %RenderRow($8020, <color>)

    iny
    iny
    inx

    dec !CharRows
    bpl ?renderingLoop
endmacro

org $80EAE8
RenderVWFChar:
    php
    rep #$20
    phx
    phy
    txa
    jsr CalculateWidth
    cmp #$7F00
    bcc .loc_81E937
    jmp .arrow

.loc_81E937:
    cmp #$07FF
    bne .loc_81E940
    stz $00
    bra .loc_81E956

.loc_81E940:
    and #$0007
    asl
    asl
    asl
    asl
    sta $00
    txa
    and #$FFF8
    asl
    asl
    asl
    asl
    asl
    ora $00
    sta $00

.loc_81E956:
    lda #$0007
    sta !CharRows
    ldx $00
    ldy !RenderBufferPtrTmp
    sep #$20
    phb
    lda #$7E
    pha
    plb
    rep #$20

.vwfInit:
    ;; Save the address of the character font.
    txa
    sta !FontPtr

    ;; If we're at the beginning of the window, clear VRAM buffer.
    lda !LineLength
    bne .clearTile
    jsr ClearVRAMBuffer

.clearTile:
    ;; If we're at the beginning of a tile, clear it.
    sep #$20
    lda !RightShifts
    bne .renderingLoop
    jsr ClearTile

.renderingLoop:
    jsr RenderingLoopRight

.calculatePixels:
    rep #$20
    lda !RightShifts
    clc
    adc !Width
    sta !RightShifts
    cmp #$0010
    bcc .return

.nextTile:
    and #$000F
    sta !RightShifts

    lda !RenderBufferPtrTmp
    clc
    adc #$0040
    sta !RenderBufferPtrTmp

    jsr RemainingPixels


.return:
    plb
    ply
    plx
    plp
    rtl

;; Arrow?
.arrow:
    brk #$00


RemainingPixels:
    ;; If the tile is perfectly full, skip.
    lda !RightShifts
    beq .return
    jsr ClearTile

    ;; Calculate number of left shifts.
    lda !Width
    sec
    sbc !RightShifts
    sta !LeftShifts

    ;; Load initial loop parameters.
    lda #$0007
    sta !CharRows
    ldy !RenderBufferPtrTmp
    lda !FontPtr
    tax
.renderingLoop:
    sep #$20
    jsr RenderingLoopLeft
.return:
    rts


CalculateWidth:
    pha

    beq .space
    cmp #$00A1
    bcc .jap
    cmp #$00E0
    bcs .jap

.eng:
    lda.l vwfTable-$a1,x
    and #$00FF
    bra .save
.jap:
    lda #$0010
    bra .save
.space:
    lda #$0004
.save:
    sta !Width

    pla
    rts


ClearTile:
    php
    phx
    phy

    rep #$20
    lda #$0000
    ldx !RenderBufferPtrTmp
    ldy #$0040
.loop:
    sta $7E8000,x
    inx
    inx
    dey
    bne .loop

    ply
    plx
    plp
    rts


ClearVRAMBuffer:
    wai
    ; Set a 0 value in RAM.
    lda #$0000
    sta !Zero
    ; VRAM destination address.
    lda #$3800
    sta $002116
    ; RAM source address.
    lda #!Zero
    sta $004302
    ; Bytes to transfer.
    lda #$0800
    sta $004305
    ; VRAM address increment after writing to $2119.
    sep #$20
    lda #$80
    sta $002115
    ; DMA transfer writes 2 bytes without incrementing RAM address.
    lda #$09
    sta $004300
    ; Destination register $2118.
    lda #$18
    sta $004301
    ; RAM source address bank $7E.
    lda #$7E
    sta $004304
    ; Start the transfer.
    lda #$01
    sta $00420B
    rts


NewLineVWF:
    lda $0CC3
    and #$00FF
    asl
    tax
    lda #$0000
    sta !RightShifts
    sta !LeftShifts
    lda $81E57A,x
    sta !LineLength
    lda #$8001
    sta $0CA0
    sta $0CA2
    rtl


SetDialoguePalettesVWF:
    rep #$20

    ;; Black.
    tdc
    sta $7ED000
    sta $7ED008
    sta $7ED010

    ;; White.
    lda #$7FFF
    sta $7ED002
    sta $7ED012

    ;; Yellow.
    lda #$0FFF
    sta $7ED00A
    sta $7ED014

    ;; Red.
    lda #$1CFF
    sta $7ED016

    rtl


AdvanceTile:
    lsr
    lsr
    lsr
    lsr
    clc
    adc #$2900
    ;; ora $0CBD                   ; Apply color.
    sta $7EC844,x
    inc
    sta $7EC846,x
    inc
    sta $7EC884,x
    inc
    sta $7EC886,x
    inx
    inx
    inx
    inx
    inc
    sta $7EC844,x
    inc
    sta $7EC846,x
    inc
    sta $7EC884,x
    inc
    sta $7EC886,x
    jml RenderCharParent_restAdvanceTile


RenderingLoopLeft:
    lda !FontColor
    cmp #!YELLOW
    beq .yellow
    cmp #!RED
    beq .red
.white:
    %RenderingLoop(!LEFT, !WHITE)
    brl .return
.yellow:
    %RenderingLoop(!LEFT, !YELLOW)
    brl .return
.red:
    %RenderingLoop(!LEFT, !RED)
.return:
    rts


RenderingLoopRight:
    lda !FontColor
    cmp #!YELLOW
    beq .yellow
    cmp #!RED
    beq .red
.white:
    %RenderingLoop(!RIGHT, !WHITE)
    brl .return
.yellow:
    %RenderingLoop(!RIGHT, !YELLOW)
    brl .return
.red:
    %RenderingLoop(!RIGHT, !RED)
.return:
    rts


;;------------------------------------------------
;;                  SHIFT ROUTINES
;;------------------------------------------------
ShiftLeft:
    phx
    tax
    rep #$20

    lda #.shiftsEnd-1
    sec
    sbc !LeftShifts
    pha
    txa
    rts

.shifts:
    asl
    asl
    asl
    asl
    asl
    asl
    asl
    asl
    asl
    asl
    asl
    asl
    asl
    asl
    asl
.shiftsEnd:
    sep #$20
    plx
    rts


ShiftRight:
    phx
    tax
    rep #$20

    lda #.shiftsEnd-1
    sec
    sbc !RightShifts
    pha
    txa
    rts

.shifts:
    lsr
    lsr
    lsr
    lsr
    lsr
    lsr
    lsr
    lsr
    lsr
    lsr
    lsr
    lsr
    lsr
    lsr
    lsr
.shiftsEnd:
    sep #$20
    plx
    rts


;; VWF table.
vwfTable:
    db $09  ; 0
    db $04  ; 1
    db $09  ; 2
    db $08  ; 3
    db $09  ; 4
    db $08  ; 5
    db $08  ; 6
    db $09  ; 7
    db $08  ; 8
    db $08  ; 9

    db $0B  ; A
    db $07  ; B
    db $0A  ; C
    db $09  ; D
    db $07  ; E
    db $07  ; F
    db $0B  ; G
    db $09  ; H
    db $03  ; I
    db $05  ; J
    db $0A  ; K
    db $07  ; L
    db $0F  ; M
    db $0C  ; N
    db $0D  ; O
    db $07  ; P
    db $0D  ; Q
    db $08  ; R
    db $07  ; S
    db $0B  ; T
    db $0A  ; U
    db $0D  ; V
    db $0F  ; W
    db $0B  ; X
    db $09  ; Y
    db $0A  ; Z

    db $08  ; a
    db $07  ; b
    db $07  ; c
    db $07  ; d
    db $07  ; e
    db $06  ; f
    db $08  ; g
    db $07  ; h
    db $03  ; i
    db $04  ; j
    db $07  ; k
    db $03  ; l
    db $0B  ; m
    db $07  ; n
    db $08  ; o
    db $07  ; p
    db $07  ; q
    db $06  ; r
    db $06  ; s
    db $06  ; t
    db $07  ; u
    db $09  ; v
    db $0B  ; w
    db $09  ; x
    db $09  ; y
    db $07  ; z
