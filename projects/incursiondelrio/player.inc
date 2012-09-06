player_y			equ 64+20

player_column:		db 15
player_index:   	db 4

player_dir:			db 0

player_ptr:			dw player_bank_ltr_dispatch + 2; player_up_ltr_dispatch+2

playerXspeed:		db 0
playerXaccu:		db 255	

playerJoystickUsed:	db 0


PlayerMotion:
	mvi d, $ff
	lda input_left
	ora a
	jnz pm_input_leftright
	mvi d, 1
	lda input_right
	ora a
	jnz pm_input_leftright
	
	; joystick left/right released, clear X speed
	mvi a, 0
	sta playerXspeed
	dcr a
	sta playerXaccu
	
	; set the plane straight
	lxi h, player_up_ltr_dispatch+2
	shld player_ptr
	jmp pm_Exit

pm_input_leftright:
	lda playerXspeed
	cpi $80 			; X speed maxed out?
	jz pm_x_accumulate	; yes, no change
	adi $4 				; increase speed
	sta playerXspeed
pm_x_accumulate:
	mov b, a
	mvi c, 0
	lda playerXaccu
	add b
	jnc pm_ilr_1
	dcr c
pm_ilr_1:
	add b
	sta playerXaccu
	jc  pm_playerXstep
	mov a, c
	ora a
	jm  pm_playerXstep
	jmp pm_Exit

	; perform X step, d = direction
pm_playerXstep:
	mov a, d
	ora a
	jm pm_left
	jmp pm_right

pm_left:
	lxi h, player_bank_rtl_dispatch
	shld player_ptr

	lxi h, player_index
	mov a, m
	sui 2 				; move in steps of 2
	jp pm_left_m1
	adi 8
	mov m, a
	lda player_column
	dcr a
	sta player_column
	jmp pm_Exit
pm_left_m1:
	mov m, a
	jmp pm_Exit

pm_right:
	lxi h, player_bank_ltr_dispatch+2
	shld player_ptr

	lxi h, player_index
	mov a, m
	adi 2 				; move in steps of 2
	cpi 8
	jm pm_right_m1
	sui 8
	mov m, a
	lda player_column
	inr a
	sta player_column
	jmp pm_Exit
pm_right_m1:
	mov m, a
pm_Exit:
	ret

PlayerSprite:
	lxi h, $8000
	shld psw_layer+1
	call PlayerSprite_W
	lxi h, $a000
	shld psw_layer+1
	call PlayerSprite_W
	lxi h, $c000
	shld psx_layer+1
	call PlayerSprite_X
	ret

PlayerWipe:
	lxi h, $c000
	shld psw_layer+1
	call PlayerSprite_W
	ret


PlayerSprite_W:

	; hl -> player_ptr
	lhld player_ptr
	mvi d, 0
	lda player_index
	ora a
	ral 
	mov e, a
	dad d
	; hl -> player_ptr[player_index]

	mov e, m
	inx h
	mov d, m
	; de -> &bits

	lxi h, 0
	dad sp 
	shld psw_restoreSP+1

psw_layer:
	lxi h, $c000
	lda player_column
	add h
	mov h, a

	mvi b, player_y
	lda frame_scroll
	add b
	mov l, a
	; hl -> screen location

	xchg	
	sphl
	xchg

	mov e, l

	;; le big pop
	pop b
	mov a, b
	cma 
	ana m
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	mov m, a

	inr h
	mov l, e
	; column end

	;; le big pop
	pop b
	mov a, b
	cma 
	ana m
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	mov m, a

	inr h
	mov l, e
	; column end
	;; le big pop
	pop b
	mov a, b
	cma 
	ana m
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	mov m, a

	inr h
	mov l, e
	; column end


psw_restoreSP:
	lxi sp, 0
	ret


PlayerSprite_X:

	; hl -> player_ptr
	lhld player_ptr
	mvi d, 0
	lda player_index
	ora a
	ral 
	mov e, a
	dad d
	; hl -> player_ptr[player_index]

	mov e, m
	inx h
	mov d, m
	; de -> &bits

	lxi h, 0
	dad sp 
	shld psx_restoreSP+1

psx_layer:
	lxi h, $c000
	lda player_column
	add h
	mov h, a

	mvi b, player_y
	lda frame_scroll
	add b
	mov l, a
	; hl -> screen location

	xchg	
	sphl
	xchg

	mov e, l

	;; le big pop
	pop b
	mov a, b
	cma 
	ana m
	ora b
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	ora c
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	ora b
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	ora c
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	ora b
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	ora c
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	ora b
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	ora c
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	ora b
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	ora c
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	ora b
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	ora c
	mov m, a

	inr h
	mov l, e
	; column end

	;; le big pop
	pop b
	mov a, b
	cma 
	ana m
	ora b
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	ora c
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	ora b
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	ora c
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	ora b
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	ora c
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	ora b
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	ora c
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	ora b
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	ora c
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	ora b
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	ora c
	mov m, a

	inr h
	mov l, e
	; column end
	;; le big pop
	pop b
	mov a, b
	cma 
	ana m
	ora b
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	ora c
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	ora b
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	ora c
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	ora b
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	ora c
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	ora b
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	ora c
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	ora b
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	ora c
	mov m, a
	dcr l
	;
	pop b
	mov a, b
	cma 
	ana m
	ora b
	mov m, a
	dcr l
	mov a, c
	cma
	ana m
	ora c
	mov m, a

	inr h
	mov l, e

	; column end


psx_restoreSP:
	lxi sp, 0
	ret