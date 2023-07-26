    CPU 65c02
    OUTPUT HEX
    * = $7A00

;The routine in the existing firmware is patched to jump here if the
;routine that discerns the address that's written to fails.
;This way, we can splice our own check inthere too.

;check magic write to address 4200
start	lda $388
	cmp #$21
	bne nomagic
	lda $0389
	cmp #$00
	bne nomagic
	bra gotcha
;Nope? Do what the original routine did & bail out.
nomagic lda #$ff
	ldx #$ff
	jmp $06de1
;ack usb wossname
gotcha  lda #$04
	sta $73

;Push registers	
	lda $34
	pha
	lda $35
	pha
	
;The LCD of this device isn't connected to the perfectly good internal LCD
;subsystem, but is an external one with its own controller. It seems to be
;connected to the dataspace when DRRH==3. Commands to it go to $8000, data
;to $c000. The controller is PCF8833-compatible btw, same type as some
;Nokia 6100s (and other Nokias) have.

;select lcd
	lda #$3
	sta $35

;reset addr
	lda #$2b
	sta $8000
	lda #$4
	sta $c000
	lda #$83
	sta $c000

	lda #$2a
	sta $8000
	lda #$4
	sta $c000
	lda #$83
	sta $c000

;go store data!
	lda #$2c
	sta $8000


	stz $37A
;wait for usb packet
waitpacket lda $73
	and #$4
	beq waitpacket

;copy packet to framebuff
	ldx #$0
copyloop lda $200,x
	sta $c000
	inx
	;cpx #$40 ;wrongly assembled by crasm, I'll do it manually:
	db $e0,$40
	bne copyloop


;subtract 1 from 37B:37D.
;Damn, this is way easier on an ARM :P
	sec
	lda $37a
	sbc #$40
	sta $37a
	lda $37b
	sbc #$0
	sta $37b
	lda $37c
	sbc #$0
	sta $37c
	lda $37d
	sbc #$0
	sta $37d

;ack
	lda #$04
	sta $73

;check for done-ness
	lda $37d
	ora $37c
	ora $37b
	ora $37a
	bne waitpacket
		
;restore registers
	pla
	sta $35
	pla
	sta $34

;send ack
	lda #$00
	jsr $6ca5

;and return as a winner :)
	lda #$ff
	ldx #$ff
	jmp $6de1
