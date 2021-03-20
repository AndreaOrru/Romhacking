namespace dialogue {
  namespace buffer {
    constant pointer = $0CB1  // Pointer to the drawing buffer.
    constant length  = $0CB3  // Length of the current dialogue (in pixels?).
  }

  namespace colors {
    constant current = $0CBE  // Color of the font to be rendered.
    // Color codes.
    constant WHITE  = $02
    constant YELLOW = $04
    constant RED    = $08
  }

  namespace hook {
    enqueue pc

    // Render a character of dialogue.
    seek($81E1D3)
    function renderChar {
      jsl vwf.renderChar       // Render the character as a VWF.
      ldx.w buffer.length      // X = pixel size of dialogue.
      lda.w buffer.pointer     // A = original buffer pointer (before VWF rendering).
      // Have we moved to the next tile while drawing the character?
      cmp.w vwf.bufferPointer
      beq sameTile             // No  -> continue without adding any new tile.
      jsl vwf.renderTile       // Yes -> add the tile to the tilemap.
      bra renderTileContinued

      seek($81E1FB)
      renderTileContinued: {
        // ...
      }

      seek($81E227)
      sameTile: {
        // ...
      }
    }

    // Handle a newline.
    seek($81E5FE)
    function newline {
      jsl vwf.newline
      plp
      rts
    }

    // Setup color palettes for dialogues.
    seek($81EAF1)
    function setDialoguePalettes {
      php
      jsl vwf.setDialoguePalettes
      plp
      rtl
    }

    dequeue pc
  }

  namespace vwf {
    constant charPointer   = $00    // Pointer to the char data in ROM.
    constant bufferPointer = $02    // Pointer to the drawing buffer.
    constant charRows      = $04    // Counter of character rows to be drawn.
    constant zero          = $1CC0  // Zero value (for DMA purposes).
    constant charWidth     = $1CC2  // Current char's width in pixels.
    constant rightShifts   = $1CC4  // Number of required shifts to the right.
    constant leftShifts    = $1CC6  // Number of required shifts to the left.

    namespace shift {
      // Shift directions.
      constant LEFT  = 0
      constant RIGHT = 1

      // Shift row left by {leftShifts}.
      // INPUT:  A = row of pixels.
      function left {
        phx
        tax
        rep #$20

        lda #shiftsEnd-1
        sub.w leftShifts
        pha; txa; rts

        asl #15
        shiftsEnd: {
          sep #$20
          plx
          rts
        }
      }

      // Shift row right by {rightShifts}.
      // INPUT:  A = row of pixels.
      function right {
        phx
        tax
        rep #$20

        lda #shiftsEnd-1
        sub.w rightShifts
        pha; txa; rts

        lsr #15
        shiftsEnd: {
          sep #$20
          plx
          rts
        }
      }
    }

    // Render a row of pixels at address {base} and color {color}.
    // INPUT:  A = row of pixels.
    // INPUT:  Y = index in the rendering buffer.
    macro RenderRow(base, color) {
      if {color} == colors.WHITE {
        ora.w {base},y
        sta.w {base},y
      } else if {color} == colors.YELLOW {
        ora.w {base}+1,y
        sta.w {base}+1,y
      } else {
        pha
        ora.w {base},y
        sta.w {base},y
        pla
        ora.w {base}+1,y
        sta.w {base}+1,y
      }
    }

    // Render a character, shifting in {shiftDirection} by
    // the required amount and in the given {color}.
    // INPUT:  X = index in the font data.
    // INPUT:  Y = index in the rendering buffer.
    macro RenderingLoopColor(shiftDirection, color) {
      renderingLoop{#}: {
        // LOAD  - Row 0+y, Column 0-7
        lda $9F8000,x
        xba
        // LOAD  - Row 0+y, Column 8-15
        lda $9F8008,x
        // SHIFT - Row 0+y
        if {shiftDirection} == shift.LEFT {
          jsr shift.left
        } else {
          jsr shift.right
        }
        // STORE - Row 0+y
        RenderRow($8010, {color})
        xba
        RenderRow($8000, {color})

        // LOAD  - Row 7+y, Column 0-7
        lda $9F8080,x
        xba
        // LOAD  - Row 7+y, Column 8-15
        lda $9F8088,x
        // SHIFT - Row 7+y
        if {shiftDirection} == shift.LEFT {
          jsr shift.left
        } else {
          jsr shift.right
        }
        // STORE - Row 7+y
        RenderRow($8030, {color})
        xba
        RenderRow($8020, {color})

        iny #2  // Advance buffer pointer.
        inx     // Advance font pointer.

        // Loop to the next row.
        dec.w charRows
        bpl renderingLoop{#}
      }
    }

    // Render a character, shifting in {shiftDirection} by the required amount.
    // INPUT:  X = index in the font data.
    // INPUT:  Y = index in the rendering buffer.
    macro RenderingLoop(shiftDirection) {
      lda.w colors.current
      cmp.b #colors.YELLOW
      beq yellow
      cmp.b #colors.RED
      beq red

      white: {
        RenderingLoopColor({shiftDirection}, colors.WHITE)
        brl +
      }
      yellow: {
        RenderingLoopColor({shiftDirection}, colors.YELLOW)
        brl +
      }
      red: {
        RenderingLoopColor({shiftDirection}, colors.RED)
      }

      +
    }

    // Render a character, shifting left by the required amount.
    // INPUT:  X = index in the font data.
    // INPUT:  Y = index in the rendering buffer.
    function renderingLoopLeft {
      RenderingLoop(shift.LEFT)
      rts
    }
    // Render a character, shifting right by the required amount.
    // INPUT:  X = index in the font data.
    // INPUT:  Y = index in the rendering buffer.
    function renderingLoopRight {
      RenderingLoop(shift.RIGHT)
      rts
    }

    // Render VWF character.
    function renderChar {
      php
      rep #$20
      phx; phy
      txa
      jsr fetchWidth

      cmp #$07FF
      bne deriveCharPointer
      stz.b charPointer
      bra initLoop

      deriveCharPointer: {
        and #$0007
        asl #4
        sta.b charPointer
        txa
        and #$FFF8
        asl #5
        ora.b charPointer
        sta.b charPointer
      }

      initLoop: {
        // Setup loop counter.
        lda #$0007
        sta.b charRows
        // Setup buffer pointers.
        ldx.b charPointer
        ldy.b bufferPointer
        // Setup data bank.
        sep #$20
        phb; lda #$7E; pha; plb
        rep #$20
      }

      clearBuffers: {
        // If we're at the beginning of the window, clear the VRAM buffer.
        lda.w buffer.length
        bne +
        jsr clearVRAMBuffer

        // If we're at the beginning of a tile in RAM, clear it.
        +; sep #$20
        lda.w rightShifts
        bne renderingLoop
        jsr clearTile
      }

      renderingLoop: {
        jsr renderingLoopRight
      }

      calculatePixels: {
        // Add the width of the character to the rightShifts counter.
        rep #$20
        lda.w rightShifts
        add.w charWidth
        sta.w rightShifts
        // If rightShifts < 16, we still have space in the current tile.
        cmp #$0010
        bcc +
      }

      // No space in the current tile, move to the next.
      nextTile: {
        // rightShifts %= 16.
        and #$000F
        sta.w rightShifts

        // Move to the next tile in the buffer.
        lda.w bufferPointer
        add #$0040
        sta.w bufferPointer

        // Draw the rest of the current character if necessary.
        jsr remainingPixels
      }

      +; plb; ply; plx; plp
      rtl
    }

    function remainingPixels {
      // If the tile was perfectly full, skip.
      lda.w rightShifts
      beq +
      // Otherwise, clear the new tile.
      jsr clearTile

      // Calculate the number of required left shifts.
      lda.w charWidth
      sub.w rightShifts
      sta.w leftShifts

      // Initialize the rendering loop.
      lda #$0007
      sta.w charRows
      ldy.w bufferPointer
      lda.w charPointer
      tax

      renderingLoop: {
        sep #$20
        jsr renderingLoopLeft
      }

      +; rts
    }

    // Fetch the width of the current character.
    // INPUT:  X = character code.
    function fetchWidth {
      pha
      beq space
      cmp #$00A1
      bcc japanese
      cmp #$00E0
      bcs japanese

      // ($A1 <= x < $E0) -> Latin.
      latin: {
        lda.l font.widthTable-$a1,x
        and #$00FF
        bra +
      }

      // (x < $A1) or (x >= $E0) -> Japanese.
      japanese: {
        lda #$000D
        bra +
      }

      // (x = $00) -> Space.
      space: {
        lda #$0004
      }

      +; sta charWidth
      pla; rts
    }

    // Render VWF tile(s) into the tilemap.
    function renderTile {
      lsr #4               // Calculate tile index.
      add #$2900           // Use the 3rd palette.
      // Draw 1st tile.
      sta $7EC844,x ; inc
      sta $7EC846,x ; inc
      sta $7EC884,x ; inc
      sta $7EC886,x ; inc
      inx #4
      // Draw 2nd tile to account for remaining pixels.
      sta $7EC844,x ; inc
      sta $7EC846,x ; inc
      sta $7EC884,x ; inc
      sta $7EC886,x
      rtl
    }

    function newline {
      // Pre-existing code.
      lda.w $0CC3
      and #$00FF
      asl
      tax

      // Reset VWF shift values.
      lda #$0000
      sta.w vwf.rightShifts
      sta.w vwf.leftShifts

      // Pre-existing code.
      lda $81E57A,x
      sta.w buffer.length
      lda #$8001
      sta.w $0CA0
      sta.w $0CA2

      rtl
    }

    // Setup color palettes for dialogues.
    // We want to be able to switch between colors by manipulating
    // pixel data instead of changing palettes, so we setup
    // the 3rd palette to contain all the colors we need.
    function setDialoguePalettes {
      rep #$20

      // Black.
      tdc
      sta $7ED000
      sta $7ED008
      sta $7ED010
      // White.
      lda #$7FFF
      sta $7ED002
      sta $7ED012
      // Yellow.
      lda #$0FFF
      sta $7ED00A
      sta $7ED014
      // Red.
      lda #$1CFF
      sta $7ED016

      rtl
    }

    // Clear the current tile in the RAM buffer.
    function clearTile {
      php; phx; phy
      rep #$20

      lda #$0000
      ldx.w bufferPointer
      ldy #$0020

      loop: {
        sta $7E8000,x
        inx #2
        dey
        bne loop
      }

      ply; plx; plp
      rts
    }

    // Clear the VRAM buffer containing the dialogue window.
    function clearVRAMBuffer {
      // Wait for VBlank.
      wai
      // Prepare a value of #$0000 in RAM.
      lda #$0000
      sta.w zero
      // VRAM destination address.
      lda #$3800
      sta $002116
      // RAM source address.
      lda #zero
      sta $004302
      // Number of bytes to transfer.
      lda #$0800
      sta $004305
      // VRAM address increment after writing to $2119.
      sep #$20
      lda #$80
      sta $002115
      // DMA transfer writes 2 bytes without incrementing RAM address.
      lda #$09
      sta $004300
      // Destination register $2118 (VRAM).
      lda #$18
      sta $004301
      // RAM source address bank $7E.
      lda #$7E
      sta $004304
      // Start the transfer.
      lda #$01
      sta $00420B
      rts
    }
  }
}
