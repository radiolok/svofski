    .encoding koi8-r
    .nodump
    .hexfile sample.hex
    .org $105
    mvi c, 9
    lxi d, hello ; see messages.inc
    call 5
    mov e, c
    ret
    lxi h, nonexistent
    lxi b, babor or $ff00
    lxi d, babor;snip
    ; the following 3 nops should be skipped in the listing
    .nolist
    nop
    nop
    nop
    .list
    ; listing should have been turned on now
    nop
    syntax error
babor: db 10, 10b, 10q, 10h
failexpr: db 'four', 8n
refexpr: dw babor;, babor + 1, 2 * babor, babor
    lxi d, babor + 4; back reference
	;.include nonono.inc
    .include messages.inc ; bob
