missile_tab:
	db 1, 10000000b
	db 0, 11000000b
	db 0, 01100000b
	db 0, 00110000b
	db 0, 00011000b
	db 0, 00001100b
	db 0, 00000110b
	db 0, 00000011b

missile_y:	db	96

MissileSprite:
	lxi h, $c000
	lda missile_y
	mov l, a
	lda frame_scroll
	add l
	mov l, a
	lda player_column
	add h
	mov h, a

	xchg

	lxi h, missile_tab
	lda player_index
	ora a
	ral
	mov c, a
	mvi b, 0
	dad b
	mov b, m
	inx h
	mov c, m

	xchg
	lxi d, 0

	mov a, b
	cma 
	mov e, m
	ana e
	cmp e
	jz .+4
	inr d
	ora b
	mov m, a
	inr h

	mov a, c
	cma 
	mov e, m
	ana e
	cmp e
	jz .+4
	inr d
	ora c
	mov m, a
	dcr l

	mov a, c
	cma 
	mov e, m
	ana e
	cmp e
	jz .+4
	inr d
	ora c
	mov m, a
	dcr h

	mov a, b
	cma 
	mov e, m
	ana e
	cmp e
	jz .+4
	inr d
	ora b
	mov m, a
	dcr l

	mov a, b
	cma 
	mov e, m
	ana e
	cmp e
	jz .+4
	inr d
	ora b
	mov m, a
	inr h

	mov a, c
	cma 
	mov e, m
	ana e
	cmp e
	jz .+4
	inr d
	ora c
	mov m, a
	dcr l

	mov a, c
	cma 
	mov e, m
	ana e
	cmp e
	jz .+4
	inr d
	ora c
	mov m, a
	dcr h

	mov a, b
	cma 
	mov e, m
	ana e
	cmp e
	jz .+4
	inr d
	ora b
	mov m, a
	dcr l

	mov a, b
	cma 
	mov e, m
	ana e
	cmp e
	jz .+4
	inr d
	ora b
	mov m, a
	inr h

	mov a, c
	cma 
	mov e, m
	ana e
	cmp e
	jz .+4
	inr d
	ora c
	mov m, a
	dcr l

	mov a, c
	cma 
	mov e, m
	ana e
	cmp e
	jz .+4
	inr d
	ora c
	mov m, a
	dcr h

	mov a, b
	cma 
	mov e, m
	ana e
	cmp e
	jz .+4
	inr d
	ora b
	mov m, a
	dcr l

	mov a, b
	cma 
	mov e, m
	ana e
	cmp e
	jz .+4
	inr d
	ora b
	mov m, a
	inr h

	mov a, c
	cma 
	mov e, m
	ana e
	cmp e
	jz .+4
	inr d
	ora c
	mov m, a
	dcr l

	mov a, c
	cma 
	mov e, m
	ana e
	cmp e
	jz .+4
	inr d
	ora c
	mov m, a
	dcr h

	mov a, b
	cma 
	mov e, m
	ana e
	cmp e
	jz .+4
	inr d
	ora b
	mov m, a
	dcr l


	ret