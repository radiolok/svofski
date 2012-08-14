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
	;out $10
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

	; initial stuff
	ei
	hlt
	call setpalette
	call showlayers

jamas:
	mvi a, 4
	out 2
	ei
	hlt
	xra a
	out 2

	; keep interrupts enabled to make errors obvious
	ei

	lxi h, foe_1
	push h
	call copyfoe_x
	call foe_byId
	pop h
	call copyback_x

	lxi h, foe_2
	push h
	call copyfoe_x
	call foe_byId
	pop h
	call copyback_x

	lxi h, foe_3
	push h
	call copyfoe_x
	call foe_byId
	pop h
	call copyback_x

	lxi h, foe_4
	push h
	call copyfoe_x
	call foe_byId
	pop h
	call copyback_x

	lxi h, foe_5
	push h
	call copyfoe_x
	call foe_byId
	pop h
	call copyback_x

	lxi h, foe_6
	push h
	call copyfoe_x
	call foe_byId
	pop h
	call copyback_x

	lxi h, foe_7
	push h
	call copyfoe_x
	call foe_byId
	pop h
	call copyback_x

	lxi h, frame_number
	inr m
	jmp jamas

frame_number:
	db 0


copyback_x:
	; h = destination
	xchg
	lxi h, 0 	
	dad sp 		
	shld sprites_scratch 
	lxi h, foeBlock
	sphl ; sp -> source
	xchg ; h = dest
	jmp copyfoe_x_0

copyfoe_x:
	xchg		; 8
	lxi h, 0 	; 12
	dad sp 		; 12
	shld sprites_scratch ; 16
	xchg		; 8
	sphl		; 8
	lxi h, foeBlock ; 12
copyfoe_x_0:
	mvi a, 2 	; 8  = 84
copyfoe_x_1:
	pop b       ; 12
	pop d 		; 12
	mov m, c    ; 8
	inx h       ; 8
	mov m, b    ; 8
	inx h 	 	; 8
	mov m, e 	; 8
	inx h       ; 8
	mov m, d    ; 8
	inx h       ; 8
	dcr a       ; 8
	jnz copyfoe_x_1  ; 12  = 64 * 4 = 216
	lhld sprites_scratch  ; 16
	sphl				  ; 8  = 24  --> 84 + 216 + 24 = total 324
	ret

copyfoe:
	lxi d, foeBlock 	; 12
	mvi c, foeSizeOf	; 8
copyfoe_1:	
	mov a, m 			; 8
	stax d  			; 8
	inx h 				; 8
	inx d 				; 8
	dcr c 				; 8
	jnz copyfoe_1 		; 12   = 20 + 52*8 = 436
	ret

copyback:
	lxi d, foeBlock
	mvi c, foeSizeOf
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

	; time marks
	lxi h, $c0c0
	shld $e000
	shld $e010
	shld $e020
	shld $e030
	shld $e040
	shld $e050
	shld $e060
	shld $e070
	shld $e080
	shld $e090
	shld $e0a0
	shld $e0b0
	shld $e0c0
	shld $e0d0
	shld $e0e0
	shld $e0f0
	shld $e0fe
	ret

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
	

FOEID_NONE 		equ 0
FOEID_SHIP 		equ 1
FOEID_COPTER 	equ 2

	;; foe class
foeColumn		equ 0 			; X column
foeIndex		equ 1 			; X offset 0..7
foeDirection	equ 2 			; 1 = LTR, -1 RTL, 0 = not moving
foeBounce		equ 3 			; bounce flag
foeY			equ 4			; Y position of sprite start
foeLeftStop		equ 5			; column # of left limit
foeRightStop	equ 6 			; column # of right limit
foeId 			equ 7 			; id: 0 = none, 1 = ship, 2 = copter
foeSizeOf 		equ 8

	;; foe 0 descriptor
foeBlock:
	db 5,0,1,0,$10,3,15
	db 0 						; id
foeBlock_LTR:	
	dw 0 						; dispatch to sprite LTR 
foeBlock_RTL:
	dw 0 						; dispatch to sprite RTL

foe_1:
	db 5,0,1,0,$10,3,15
	db FOEID_SHIP
foe_2:
	db 5,0,1,0,$20,5,7
	db FOEID_SHIP
foe_3:
	db 5,0,1,0,$30,3,25
	db FOEID_SHIP
foe_4:
	db 7,0,1,0,$40,7,10
	db FOEID_SHIP
foe_5:
	db 5,0,1,0,$50,2,8
	db FOEID_COPTER
foe_6:
	db 5,0,1,0,$60,2,10
	db FOEID_COPTER
foe_7:
	db 5,0,1,0,$70,3,25
	db FOEID_COPTER

foe_byId:
	lda foeBlock + foeId
	; if (foe.Id == 0) return;
	ora a
	rz		

	; prepare dispatches
	dcr a
	jz  foe_byId_ship		; 1 == ship
	dcr a
	jz  foe_byId_copter		; 2 == copter
	; default: return  
	ret
foe_byId_ship:
	lxi h, ship_ltr_dispatch
	shld foeBlock_LTR
	lxi h, ship_rtl_dispatch
	shld foeBlock_RTL
	jmp foe_frame

foe_byId_copter:
	; draw the copter body
	lxi h, copter_ltr_dispatch
	shld foeBlock_LTR
	lxi h, copter_rtl_dispatch
	shld foeBlock_RTL
	call foe_frame
	
	; animate the propeller
	lda frame_number
	ani $2
	jz  foe_byId_propA
	lxi h, propellerB_ltr_dispatch
	lxi d, propellerB_rtl_dispatch
	jmp foe_byId_propC
foe_byId_propA:
	lxi h, propellerA_ltr_dispatch
	lxi d, propellerA_rtl_dispatch
foe_byId_propC:
	shld foeBlock_LTR
	xchg
	shld foeBlock_RTL

	lxi h, foeBlock + foeY
	mov a, m
	push psw
	adi 4
	mov m, a
	push h
	; draw propeller
	call foe_paint_preload
	pop h
	pop psw
	mov m, a
	ret

foe_frame:
foe_Move:
	; load Column to e
	lda foeBlock + foeColumn
	mov e, a 	

	; index = index + direction
	lda foeBlock + foeDirection
	mov b, a
	lda foeBlock + foeIndex
	add b
	; if (Index == -1  
	cpi $ff 
	jz foe_move_L4
	;     || Index == 8)
	cpi $8
	jz foe_move_L4
	jmp foe_move_L1
foe_move_L4:
	; {
	; Index = Index % 8
	ani $7
	; save Index, update c
	sta foeBlock + foeIndex
	mov c, a
	; Column = Column + Direction	
	; column in e
	mov a, e
	add b	
	sta foeBlock + foeColumn
	mov e, a
	; }
	jmp foe_move_CheckBounce
foe_move_L1:
	; save Index, update c
	sta foeBlock + foeIndex
	mov c, a
	; not at column boundary -> skip bounce check
	jmp foe_move_nobounce
foe_move_CheckBounce:
	; check for bounce
	; we are here if Index == 0 || Index == 7
	; e = Column
	lda foeBlock + foeRightStop
	cmp e
	jz foe_move_yes_bounce
	lda foeBlock + foeLeftStop
	cmp e
	jnz foe_move_nobounce
	; yes, bounce
foe_move_yes_bounce:
	; Bounce = 1
	mvi a, 1
	sta foeBlock + foeBounce
	; Direction = -Direction
	mov a, b
	cma 
	inr a
	sta foeBlock + foeDirection
	; do the Move() once again
	jmp foe_Move

	;; additional entry point for sprites with precalculated position
	;; used for propellers
foe_paint_preload:
	lda foeBlock + foeColumn
	mov e, a
	lda foeBlock + foeIndex
	mov c, a
	lda foeBlock + foeDirection
	mov b, a

foe_move_nobounce: 
	;; foeBlock movement calculation ends here

foe_paint:
	;; paint foe
	; e contains column
	mov a, e

	; de = base addr ($8000 + foe.Y)
	adi $80
	mov d, a
	lda foeBlock + foeY
	mov e, a

	; de == base address
	; c == index
	; b == Direction
	mov a, b

	; b = Bounce, Bounce = 0
	lxi h, foeBlock + foeBounce
	mov b, m
	mvi m, 0

	ora a
	jm  sprite_rtl
	jmp sprite_ltr

sprites_scratch:	dw 0

	;; c = offset
	;; de = column base address
	;; b = bounce
sprite_ltr:
	; load dispatch table	
	lhld foeBlock_LTR

	; if (Index != 0) -> regular, no pre-wipe
	mov a, c
	ora a
	jnz sprite_ltr_regular

	; if (Bounce) -> regular, no pre-wipe 
	mov a, b 	
	ora a
	jnz  sprite_ltr_regular

	; Index == 0, no bounce -> wipe previous column
	dcr d
	; jump to _ltr_inf [dispatch - 2]

	jmp sprite_ltr_rtl_dispatchjump

	;; Draw ship without prepending column for wiping
sprite_ltr_regular:
	; index 0..7 -> 1..8
	inr c
	;lxi h, sprite_ltr_dispatch+2
	jmp sprite_ltr_rtl_dispatchjump

	;; c = offset
	;; de = column base address
	;; b = bounce
sprite_rtl:

sprite_rtl_regular:
	lhld foeBlock_RTL

sprite_ltr_rtl_dispatchjump:
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

