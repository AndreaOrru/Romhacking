// TODO: comment this.
bank    = $0002
numBits = $0004
bits    = $0006


////
// Jump to decompression routine.
//
callDecompressData:
    jmp decompressData  // $C00760


////
// Decompression routine.
//
// IN:    X = index of the decompression status struct
//
// OUT:   A = 16 bits of decompressed data.
//
decompressData:
    php                 // $C0EBC0
    rep #$39            // $C0EBC1
    phb                 // $C0EBC3
    pea $7F7F           // $C0EBC4
    plb                 // $C0EBC7
    plb                 // $C0EBC8
    phx                 // $C0EBC9
    phy                 // $C0EBCA

.begin:
    // Something weird happens when this value is not 0xFEFE???
    lda $0014,x         // $C0EBCB
    cmp #$FEFE          // $C0EBCE
    beq .foundFEFE      // $C0EBD1

    // $0014 != 0xFEFE, return the value in $0014 and set $0014 to FEFE???
    tay                 // $C0EBD3
    lda #$FEFE          // $C0EBD4
    sta $0014,x         // $C0EBD7
    tya                 // $C0EBDA
    bra .return         // $C0EBDB

.foundFEFE:
    lda bank,x          // $C0EBDD
    bne .foundBank      // $C0EBE0

    // Weird stuff that happens when bank is set to 0...???
    lda $0000,x         // $C0EBE2
    tax                 // $C0EBE5
    bra .begin          // $C0EBE6

.foundBank:
    // TODO: figure this out.
    lda $0008,x         // $C0EBE8
    beq .startTreeWalk  // $C0EBEB

    txa                 // $C0EBED
    clc                 // $C0EBEE
    adc $0008,x         // $C0EBEF
    adc $0008,x         // $C0EBF2
    dec $0008,x         // $C0EBF5
    tay                 // $C0EBF8
    lda $0008,y         // $C0EBF9
    bra .expandBytepair // $C0EBFC

.startTreeWalk:
    // Load the index of the root of the tree.
    lda $FE6600         // $C0EBFE
.bitsLoop:
    // Get next bit in the stream. If 0, pick left child, else right.
    tay                 // $C0EC02
    jsr nextBit         // $C0EC03
    phx                 // $C0EC06
    tyx                 // $C0EC07
    bcc .leftChild      // $C0EC08

.rightChild:
    inx                 // $C0EC0A
    inx                 // $C0EC0B
.leftChild:
    // If most significant bit of the node is 1, it's a leaf.
    lda $FE6600,x       // $C0EC0C
    bmi .foundLeaf      // $C0EC10

    // Otherwise keep walking the tree using the node value as next offset.
    plx                 // $C0EC12
    bra .bitsLoop       // $C0EC13

.foundLeaf:
    plx                 // $C0EC15
    // Get rid of most significant bit.
    asl                 // $C0EC16
    lsr                 // $C0EC17
.expandBytepair:
    // x == 0x100 ---> 0xFFFF
    cmp #$0100          // $C0EC18
    beq .returnFFFF     // $C0EC1B

    // x < 0x100 ---> x
    bcc .return         // $C0EC1D

    // x > 0x100 ---> expand bytepair
    sbc #$0101          // $C0EC1F
    asl                 // $C0EC22
    asl                 // $C0EC23
    phx                 // $C0EC24
    tax                 // $C0EC25
    tay                 // $C0EC26
    // Get first two bytes from $FE6606[x * 4].
    lda $FE6606,x       // $C0EC27
    plx                 // $C0EC2B
    phy                 // $C0EC2C
    pha                 // $C0EC2D
    // TODO: figure out this $0008 non-sense.
    inc $0008,x         // $C0EC2E
    txa                 // $C0EC31
    clc                 // $C0EC32
    adc $0008,x         // $C0EC33
    adc $0008,x         // $C0EC36
    tay                 // $C0EC39
    pla                 // $C0EC3A
    sta $0008,y         // $C0EC3B
    txy                 // $C0EC3E
    plx                 // $C0EC3F
    // Get last two bytes from $FE6604[x * 4].
    lda $FE6604,x       // $C0EC40
    tyx                 // $C0EC44
    bra .loc_C0EC18     // $C0EC45

.returnFFFF:
    lda #$FFFF          // $C0EC47
.return:
    ply                 // $C0EC4A
    plx                 // $C0EC4B
    plb                 // $C0EC4C
    plp                 // $C0EC4D
    rtl                 // $C0EC4E


////
// Get the next bit in the Huffman stream.
//
// IN:    X = index of the decompression status struct
//
// OUT:   C flag = bit
//
// SIDE EFFECTS:
//        * bits,x    = bits,x << 1
//        * numBits,x = numBits,x - 1
//
nextBit:
    dec numBits,x       // $C008A5
    bne .dontFetch      // $C008A8

    jsr fetchData       // $C008AA
.dontFetch:
    asl bits,x          // $C008AD
    rts                 // $C008B0


////
// Fetch two bytes of compressed data.
//
// IN:    X = index of the decompression status struct
//
// SIDE EFFECTS:
//        * bits,x    = fetched bytes
//        * numBits,x = 16
//
fetchData:
    phd                 // $C008F0
    php                 // $C008F1
    rep #$30            // $C008F2
    tsc                 // $C008F4
    sec                 // $C008F5
    sbc #$0004          // $C008F6
    tcd                 // $C008F9
    tcs                 // $C008FA

    // Fetch bank.
    lda bank,x          // $C008FB
    bne .bankFound      // $C008FE
    // ...
    // Some weird stuff happens here if the bank is 0???
    // TODO: disassemble this.
    // ...
.bankFound:
    // Save bank in $03.
    sta $03             // $C00922
    // Save rest of pointer in $01-02
    lda $0000,x         // $C00924
    sta $01             // $C00927

    // Save next pointer (pointer += 2).
    inc                 // $C00929
    inc                 // $C0092A
    sta $0000,x         // $C0092B

    // Fetch the two bytes of data (inverted).
    lda [$01]           // $C0092E
    xba                 // $C00930
    sta bits,x          // $C00931
    // 16 bits were fetched.
    lda #$0010          // $C00934
    sta numBits,x       // $C00937

    sep #$20            // $C0093A
    pla                 // $C0093C
    pla                 // $C0093D
    pla                 // $C0093E
    pla                 // $C0093F
    plp                 // $C00940
    pld                 // $C00941
    rts                 // $C00942


////
// Fetch block address.
//
// IN:    A = block index
//
// OUT:   A = bank
//        X = lower 16-bits
//
fetchBlockAddress:
    php                 // $C0EA68
    rep #$39            // $C0EA69
    and #$0FFF          // $C0EA6B

    // Fetch lower 16 bits from $FE2E00[index * 2].
    asl                 // $C0EA6E
    tax                 // $C0EA6F
    lda $FE2E00,x       // $C0EA70
    // Save pointer in the stack.
    pha                 // $C0EA74
    txa                 // $C0EA75

    // Fetch bank index from $FE4E00[index / 2].
    lsr                 // $C0EA76
    lsr                 // $C0EA77
    tax                 // $C0EA78
    lda $FE4E00,x       // $C0EA79
    // If (index % 2) != 0, take upper 4 bits, otherwise low.
    bcc .lowerHalf      // $C0EA7D
.higherHalf:
    lsr                 // $C0EA7F
    lsr                 // $C0EA80
    lsr                 // $C0EA81
    lsr                 // $C0EA82
.lowerHalf:
    and #$000F          // $C0EA83
    tax                 // $C0EA86

    // Retrieve the actual bank.
    lda $C0EA91,x       // $C0EA87
    and #$00FF          // $C0EA8B
    plx                 // $C0EA8E
    plp                 // $C0EA8F
    rtl                 // $C0EA90
