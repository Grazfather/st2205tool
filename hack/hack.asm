    CPU 65c02
    OUTPUT HEX
    INCLUDE spec
    * = EMPTY_AT+$4000

;The routine in the existing firmware is patched to jump here if the
;routine that discerns the address that's written to fails.
;This way, we can splice our own check inthere too.

;Watch out & be carefull with bloating this: one of the devices (the Coby)
;only has 230 bytes free to cram this in!

;check magic write to address 4400
start	lda CMP_VAR1
	cmp #$22
	bne nomagic
	lda CMP_VAR2
	cmp #$00
	bne nomagic
	bra gotcha
;Nope? Do what the original routine did & bail out.
nomagic lda #$ff
	ldx #$ff
	jmp PATCH_AT+$4004

;ack usb wossname
gotcha  lda #$04
	sta $73

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
	beq blon
	cmp #$3
	beq bloff
	bra packetend

blon	lda $03
	and #($ff-$04)
	sta $03
	bra packetend

bloff	lda $03
	ora #$04
	sta $03
	bra packetend


IF CTRTYPE=1 ;UC1697V
;set visible window
setaddr	lda #$F4
	sta $8000
	lda $201
	sta $8000

	lda #$F5
	sta $8000
	lda $203
	sta $8000

	lda #$F6
	sta $8000
	lda $202
	sta $8000

	lda #$F7
	sta $8000
	lda $204
	sta $8000

;reset addr to (0,0)
	lda #$00
	sta $8000
	lda #$10
	sta $8000
	lda #$60
	sta $8000
	lda #$70
	sta $8000
	
;	lda #$AF
;	sta $8000

	bra packetend
ENDC
IF CTRTYPE=0 ;PCF8833
;set addr
setaddr	lda #$2A
	sta $8000
	lda $201
	sta $c000
	lda $202
	sta $c000

	lda #$2B
	sta $8000
	lda $203
	sta $c000
	lda $204
	sta $c000

	lda #$2c
	sta $8000

	bra packetend
ENDC

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


;send ack
	lda #$00
	jsr SEND_CSW+0x4000

;and return as a winner :)
	lda #$ff
	ldx #$ff
	jmp PATCH_AT+$4004


	db "H","4","C","K"
	db 1 ;version of info block
	db CONF_XRES
	db CONF_YRES
	db CONF_BPP
	db CONF_PROTO	
	db OFFX
	db OFFY
