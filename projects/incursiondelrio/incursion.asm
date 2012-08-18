    ; i8080 assembler code

    .binfile asaltodelrio.rom

    .nodump

stacksave	equ $20

	 
	.org $100

clrscr:
    di
	lxi h, $8000
	; good for seeing sprite scratch area 
	; mvi b, $ff
	mvi b, 0
clearscreen:
	xra a
	mov m, b
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
	;call showlayers

jamas:
	mvi a, 4
	out 2
	ei
	hlt
	xra a
	out 2

	; scroll
	mvi a, 88h
	out 0
	lda frame_scroll
	out 3

	; keep interrupts enabled to make errors obvious
	ei

	; prepare animated sprites
	call foe_animate

	lxi d, foe_1
	call foe_in_de
	lxi d, foe_2
	call foe_in_de
	lxi d, foe_3
	call foe_in_de
	lxi d, foe_4
	call foe_in_de
	lxi d, foe_5
	call foe_in_de
	lxi d, foe_6
	call foe_in_de
	lxi d, foe_7
	call foe_in_de
	lxi d, foe_8
	call foe_in_de

	call clearblinds
	call produce_line
	call drawblinds_bottom

	lxi h, frame_number
	inr m

	lxi h, frame_scroll
	inr m

	;di
	;jmp .

	jmp jamas

frame_number:
	db 0
frame_scroll:
	db $ff

;;
;; Process foe with descriptor in HL
;;
foe_in_de:
	push d

copyfoe_y:
	lxi h, 0 				; 12
	dad sp 					; 12
	shld copyfoe_restoresp+1	; 16
	xchg					; 8
	sphl					; 8  = 64

	pop h       			; 12
	shld foeBlock 			; 16
	pop h       			; 12
	shld foeBlock + 2 		; 16
	pop h       			; 12
	shld foeBlock + 4 		; 16
	pop h       			; 12
	shld foeBlock + 6 		; 16  = 68
copyfoe_restoresp:
	lxi sp, 0

	call foe_byId
	pop d

	; only the first 4 bytes of foeBlock need to be copied back
copyback_y:
	lxi h, 0 	
	dad sp 		
	shld copyback_return+1 
	xchg		
	lxi d, 4 				; foeBlock size
	dad d
	sphl		

	;lhld foeBlock + 6 		
	;push h
	;lhld foeBlock + 4
	;push h
	lhld foeBlock + 2
	push h
	lhld foeBlock
	push h           

copyback_return:
	lxi sp, 0
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

;------------------------------------------------------
; Draw 2 lines of black at the bottom of game field
; Clear them at the top, leave 16+16 of black at sides
;------------------------------------------------------
drawblinds_bottom:
	lxi h, $e000 + 70
	mvi b, $ff
	mvi c, 16 ; 32
	jmp clearblinds_entry2

clearblinds:
	lxi h, $e2ff-16
	mvi b, 0
	mvi c, 14 ; 28
clearblinds_entry2:
	lda frame_scroll
	add l
	mov l, a

	; fill in a meander-like pattern y,y+1,x+1,y+1
clearblinds_L1:
	mov m, b 
	inr l    
	mov m, b 
	inr h
	mov m, b
	dcr l
	mov m, b
	inr h
	dcr c 	
	jnz clearblinds_L1 
	ret	

line_left:	db 4
line_fieldA: db 8
line_island: db 8
line_fieldB: db 8

produce_line:
	lxi h, $80ff-16

	lda frame_scroll
	add l
	mov l, a

	lxi b, $ff00
	lda line_left
produce_loop_leftbank:
	mov m, b
	inr h
	dcr a
	jnz produce_loop_leftbank

	lda line_fieldA
produce_loop_leftwater:
	mov m, c
	inr h
	dcr a
	jnz produce_loop_leftwater

	lda line_island
produce_loop_island:
	mov m, b
	inr h
	dcr a
	jnz produce_loop_island

	lda line_fieldB
produce_loop_rightwater:
	mov m, c
	inr h
	dcr a
	jnz produce_loop_rightwater

	mvi a, $80+32
	sub h
produce_loop_rightbank:
	mov m, b
	inr h
	dcr a
	jnz produce_loop_rightbank

	ret

FOEID_NONE 		equ 0
FOEID_SHIP 		equ 1
FOEID_COPTER 	equ 2
FOEID_RCOPTER	equ 3
FOEID_JET 		equ 4

	;; foe class
foeId 			equ 0 			; id: 0 = none, 1 = ship, 2 = copter
foeColumn		equ 1 			; X column
foeIndex		equ 2 			; X offset 0..7
foeDirection	equ 3 			; 1 = LTR, -1 RTL, 0 = not moving
foeY			equ 4			; Y position of sprite start
foeLeftStop		equ 5			; column # of left limit
foeRightStop	equ 6 			; column # of right limit
foeBounce		equ 7 			; bounce flag
foeSizeOf 		equ 8

	;; foe 0 descriptor
foeBlock:
	db 5,0,1,0,$10,3,15
	db 0 						; id
foeBlock_LTR:	
	dw 0 						; dispatch to sprite LTR 
foeBlock_RTL:
	dw 0 						; dispatch to sprite RTL

foePropeller_LTR:
	dw 0
foePropeller_RTL:
	dw 0

foe_1:
	db FOEID_SHIP
	db 5,0,1,$10,	3,8,0
foe_2:
	db FOEID_COPTER
	db 6,0,1,$30,	3,10,0
foe_3:
	db FOEID_JET
	db 5,0,$ff,$50,	3,25,0
foe_4:
	db FOEID_RCOPTER
	db 8,0,1,$70,	3,10,0
foe_5:
	db FOEID_SHIP
	db 19,0,1,$90,	19,24,0
foe_6:
	db FOEID_JET
	db 5,0,1,$b0,	0,0,0
foe_7:
	db FOEID_SHIP
	db 21,0,1,$d0,	19,24,0
foe_8:
	db FOEID_COPTER
	db 9,0,1,$f0,	3,10,0

	;; animate sprites
	;; should be called once per frame, before the first sprite
foe_animate:
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
	shld foePropeller_LTR
	xchg
	shld foePropeller_RTL
	ret

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
	dcr a
	jz  foe_byId_rcopter	; 3 == redcopter
	dcr a
	jz 	foe_byId_jet 		; 4 == jet
	; default: return  
	ret
foe_byId_ship:
	lxi h, ship_ltr_dispatch
	shld foeBlock_LTR
	lxi h, ship_rtl_dispatch
	shld foeBlock_RTL
	jmp foe_frame

foe_byId_jet:
	lxi h, jet_ltr_dispatch
	shld foeBlock_LTR
	lxi h, jet_rtl_dispatch
	shld foeBlock_RTL
	jmp jet_frame 			; jet has a separate handler

foe_byId_rcopter:
	; draw the copter body
	lxi h, redcopter_ltr_dispatch
	shld foeBlock_LTR
	lxi h, redcopter_rtl_dispatch
	shld foeBlock_RTL
	call foe_frame
	jmp foe_byId_copterpropeller
foe_byId_copter:
	; draw the copter body
	lxi h, copter_ltr_dispatch
	shld foeBlock_LTR
	lxi h, copter_rtl_dispatch
	shld foeBlock_RTL
	call foe_frame
foe_byId_copterpropeller:
	; load animated propeller
	lhld foePropeller_LTR
	shld foeBlock_LTR
	lhld foePropeller_RTL
	shld foeBlock_RTL

	; offset propeller Y position
	lda foeBlock + foeY ; 16
	adi 4 				; 8
	sta foeBlock + foeY ; 16
	push psw 			; 12
	; draw propeller
	call foe_paint_preload 
	; restore copter Y position in the foe block
	pop psw 			; 12
	sta foeBlock + foeY ; 16 = 64
	ret


;; --------------------------
;; Jet frame
;; --------------------------
jet_frame:
	; load Column to e

	lhld foeBlock + foeColumn
	mov e, l ; e = foeColumn
	mov b, h ; b = foeIndex
	mvi h, 0 ; bounce = 0

	lda foeBlock + foeDirection
	mov c, a   ; keep direction in c
	ral
	mvi a, 4
	jnc jet_move_diradd
	mvi a, $fc ; -4
jet_move_diradd:
	add b
	mov b, c   ; restore direction in b
	; if (Index < 0  
	ora a
	jm jet_move_indexoverrun
	;     || Index == 8)
	cpi $8
	jz jet_move_indexoverrun

	; for right screen limit 
	; a here can be only 0 or 4 (we're looking for 4)
	; so check if a + 30 == 34
	mov c, a
	add e
	cpi 34
	jnz jet_move_normal
	xra a
	jmp jet_move_reset_to_a

jet_move_normal:
	; index within column boundary
	; save Index, update c
	mov a, c
	sta foeBlock + foeIndex
	; all done, paint
	jmp foe_paint

jet_move_indexoverrun:
	; {
	; Index = Index % 8
	ani $7
	; save Index, update c
	sta foeBlock + foeIndex
	mov c, a
	; Column = Column + Direction 
	; column in e
	mov a, b
	add e
	sta foeBlock + foeColumn
	mov e, a
	; }
jet_move_checklimits:
	; check for left screen limit
	; we are here if Index == 0 || Index == 7
	; e = Column
	ora a 		; if (a >= 0) 
	mvi a, 29
	jp jet_move_continue ; -->
jet_move_reset_to_a:
	sta foeBlock + foeColumn
	mov e, a
jet_move_continue:
	jmp foe_paint

;; ----------------------------------------------
;; Frame routine for a regular foe: ship, copters
;; ----------------------------------------------
foe_frame:
	mvi h, 0 ; bounce flag in h
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
	ora a
	jm foe_move_indexoverrun
	;     || Index == 8)
	cpi $8
	jz foe_move_indexoverrun

	; index within column boundary
	; save Index, update c
	sta foeBlock + foeIndex
	mov c, a
	; not at column boundary -> skip bounce check
	jmp foe_paint

foe_move_indexoverrun:
	; {
	; Index = Index % 8
	ani $7
	; save Index, update c
	sta foeBlock + foeIndex
	mov c, a
	; Column = Column + Direction
	; column in e
	mov a, b
	add e
	sta foeBlock + foeColumn
	mov e, a
	; }
foe_move_CheckBounce:
	; check for bounce
	; we are here if Index == 0 || Index == 7
	; e = Column
	lda foeBlock + foeRightStop
	cmp e
	jz foe_move_yes_bounce
	lda foeBlock + foeLeftStop
	cmp e
	jnz foe_paint
	; yes, bounce
foe_move_yes_bounce:
	; Bounce = 1
	mvi h, 1 
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

	lhld foeBlock + foeColumn
	mov e, l ; e = foeColumn
	mov c, h ; c = foeIndex
	mvi h, 0 ; bounce = 0
	lda foeBlock + foeDirection
	mov b, a
	mvi h, 1 ; 
	;; foeBlock movement calculation ends here

	;; actual paint
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
	xra a
	ora b
	jm  sprite_rtl
	; fallthrough to sprite_ltr

	;; c = Index
	;; de = column base address
	;; h = bounce
sprite_ltr:
	; if (index != 0 || Bounce) regular();
	xra a
	ora c
	ora h
	jnz sprite_ltr_regular

	; Index == 0, no bounce -> wipe previous column
	dcr d
	; load dispatch table	
	lhld foeBlock_LTR
	jmp sprite_ltr_rtl_dispatchjump

	;; Draw ship without prepending column for wiping
sprite_ltr_regular:
	; load dispatch table	
	lhld foeBlock_LTR
	; index 0..7 -> 1..8
	inr c
	jmp sprite_ltr_rtl_dispatchjump

	;; c = offset
	;; de = column base address
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

sprites_scratch:	dw 0 	; saved SP for various stack-abusing routines

    .include ship.inc

c_black		equ $00
c_blue		equ $c1
c_green 	equ $73 ; 01 110 011
c_yellow 	equ $bf ; 
c_magenta 	equ $8d ;
c_white		equ $f6
c_grey		equ $09 ; 00 010 010
c_cyan		equ $f4	; 10 011 001
c_dkblue	equ $81	; 10 010 001

palette_data:
	db c_blue,   	c_black,  c_white,  	c_black
	db c_magenta, 	c_black,  c_grey,		c_black
	db c_green, 	c_black,  c_cyan,  		c_black
	db c_yellow,    c_black,  c_dkblue, 	c_black


	;; Depuración y basura

	; pintar los colores
showlayers:
	lxi h, $ffff
	; 1 0 0 0
	shld $81fe 

	; 0 1 0 0 
	shld $a2fe

	; 1 1 0 0
	shld $83fe
	shld $a3fe

	; 0 0 1 0
	shld $c4fe

	; 1 0 1 0
	shld $85fe	
	shld $c5fe

	; 0 1 1 0
	shld $a6fe	
	shld $c6fe

	; 1 1 1 0
	shld $87fe	
	shld $a7fe
	shld $c7fe

	; x x x 1
	shld $e8fe	

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

