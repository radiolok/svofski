@
@ swi.s
@ Software interrupts (aka BIOS calls)
@
@ (Created: 2004-06-05, Modified: 2004-07-06, Cearn)
@
	.file	"swi.s"
	.text
	.align	2
	.code	16

@ --- swi_reset [N/A] -------------------------------------------------
@ DECL: void swi_reset();
@ DESC: software reset; + safeties for REG_IME (TODO: REG_RESET_DST safety)
	.global swi_reset
	.thumb_func
swi_reset:
	ldr		r0, =0x04000208
	mov		r1, #0
	strh	r1, [r0]	
	swi		0x00
	bx		lr
@ --- swi_vsync [VBlankIntrWait] --------------------------------------
@ DECL: void swi_vsync()
@ DESC: waits til the next VBlank (make sure the VBlank interrupt works)
	.global swi_vsync
	.thumb_func
swi_vsync:
	swi		0x05
	bx		lr

@ --- swi_div [Div] ---------------------------------------------------
@ DECL: int swi_div(int num, int denom)
@ DESC: num/denom (NOTE: chokes on div by 0!)
	.global	swi_div
	.thumb_func
swi_div:
	swi		0x06
	bx		lr

@ --- swi_div_safe [N/A] ----------------------------------------------
@ DECL: int swi_div_safe(int num, int denom)
@ DESC: As swi_div, but doesn't choke on div by 0
	.global	swi_div_safe
	.thumb_func
swi_div_safe:
	cmp		r1, #0
	beq		.Ldivzero		@ no division by zero plz
	swi		0x06
	bx		lr
.Ldivzero:
	mov		r0, #0x80		@ div/0 gives + infinity
	lsl		r0, #24
	mvn		r0, r0
	bx		lr

@ --- swi_mod [N/A] ---------------------------------------------------
@ DECL: int swi_mod(int num, int denom)
@ DESC: gives num % denom
	.global	swi_mod
	.thumb_func
swi_mod:
	swi		0x06
	mov		r0, r1
	bx		lr

@ --- swi_sqrt [Sqrt] -------------------------------------------------
@ DECL: int swi_sqrt(u32 num)
@ DESC: Square root of num
	.global	swi_sqrt
	.thumb_func
swi_sqrt:
	swi		0x08
	bx		lr

@ --- swi_arctan2 [ArcTan2] -------------------------------------------
@ DECL: s16 swi_arctan2(s16 x, s16 y)
@ DESC: gives theta= atan(y/x),
@   with theta in the [0,0xffff] (eq [0,2pi> ) range   
	.global	swi_arctan2
	.thumb_func
swi_arctan2:
	swi		0x0a
	bx		lr

@ --- swi_aff_ex [BgAffineSet] ----------------------------------------
@ DECL: void swi_aff_ex(AFF_SRC_EX *src, BGAFF_EX *dst);
@ DESC: Fills dst with P=S(sx,sy)*R(theta) and dx=p-P·q
@   
	.global	swi_aff_ex
	.thumb_func
swi_aff_ex:
	swi	   0x0e
	bx	   lr
	
@ --- swi_aff [ObjAffineSet] ---------------------------------------
@ DECL: void swi_aff(AFF_SRC *src, void *dst, int num, int offset);
@ DESC: Fills dst with P=S(sx,sy)*R(theta).
@   dst is a pointer to pa element; offset says howfar apart the 
@   elements are
	.global	swi_aff
	.thumb_func
swi_aff:
	swi	   0x0f
	bx	   lr

@ --- swi_bitunpack [BitUnPack] ---------------------------------------
@ DECL: void swi_bitunpack(const void *src, void *dst, UNPACKINFO *upi)
@ DEST: unpacks data to/from diffent bit-depths
	.global	swi_bitunpack
	.thumb_func
swi_bitunpack:
	swi	   0x10
	bx	   lr
