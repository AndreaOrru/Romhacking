arch 65816
hirom


!message = $22
!pointers = $26


;; Disable compression and use normal pointers.
macro getPointer()
  sta !message
  iny
  lda [!pointers],y
  xba
  and #$00FF
  sta !message+2
  rtl


endmacro
org $CCCD01
  %getPointer()

org $CCCD3F
  %getPointer()
