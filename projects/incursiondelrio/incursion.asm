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
	call foe_0_frame
	
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
foeWidth		equ 4 			; width in columns
foeHeight		equ 5 			; sprite height
foeY			equ 6			; Y position of sprite start
foeLeftStop		equ 7			; column # of left limit
foeRightStop	equ 8 			; column # of right limit
foeSizeOf		equ 9

	;; foe 0 descriptor
foe_0:
	;db 3,0,1,0,0,0,20,0,30 		
	db 3,0,1,0,0,0,20,3,15 		
	;; foe 1 descriptor
foe_1:
	db 0,0,0,0,0,0,0,0,0 		
	;; foe 2 descriptor
foe_2:
	db 0,0,0,0,0,0,0,0,0 		
	;; foe 3 descriptor
foe_3:
	db 0,0,0,0,0,0,0,0,0 		

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
	jmp foe_0_move_L3
foe_0_move_CheckBounce:
	; check for bounce
	; we are here if Index == 0 || Index == 7
	; e = Column
	lda foe_0 + foeRightStop
	cmp e
	jz foe_0_move_yes_bounce
	lda foe_0 + foeLeftStop
	cmp e
	jnz foe_0_move_L3
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
foe_0_move_L3: ; no bounce
	;; foe_0 movement calculation ends here

	;; paint foe_0 

	; de = base addr ($8000 + foe.Y)
	; e already contains column
	mov a, e
	adi $80
	mov d, a
	lda foe_0 + foeY
	mov e, a

	; c already contains index
	jmp onesprite

	;; draw sprite at x,y coordinate
	;; hl = base address (y)
	;; c = x (0..255)
ship_at_x:
	; find out column number and add it to de
	; column number = x/8
	; offset 0..7
	mov b, c
	mov a, c
	ani 7
	mov c, a 		; c = offset
	mov a, b
	rar 
	rar
	rar
	ani $1f
inf: nop
	mov d, a
	mvi e, 0
	dad d 			; 
	xchg			; de = column base address 


	jmp onesprite

sprites_scratch:	dw 0
onesprite:
	; c = offset
	; de = column base address
	mov a, c
	ora a
	jnz onesprite_regular

;	lda ship_bounce
;	ora a
;	jnz onesprite_regular

	dcr d
	dcr d
	jmp ship_ltr_inf

onesprite_regular:
	lxi h, ship_ltr_dispatch+2
onesprite_1:
	mvi b, 0
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
