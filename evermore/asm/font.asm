arch 65816
hirom


;; Substitute font.
org $C40002
  incbin font.bin


;; Make font start from 0x10 instead of 0x20.
org $CC9ED7
	sbc #$10
org $CCA54E
	sbc #$10
org $CCA5A3
	sbc #$10
org $CCA929
	sbc #$10
org $CCB9B7
  sbc #$10
org $CCB9FC
  sbc #$10
org $CCBB20
  sbc #$10
org $CCC950
  sbc #$10
org $CCC9A4
  sbc #$10


;; Choice arrow character has a different index now.
org $CC9E8B
  lda #$0071
org $CCA492
  lda #$0071
