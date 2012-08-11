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
	call ship_oneframe
	
	jmp jamas

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
	lxi h, ship_x
	mov c, m
	inr m
	lxi h, $8020
	call ship_at_x
	ret
ship_x: db 0


	;; draw sprite at x,y coordinate
	;; hl = base address (y)
	;; c = x (0..255)
ship_at_x:
	; find out column number and add it to de
	; column number = x/8
	mov a, c
	rar 
	rar
	rar
	ani $1f
	mov d, a
	mvi e, 0
	dad d 			; 
	xchg			; de = column base address 
	; offset 0..7
	mov a, c
	ani 7
	mov c, a 		; c = offset

	jmp onesprite

sprites_scratch:	dw 0
onesprite:
	; c = offset
	lxi h, ship_ltr_dispatch
	xra a
	mov b, a
	mov a, c
	ral 
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

sintbl	equ $300 

	mvi c, 0
	lxi d, $8008
	call onesprite
	mvi c, 1
	lxi d, $8018
	call onesprite
	mvi c, 2
	lxi d, $8028
	call onesprite
	mvi c, 3
	lxi d, $8038
	call onesprite
	mvi c, 4
	lxi d, $8048
	call onesprite
	mvi c, 5
	lxi d, $8058
	call onesprite
	mvi c, 6
	lxi d, $8068
	call onesprite
	mvi c, 7
	lxi d, $8078
	call onesprite
	mvi c, 0
	lxi d, $8088
	call onesprite
	mvi c, 1
	lxi d, $8098
	call onesprite

	lxi d, $8508
	call onesprite
	lxi d, $8518
	call onesprite
	lxi d, $8528
	call onesprite
	lxi d, $8538
	call onesprite
	lxi d, $8548
	call onesprite
	lxi d, $8558
	call onesprite
	lxi d, $8568
	call onesprite
	lxi d, $8578
	call onesprite
	lxi d, $8588
	call onesprite
	lxi d, $8598
	call onesprite

	lxi d, $8a08
	call onesprite
	lxi d, $8a18
	call onesprite
	lxi d, $8a28
	call onesprite
	lxi d, $8a38
	call onesprite
	lxi d, $8a48
	call onesprite
	lxi d, $8a58
	call onesprite
	lxi d, $8a68
	call onesprite
	lxi d, $8a78
	call onesprite
	lxi d, $8a88
	call onesprite
	lxi d, $8a98
	call onesprite

	lxi d, $8f08
	call onesprite
	lxi d, $8f18
	call onesprite
	lxi d, $8f28
	call onesprite
	lxi d, $8f38
	call onesprite
	lxi d, $8f48
	call onesprite
	lxi d, $8f58
	call onesprite
	lxi d, $8f68
	call onesprite
	lxi d, $8f78
	call onesprite
	lxi d, $8f88
	call onesprite
	lxi d, $8f98
	call onesprite

	ret
