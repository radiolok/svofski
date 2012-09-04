player_y			equ 64+20

player_column:		db 15
player_index:   	db 4

player_ptr:			dw player_bank_ltr_dispatch + 2; player_up_ltr_dispatch+2

PlayerMotion:
	lda input_left
	ora a
	jnz pm_left
	lda input_right
	ora a
	jnz pm_right
	lxi h, player_up_ltr_dispatch+2
	shld player_ptr
	jmp pm_Exit
pm_left:
	lxi h, player_bank_rtl_dispatch
	shld player_ptr
	lxi h, player_index
	mov a, m
	dcr a
	jp pm_left_m1
	lda player_column
	dcr a
	sta player_column
	mvi a, 7
pm_left_m1:
	mov m, a
	jmp pm_Exit

pm_right:
	lxi h, player_bank_ltr_dispatch+2
	shld player_ptr
	lxi h, player_index
	mov a, m
	inr a
	cpi 8
	jm pm_right_m1
	lda player_column
	inr a
	sta player_column
	xra a
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