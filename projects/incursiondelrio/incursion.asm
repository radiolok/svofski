    ; i8080 assembler code

    .binfile asaltodelrio.rom

    .nodump

stacksave	equ $20

	 
	.org $100

clrscr:
    di
	lxi h, $8000
clearscreen:
	xra a
	out $10
	mov m, a
	inx h
	ora h
	jnz clearscreen

	; init stack pointer
	lxi sp, $100

	; enable interrupts

	; write ret to rst 7 vector	
	mvi a, $c9
	sta $38
jamas:
	ei
	hlt
	; write death to rst 7 vector
	;;mvi a, $c3
	;;sta $38
	;;lxi h, $38
	;;shld $39
	ei
	; do stuff
	
	call setpalette
	call showlayers

	lxi h, foe_1
	push h
	call copyfoe
	call foe_0_frame
	pop h
	call copyback

	lxi h, foe_2
	push h
	call copyfoe
	call foe_0_frame
	pop h
	call copyback

	lxi h, foe_3
	push h
	call copyfoe
	call foe_0_frame
	pop h
	call copyback

	lxi h, foe_4
	push h
	call copyfoe
	call foe_0_frame
	pop h
	call copyback

	lxi h, foe_5
	push h
	call copyfoe
	call foe_0_frame
	pop h
	call copyback

	jmp jamas
copyfoe:
	lxi d, foe_0
	mvi c, 8
copyfoe_1:	
	mov a, m
	stax d
	inx h
	inx d
	dcr c
	jnz copyfoe_1
	ret

copyback:
	lxi d, foe_0
	mvi c, 8
copyback_1:
	ldax d
	mov m, a
	inx h
	inx d
	dcr c
	jnz copyback_1
	ret

	; pintar los colores
showlayers:
	lxi h, $ffff
	; verde		1 0 0 0
	shld $81fe 
	shld $81fc 
	; amarillo	0 0 1 0 
	shld $c2fe
	shld $c2fc
	; negro		0 1 0 0
	shld $a3fe
	shld $a3fc
	; rosa		0 1 1 0
	shld $a4fe	
	shld $c4fe
	shld $a4fc	
	shld $c4fc
	; blanco	0 0 0 1
	shld $e5fe	
	shld $e5fc

	; test bounds
	shld $8306
	shld $8308
	shld $830a
	shld $830c
	shld $830e
	shld $8310
	shld $8312

	shld $9306
	shld $9308
	shld $930a
	shld $930c
	shld $930e
	shld $9310
	shld $9312

setpalette:
	lxi h, palette_data+15
	mvi c, 16
palette_loop:
	mov a, c
	dcr a
	out $2
	mov a, m
	out $c
	dcr c
	dcx h
	jnz palette_loop
	ret
	

ship_oneframe:
;	lxi h, ship_x
;	mov c, m
;	inr m

;	lxi h, $80f0
;	lxi h, ship_y
;	mov a, m
;	mov b, a
;	sui 8
;	mov m, a
;	mvi h, $80
;	mov l, b
;	call ship_at_x
;	ret 
	

ship_x: db 0
ship_y: db $f0

	;; foe class
foeColumn		equ 0 			; X column
foeIndex		equ 1 			; X offset 0..7
foeDirection	equ 2 			; 1 = LTR, -1 RTL, 0 = not moving
foeBounce		equ 3 			; bounce flag
foeY			equ 4			; Y position of sprite start
foeLeftStop		equ 5			; column # of left limit
foeRightStop	equ 6 			; column # of right limit
;--
foeWidth		equ 7 			; width in columns
foeHeight		equ 8 			; sprite height
foeSizeOf		equ 9

	;; foe 0 descriptor
foe_0:
	db 5,0,1,0,$10,3,15,  4,8
foe_1:
	db 5,0,1,0,$10,3,15,  4,8
foe_2:
	db 5,0,1,0,$20,5,7,  4,8
foe_3:
	db 5,0,1,0,$30,3,25,  4,8
foe_4:
	db 7,0,1,0,$40,7,10,  4,8
foe_5:
	db 5,0,1,0,$50,2,8,  4,8

foe_0_frame:
foe_0_Move:
	; load Column to e
	lda foe_0 + foeColumn
	mov e, a 	

	; index = index + direction
	lda foe_0 + foeDirection
	mov b, a
	lda foe_0 + foeIndex
	add b
	; if (Index == -1  
	cpi $ff 
	jz foe_0_move_L4
	;     || Index == 8)
	cpi $8
	jz foe_0_move_L4
	jmp foe_0_move_L1
foe_0_move_L4:
	; {
	; Index = Index % 8
	ani $7
	; save Index, update c
	sta foe_0 + foeIndex
	mov c, a
	; Column = Column + Direction	
	; column in e
	mov a, e
	add b	
	sta foe_0 + foeColumn
	mov e, a
	; }
	jmp foe_0_move_CheckBounce
foe_0_move_L1:
	; save Index, update c
	sta foe_0 + foeIndex
	mov c, a
	; not at column boundary -> skip bounce check
	jmp foe_0_move_nobounce
foe_0_move_CheckBounce:
	; check for bounce
	; we are here if Index == 0 || Index == 7
	; e = Column
	lda foe_0 + foeRightStop
	cmp e
	jz foe_0_move_yes_bounce
	lda foe_0 + foeLeftStop
	cmp e
	jnz foe_0_move_nobounce
	; yes, bounce
foe_0_move_yes_bounce:
	; Bounce = 1
	mvi a, 1
	sta foe_0 + foeBounce
	; Direction = -Direction
	mov a, b
	cma 
	inr a
	sta foe_0 + foeDirection
	; do the Move() once again
	jmp foe_0_Move

foe_0_move_nobounce: 
	;; foe_0 movement calculation ends here

	;; paint foe_0 

	; de = base addr ($8000 + foe.Y)
	; e contains column
	mov a, e
	adi $80
	mov d, a
	lda foe_0 + foeY
	mov e, a

	; de == base address
	; c == index
	; b == Direction
	mov a, b

	; b = Bounce, Bounce = 0
	lxi h, foe_0 + foeBounce
	mov b, m
	mvi m, 0

	ora a
	jm  ship_rtl
	jmp ship_ltr

sprites_scratch:	dw 0

ship_ltr:
	; c = offset
	; de = column base address
	; b = bounce
	
	; if (Index != 0) -> regular, no pre-wipe
	mov a, c
	ora a
	jnz ship_ltr_regular

	; if (Bounce) -> regular, no pre-wipe 
	mov a, b 	
	ora a
	jnz  ship_ltr_regular

	; Index == 0, no bounce -> wipe previous column
	dcr d
	jmp ship_ltr_inf

	;; Draw ship without prepending column for wiping
ship_ltr_regular:
	lxi h, ship_ltr_dispatch+2
	jmp ship_ltr_rtl_dispatchjump

ship_rtl:
	; c = offset
	; de = column base address
	; b = bounce

	jmp ship_rtl_regular
	; if (Index != 7) -> regular, no wipe
	mov a, c
	;cpi 7
	;jnz ship_rtl_regular
	ora a
	jz ship_rtl_sup
	jmp ship_rtl_regular

	; if (Bounce) -> regular, no wipe 
	mov a, b
	ora a
	jnz  ship_rtl_regular

	; Index == 7, no bounce -> draw and wipe the column to the right
	jmp ship_rtl_sup

ship_rtl_regular:
	lxi h, ship_rtl_dispatch
	;jmp ship_ltr_rtl_dispatchjump

ship_ltr_rtl_dispatchjump:
	mvi b, 0
	mov a, c
	rlc
	mov c, a
	dad b 		
	;jmp [h]
	shld .+4
	lhld 0000	
	pchl

    .include ship.inc


zero16:	dw 0

c_black		equ $00
c_blue		equ $c0
c_green 	equ $38
c_yellow 	equ $3f
c_magenta	equ $c7
c_white		equ $ff

palette_data:
	db c_blue,  c_white,  c_yellow,  c_white
	db c_black, c_white,  c_magenta, c_white
	db c_green, c_white,  c_yellow,  c_white
	db c_black, c_white,  c_magenta, c_white

dactr:	equ .

