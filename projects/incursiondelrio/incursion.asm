        ; i8080 assembler code

        .binfile asaltodelrio.rom

        .nodump

stacksave   equ $20

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
    mvi a, $c3
    sta $38
    lxi h, $38
    shld $39
    ei

    ; do stuff

    call setpalette
    call showlayers
    call sprites

    jmp jamas

    ; pintar los colores
showlayers:
    lxi h, $ffff
    ; verde     1 0 0 0
    shld $81fe 
    shld $81fc 
    ; amarillo  0 0 1 0 
    shld $c2fe
    shld $c2fc
    ; negro     0 1 0 0
    shld $a3fe
    shld $a3fc
    ; rosa      0 1 1 0
    shld $a4fe  
    shld $c4fe
    shld $a4fc  
    shld $c4fc
    ; blanco    0 0 0 1
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
    

sprites:
    lxi d, $e008
    call onesprite
    lxi d, $e018
    call onesprite
    lxi d, $e028
    call onesprite
    lxi d, $e038
    call onesprite
    lxi d, $e048
    call onesprite
    lxi d, $e058
    call onesprite
    lxi d, $e068
    call onesprite
    lxi d, $e078
    call onesprite
    lxi d, $e088
    call onesprite
    lxi d, $e098
    call onesprite
    lxi d, $e0a8
    call onesprite
    lxi d, $e0b8
    call onesprite
    lxi d, $e0c8
    call onesprite
    lxi d, $e0d8
    call onesprite
    lxi d, $e0e8
    call onesprite

;   ;
    lxi d, $e608
    call onesprite
    lxi d, $e618
    call onesprite
    lxi d, $e628
    call onesprite
    lxi d, $e638
    call onesprite
    lxi d, $e648
    call onesprite
    lxi d, $e658
    call onesprite
    lxi d, $e668
    call onesprite
    lxi d, $e678
    call onesprite
    lxi d, $e688
    call onesprite
    lxi d, $e698
    call onesprite
    lxi d, $e6a8
    call onesprite
    lxi d, $e6b8
    call onesprite
    lxi d, $e6c8
    call onesprite
    lxi d, $e6d8
    call onesprite
    lxi d, $e6e8
    call onesprite
;
;   ;
    lxi d, $ec08
    call onesprite
    lxi d, $ec18
    call onesprite
    lxi d, $ec28
    call onesprite
    lxi d, $ec38
    call onesprite
    lxi d, $ec48
    call onesprite
    lxi d, $ec58
    call onesprite
    lxi d, $ec68
    call onesprite
    lxi d, $ec78
    call onesprite
    lxi d, $ec88
    call onesprite
    lxi d, $ec98
    call onesprite
    lxi d, $eca8
    call onesprite
    lxi d, $ecb8
    call onesprite
    lxi d, $ecc8
    call onesprite
    lxi d, $ecd8
    call onesprite
    lxi d, $ece8
    call onesprite

;
    lxi d, $f208
    call onesprite
    lxi d, $f218
    call onesprite
    lxi d, $f228
    call onesprite
    lxi d, $f238
    call onesprite
    lxi d, $f248
    call onesprite
    lxi d, $f258
    call onesprite
    lxi d, $f268
    call onesprite
    lxi d, $f278
    call onesprite
    lxi d, $f288
    call onesprite
    lxi d, $f298
    call onesprite
    lxi d, $f2a8
    call onesprite
    lxi d, $f2b8
    call onesprite
    lxi d, $f2c8
    call onesprite
    lxi d, $f2d8
    call onesprite
    lxi d, $f2e8
    call onesprite

;
    lxi d, $f708
    call onesprite
    lxi d, $f718
    call onesprite
    lxi d, $f728
    call onesprite
    lxi d, $f738
    call onesprite
    lxi d, $f748
    call onesprite
    lxi d, $f758
    call onesprite
    lxi d, $f768
    call onesprite
    lxi d, $f778
    call onesprite
    lxi d, $f788
    call onesprite
    lxi d, $f798
    call onesprite
    lxi d, $f7a8
    call onesprite
    lxi d, $f7b8
    call onesprite
    lxi d, $f7c8
    call onesprite
    lxi d, $f7d8
    call onesprite
    lxi d, $f7e8
    call onesprite

;
    lxi d, $fc08
    call onesprite
    lxi d, $fc18
    call onesprite
    lxi d, $fc28
    call onesprite
    lxi d, $fc38
    call onesprite
    lxi d, $fc48
    call onesprite
    lxi d, $fc58
    call onesprite
    lxi d, $fc68
    call onesprite
    lxi d, $fc78
    call onesprite
    lxi d, $fc88
    call onesprite
    lxi d, $fc98
    call onesprite
    lxi d, $fca8
    call onesprite
    lxi d, $fcb8
    call onesprite
    lxi d, $fcc8
    call onesprite
    lxi d, $fcd8
    call onesprite
    lxi d, $fce8
    call onesprite


    ret

onesprite_soft:
    lxi h, 0
    dad sp
    shld sprites_scratch    

    mov a, e        ; save low (dest addr) in a
    lxi sp, gorilla_0   ; sprite address
    xchg //lxi h, $e000     ; start location
    mvi b, $4       ; 4 columns
spritehloop:
    mvi c, $4       ; 4*2 lines high
spritevloop:    
    pop d
    mov m, e
    dcr l
    mov m, d
    dcr l
    dcr c   
    jnz spritevloop 
    inr h
    ;mvi l, $0
    mov l, a
    dcr b
    jnz spritehloop

    lhld sprites_scratch
    sphl
    ret


sprites_scratch:    dw 0

onesprite:
hardsprite:
    lxi h, 0
    dad sp
    shld sprites_scratch    
    xchg
    sphl

    lxi d, $76f6
    push d
    lxi d, $c666
    push d
    lxi d, $36f2
    push d
    lxi d, $e200
    push d
    lxi h, $100+8
    dad sp
    sphl
    lxi d, $ccda
    push d
    lxi d, $dada
    push d
    lxi d, $da9a
    push d
    lxi d, $0c00 
    push d
    lxi h, $100+8
    dad sp
    sphl

    lxi d, $3236
    push d
    lxi d, $1606
    push d
    lxi d, $0606
    push d
    lxi d, $0200
    push d
    lxi h, $100+8
    dad sp
    sphl

    lxi d, $8cda
    push d
    lxi d, $dace
    push d
    lxi d, $c2da
    push d
    lxi d, $8c00
    push d

    lhld sprites_scratch
    sphl
    ret


zero16: dw 0

c_black     equ $00
c_blue      equ $c0
c_green     equ $38
c_yellow    equ $3f
c_magenta   equ $c7
c_white     equ $ff

palette_data:
    db c_blue, c_white,  c_yellow, c_white
    db c_black,c_white,  c_magenta, c_white
    db c_green,c_white,  c_yellow, c_white
    db c_black, c_white, c_magenta, c_white

;palette_data:
;   db $ff, $66, $ff, $66, $ff, $66, $ff, $66
;   db $ff, $66, $ff, $66, $ff, $66, $ff, $66

gorilla_columns equ 4
gorilla_rows    equ 8
gorilla_0:
db 076h,0f6h,0c6h,066h,036h,0f2h,0e2h,00h,
db 0cch,0dah,0dah,0dah,0dah,09ah,0ch,00h,
db 032h,036h,016h,06h,06h,06h,02h,00h,
db 08ch,0dah,0dah,0ceh,0c2h,0dah,08ch,00h,

dactr:  equ .

sintbl  equ $300 

