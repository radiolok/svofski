;;;
;;; Copyright 2006, 2007, 2008, 2009 by Brian Dominy <brian@oddchange.com>
;;; ported to Vectrex, 2013 by Frank Buss <fb@frank-buss.de>
;;;
;;; This file is part of GCC.
;;;
;;; GCC is free software; you can redistribute it and/or modify
;;; it under the terms of the GNU General Public License as published by
;;; the Free Software Foundation; either version 3, or (at your option)
;;; any later version.
;;;
;;; GCC is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.

;;; You should have received a copy of the GNU General Public License
;;; along with GCC; see the file COPYING3.  If not see
;;; <http://www.gnu.org/licenses/>.

	; Declare external for main()
	.globl _main

#define __STACK_TOP 0xcbea

	; Declare all linker sections, and combine them into a single bank
	.bank	prog
	.area	.text  (BANK=prog)
	.area .data  (BANK=prog)
	.area .ctors (BANK=prog)
	.word 0
	.area .bss   (BANK=prog)

   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;
	;;; cartridge init block
	;;;
   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.area	.text
	.ascii "g GCE FUCK"	; cartrige id and year
	.byte 0x80			; string end
	.word music
	.byte 0xf8, 0x50, 0x20, -0x40	; height, width, rel y, rel x
	.ascii "VECTREXADOR"	; game title
	.byte 0x80			; string end
	.byte 0				; header end

   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;
	;;; __start : Entry point to the program
	;;;
   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.area	.text
	.globl __start
__start:

; TODO: crash
	;; Call any "initializer" functions
;	ldu	#s_.ctors
__ctors_loop:
;	ldy	,u++
;	cmpy	#0
;	beq	__ctors_done
;	jsr	,y
;	bra	__ctors_loop
__ctors_done:

	;; Call the main function.  The exit code will
	;; be returned in the X register, unless compiled
	;; with -mdret, in which case it comes back in D.
	jsr	_main
	jmp	_exit

music:
    .word   0xfee8
    .word   0xfeb6
    .byte   0x0, 0x80
    .byte   0x0, 0x80

	.globl _zergnd
_zergnd:
	pshs	a, b, x, dp
	lda	#0xd0		; required input values for the positd executive subroutine:
	tfr	a, dp		; DP = 0xd0
	jsr	0xf354
	puls	a, b, x, dp, pc

	.globl _frwait
_frwait:
	pshs	a, b, x, dp
	lda	#0xd0		; required input values for the positd executive subroutine:
	tfr	a, dp		; DP = 0xd0
	jsr	0xf192
	puls	a, b, x, dp, pc

	.globl _positd
_positd:
; input from C compiler:
; - first 8-bit argument x in register b
; - second 8-bit argument y on stack
	pshs	a, b, dp	; save registers
	lda	#0xd0		; required input values for the positd executive subroutine:
	tfr	a, dp		; DP = 0xd0
	lda	5, s		; a = y coordinate
	jsr	0xf2fc		; b = y coordinate
	puls	a, b, dp, pc	; restore registers and PC (=return)

	.globl _intens
_intens:
; input from C compiler:
; - 8-bit argument in register b
	pshs	a, b, dp	; save registers
	lda	#0xd0		; required input values for the intens executive subroutine:
	tfr	a, dp		; DP = 0xd0
	tfr	b, a		; a = intensity
	jsr	0xf2ab
	puls	a, b, dp, pc	; restore registers and PC (=return)

	.globl _diffab
_diffab:
; input from C compiler:
; - first 8-bit argument x in register b
; - second 8-bit argument y on stack
	pshs	a, b, x, dp	; save registers
	lda	#0xd0		; required input values for the diffab executive subroutine:
	tfr	a, dp		; DP = 0xd0
	lda	7, s		; a = y coordinate
	jsr	0xf3df		; b = y coordinate
	puls	a, b, x, dp, pc	; restore registers and PC (=return)

	.globl _pack1x
_pack1x:
; input from C compiler:
; - 1st 16-bit argument in register x: pointer to packet style list
; - 2nd 8-bit argument in register b: zskip: 0 = skip zeroing integrator, 1 = zero integrators
	pshs	a, b, x, dp	; save registers
	lda	#0xd0		; required input values for the executive subroutine
	tfr	a, dp		; DP = 0xd0
	stb	0xc824		; ZSKIP
	jsr	0xf408
	puls	a, b, x, dp, pc	; restore registers and PC (=return)

	.globl _Draw_VLp_scale
_Draw_VLp_scale:
; input from C compiler:
; - 1st 16-bit argument in register x: pointer to packet style list
; - 2nd 8-bit argument in register b: zskip: 0 = skip zeroing integrator, 1 = zero integrators
	pshs	a, b, x, dp	; save registers
	lda	#0xd0		; required input values for the executive subroutine
	tfr	a, dp		; DP = 0xd0
	stb	0xc824		; ZSKIP
	jsr	0xf40c
	puls	a, b, x, dp, pc	; restore registers and PC (=return)


	.globl _Print_Str_d
_Print_Str_d:
; input:
;	byte x in register b
;	byte y on stack
;   string pointer in register x
	pshs a, b, x, dp
	lda #0xd0
	tfr a, dp
	exg x, u
	lda 7, s
	jsr 0xf37a
	puls a, b, x, dp, pc

_Print_Str_hwyx:
; input:
;   parameter block
	pshs a, b, x, dp
	exg x, u
	jsr 0xf373
	puls a, b, x, dp, pc

	;Draws a big line from the middle of screen to right edge.
demo:
    jsr   0xf192  ; __waitrecal                 ;Reset the CRT
    lda   #00                       ;Get y
    ldb   #00                       ;Get x
    jsr   0xf2fc  ; __move_pen7f_to_d           ;go to (x,y)
    lda   #0x7f                      ;Get the Intensity
    jsr   0xf2ab  ; __intensity_to_A            ;Set intensity
    lda   #00                       ;Get y
    ldb   #127                      ;Get x
    jsr   0xf3df  ; __draw_to_d                 ;draw a line to (x,y)
    bra   demo

   	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;;;
	;;; __exit : Exit point from the program
	;;; There is no exit for Vectrex, infinite loop
	;;;
   	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	.area	.text
 	.globl __exit
__exit:
	bra	__exit


        ;;;
        ;;; __crt0_vector : Default handler for interrupts
        ;;;
        .area   .text
___crt0_vector:
        ;; The default behavior is to simply ignore all
        ;; non-reset interrupts.
        rti
        
        ;;;
        ;;; vector : The interrupt vector table
        ;;; The linker will ensure that this gets loaded at address 0xFFF0.
        ;;;
        .area vector
vectors:
        .word ___crt0_vector
        .word ___crt0_vector
        .word ___crt0_vector
        .word ___crt0_vector
        .word ___crt0_vector
        .word ___crt0_vector
        .word ___crt0_vector
        .word __start

	.end __start

