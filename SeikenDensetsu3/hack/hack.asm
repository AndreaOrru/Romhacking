;;;;
;; Expand ROM to 6MB.
;;
norom

;; Signal 48Mbit.
org $FFD5
    db $35,$02,$0D

;; Fill with zeroes till 6MB.
!address = $400000
while !address < $600000
    org !address
        fillbyte $00 : fill $10000
    !address #= !address+$10000
endif

;; Copy the first bank.
org $408000
    !address = $8000
    while !address < $10000
        dd read4(!address)
        !address #= !address+4
    endif

exhirom



;;;;
;; Fetch block address from a new location.
;;
org $C0EA68
fetchBlockAddress:
    ;; A = Block index.
    php
    rep #$39
    and #$0FFF

    ;; Multiply by 4.
    asl
    asl
    ;; Fetch lower 16 bits of address.
    tax
    lda $400000,x
    pha  ; Save offset in the stack.
    ;; Fetch bank.
    lda $400002,x
    and #$00FF

    plx  ; Restore offset from the stack.
    plp
    rtl

;; Handle long jump from bank $C0.
jumpCheckIfDecompressed:
    jml checkIfDecompressed

;; Put the addresses in the new location, translated in
;; this format: ppppbb00, where p = offset, b = bank.
org $400000
    ;; Iterate through the blocks.
    !index = $0
    while !index < $1000
        ;; Fetch the offset.
        dw read2($FE2E00+(!index*2))
        ;; Fetch and decode the bank index.
        !bankIndex = read1($FE4E00+(!index/2))
        if (!index&1) != 0
            !bankIndex #= !bankIndex>>4
        endif
        !bankIndex #= !bankIndex&$F
        ;; Fetch the bank.
        db read1($C0EA91+!bankIndex)
        db $00
        !index #= !index+1
    endif



;;;;
;; Patch the decompression routine to call
;; our custom functions.
;;
org $C0EBE8
decompressData_foundBank:
    ;; ORIGINAL:
    ;;     lda $0008,x
    ;;
    ;; Right after the bank has been fetched,
    ;; and the game is checking whether there
    ;; is decompressed data inside the buffer.

    ;; We jump to our own routine instead.
    jmp jumpCheckIfDecompressed
    decompressData_continue:
    ;; ...continue with original routine...

org $C0EC4A
decompressData_return:
    ;; Return decompressed data in A...



;;;;
;; Do not decompress using Huffman and bytepair
;; if reading from one of the reinserted blocks.
;;
org $404000
checkIfDecompressed:
    ;; A = Bank.
    ;; X = Struct offset.

    ;; If bank >= 0xC0, continue as usual (compressed).
    cmp #$00C0
    bcs .compressed

.decompressed:
    ;; Save bank in $03.
    sta $03
    ;; Save rest of pointer in $01-02.
    lda $0000,x  ; Contains the pointer.
    sta $01

    ;; Advance pointer.
    inc $0000,x

    ;; Load the data.
    lda [$01]
    ;; Check if it's the special 0xFFFF case.
    cmp #$FFFF
    beq .ffff

    ;; It's not - only return the first byte.
    and #$00FF
    bra .return

.ffff:
    ;; It is - return two bytes.
    inc $0000,x
.return:
    jml decompressData_return

.compressed:
    ;; Just go back to decompression routine.
    lda $0008,x    ; Instruction we substituted with jump.
    jml decompressData_continue
