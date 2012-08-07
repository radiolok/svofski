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
    lxi d, $8008
    call onesprite
    lxi d, $8018
    call onesprite
    lxi d, $8028
    call onesprite
    lxi d, $8038
    call onesprite
    lxi d, $8048
    call onesprite
    lxi d, $8058
    call onesprite
    lxi d, $8068
    call onesprite
    lxi d, $8078
    call onesprite
    lxi d, $8088
    call onesprite
    lxi d, $8098
    call onesprite

    lxi d, $8408
    call onesprite
    lxi d, $8418
    call onesprite
    lxi d, $8428
    call onesprite
    lxi d, $8438
    call onesprite
    lxi d, $8448
    call onesprite
    lxi d, $8458
    call onesprite
    lxi d, $8468
    call onesprite
    lxi d, $8478
    call onesprite
    lxi d, $8488
    call onesprite
    lxi d, $8498
    call onesprite

    lxi d, $8808
    call onesprite
    lxi d, $8818
    call onesprite
    lxi d, $8828
    call onesprite
    lxi d, $8838
    call onesprite
    lxi d, $8848
    call onesprite
    lxi d, $8858
    call onesprite
    lxi d, $8868
    call onesprite
    lxi d, $8878
    call onesprite
    lxi d, $8888
    call onesprite
    lxi d, $8898
    call onesprite

    lxi d, $8c08
    call onesprite
    lxi d, $8c18
    call onesprite
    lxi d, $8c28
    call onesprite
    lxi d, $8c38
    call onesprite
    lxi d, $8c48
    call onesprite
    lxi d, $8c58
    call onesprite
    lxi d, $8c68
    call onesprite
    lxi d, $8c78
    call onesprite
    lxi d, $8c88
    call onesprite
    lxi d, $8c98
    call onesprite


    ret

sprites_scratch:    dw 0

onesprite:
hardsprite:
    lxi h, 0
    dad sp
    shld sprites_scratch    
    
    mov h, d
    mov l, e
    sphl
;; green
    lxi b, $0000
    push b
    push b
    push b
    lxi b, $ff0f
    push b
    lxi h, 256+8
    dad sp
    sphl
    lxi b, $0000
    push b
    push b
    push b
    lxi b, $ffff
    push b
    lxi h, 256+8
    dad sp
    sphl
    lxi b, $0000
    push b
    push b
    push b
    lxi b, $fcfc
    push b
    lxi h, 256+8
    dad sp
    sphl
    lxi b, $0000
    push b
    push b
    push b
    push b
;;;; black/magenta layer 1
    lxi h, $2000
    dad d
    sphl
    lxi b, $0000
    push b
    lxi b, $000f
    push b
    lxi b, $ffff
    push b
    lxi b, $0000
    push b
    lxi h, 256+8
    dad sp
    sphl
    lxi b, $0f0f
    push b
    lxi b, $ffff
    push b
    push b
    lxi b, $0000
    push b
    lxi h, 256+8
    dad sp
    sphl
    push b
    lxi b, $00f0
    push b
    lxi b, $ffff
    push b
    lxi b, $0000
    push b
    lxi h, 256+8
    dad sp
    sphl
    push b
    push b
    lxi b, $fff0
    push b
    lxi b, $0000
    push b
;;;; yellow/magenta layer 2
    lxi h, $4000
    dad d
    sphl
    lxi b, $0000
    push b
    push b
    lxi b, $ffff
    push b
    lxi b, $0000
    push b
    lxi h, 256+8
    dad sp
    sphl
    push b
    push b
    lxi b, $ffff
    push b
    lxi b, $0000
    push b
    lxi h, 256+8
    dad sp
    sphl
    push b
    push b
    lxi b, $ffff
    push b
    lxi b, $0000
    push b
    lxi h, 256+8
    dad sp
    sphl
    push b
    push b
    lxi b, $fff0
    push b
    lxi b, $0000
    push b


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
