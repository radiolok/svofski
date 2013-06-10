    org 100h
    ld c, 9
    ld de, msg
    call 5

    ld hl, 0

dumploop:
    ld a, l
    and 0fh
    call z, displayRest

    ld c, (hl)
    call space 
    call OutHex8
    call checkkey

    inc l
    jr z, dl_nexth
    jr dumploop
dl_nexth:
    inc h
    jr dumploop
    ret

displayRest:
	; chars
	call space
	
	ld a, l
	sub 10h
	ld l, a
nextchar:
	ld a, (hl)
	cp 20h
	jr nc, charok
	ld a, '.'
charok:	
	call putchar
	inc l
	ld a, l
	and 0fh
	jr nz, nextchar

nextline:
	call nl
	call DispHLhex
	ret

nl:
	ld a, 0dh
	call putchar
	ld a, 0ah
	call putchar
	ret

checkkey:
	push hl
	ld c, 0bh
	call 5
	or a
	jr z, checkkeyret

	ld c, 1
	call 5
	ld c, 1
	call 5
	ld c, 1
	call 5
checkkeyret:
	pop hl
	ret

space:
	ld a, ' '
	call putchar
	ret

putchar:
	push bc
	push de
	push hl
	ld c, 2
	ld e, a
	call 5
	pop hl
	pop de
	pop bc
	ret


;Display a 16- or 8-bit number in hex.
DispHLhex:
; Input: HL
   ld  c,h
   call  OutHex8
   ld  c,l
OutHex8:
; Input: C
   ld  a,c
   rra
   rra
   rra
   rra
   call  Conv
   ld  a,c
Conv:
   and  $0F
   add  a,$90
   daa
   adc  a,$40
   daa
   call putchar
   ret


msg:    db 'TPA program launched', 0dh, 0ah, '$'
