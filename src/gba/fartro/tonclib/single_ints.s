@
@ Interrupt switch block
@ Taken from wintermute's libgba (www.devkit.tk)
@
@---------------------------------------------------------------------------------
	.section	.iwram,"ax",%progbits
	.extern	IntrTable
	.code 32

	.global	IntrMain
@---------------------------------------------------------------------------------
IntrMain:
@---------------------------------------------------------------------------------
									@ Single interrupts support
	mov		r3, #0x4000000			@ REG_BASE
	ldr		r2, [r3, #0x200]		@ Read REG_IE and REG_IF
	and		r1, r2, r2, lsr #16		@ r1 =	REG_IE & REG_IF

	ldrh	r2, [r3, #-8]			@\mix up with BIOS irq flags at 3007FF8h,
	orr		r2, r2, r1				@ aka mirrored at 3FFFFF8h, this is required
	strh	r2, [r3, #-8]			@/when using the (VBlank)IntrWait functions

	add		r3, r3, #0x200
	ldr		r2, =IntrTable

	ands	r0, r1, #1				@ V-Blank Interrupt
	bne		jump_intr
	add		r2, r2, #4

	ands	r0, r1, #2				@ H-Blank Interrupt
	bne		jump_intr
	add		r2, r2, #4

	ands	r0, r1, #4				@ V Counter Interrupt
	bne		jump_intr
	add		r2, r2, #4

	ands	r0, r1, #8				@ Timer 0 Interrupt
	bne		jump_intr
	add		r2,	r2,	#4

	ands	r0, r1, #0x10			@ Timer 1 Interrupt
	bne		jump_intr
	add		r2, r2, #4

	ands	r0, r1, #0x20			@ Timer 2 Interrupt
	bne		jump_intr
	add		r2, r2, #4

	ands	r0, r1, #0x40			@ Timer 3 Interrupt
	bne		jump_intr
	add		r2, r2, #4

	ands	r0, r1, #0x80			@ Serial Communication	Interrupt
	bne		jump_intr
	add		r2, r2, #4

	ands	r0, r1, #0x100			@ DMA0 Interrupt
	bne		jump_intr
	add		r2, r2, #4

	ands	r0, r1, #0x200			@ DMA1 Interrupt
	bne		jump_intr
	add		r2, r2, #4

	ands	r0, r1, #0x400			@ DMA2 Interrupt
	bne		jump_intr
	add		r2, r2, #4

	ands	r0, r1, #0x800			@ DMA3 Interrupt
	bne		jump_intr
	add		r2, r2, #4

	ands	r0, r1, #0x1000			@ Key Interrupt
	bne		jump_intr
	add		r2, r2, #4

	ands	r0, r1, #0x2000			@ Cart Interrupt

	strneb	r0, [r3, #0x84 - 0x200]	@ Stop	sound if cart removed (REG_SOUNDCNT_X)
loop:
	bne		loop					@ Infinite	loop if	cart removed

@---------------------------------------------------------------------------------
jump_intr:
@---------------------------------------------------------------------------------
	strh	r0, [r3, #2]			@ Acknowlegde int (will clear REG_IF)
	ldr		r0, [r2]				@ Jump	to user	IRQ	process
	bx		r0
@ required for assembling on dkAdv (but not dkARM)
	.align
	.pool
