;The LCD of this device isn't connected to the perfectly good internal LCD
;subsystem, but is an external one with its own controller. It seems to be
;connected to the dataspace when DRRH==3. Commands to it go to $8000, data
;to $c000. The controller is PCF8833-compatible btw, same type as some
;Nokia 6100s (and other Nokias) have.

    CPU 65c02
    OUTPUT HEX
    INCLUDE spec
    * = $2000

;Push registers	
	lda $35
	pha

;select lcd
	lda #$3
	sta $35

	stz LEN0
;wait for usb packet
waitpacket lda $73
	and #$4
	beq waitpacket

;fetch command
	lda $200
	cmp #$0
	beq copy2fb
	cmp #$1
	beq setaddr
	cmp #$2
	beq setbl
	bra packetend

;nothing yet...	
setbl  bra packetend

;set addr
setaddr	lda #$2b
	sta $8000
	lda $201
	sta $c000
	lda $202
	sta $c000

	lda #$2a
	sta $8000
	lda $203
	sta $c000
	lda $204
	sta $c000
	lda #$2c
	sta $8000
	bra packetend

;copy packet to framebuff. Len is in $201
copy2fb	lda $201
	tay
	ldx #$2
copyloop lda $200,x
	sta $c000
	inx
	dey
	bne copyloop
	bra packetend


;subtract 0x40 from 37A:37D.
;Damn, this is way easier on an ARM :P
packetend sec
	lda LEN0
	sbc #$40
	sta LEN0
	lda LEN1
	sbc #$0
	sta LEN1
	lda LEN2
	sbc #$0
	sta LEN2
	lda LEN3
	sbc #$0
	sta LEN3

;ack
	lda #$04
	sta $73

;check for done-ness
	lda LEN3
	ora LEN2
	ora LEN1
	ora LEN0
	beq nowaitpacket
	jmp waitpacket
	
;restore registers
nowaitpacket	pla
	sta $35
	rts
	

