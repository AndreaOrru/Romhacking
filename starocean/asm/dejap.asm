arch 65816
exhirom


!decompress_addr = $FFC000
incsrc "decompress.asm"


;; Bitmasks had to be moved from the original position.
;; Make sure references to it are correct.
org $C2FF86
  lda bitmasks,x
