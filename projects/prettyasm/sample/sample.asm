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
    ; the following 3 nops should be skipped in the listing
    .nolist
    nop
    nop
    nop
    .list
    ; listing should have been turned on now
    nop
babor: db 10, 10b, 10q, 10h, 8n
    .include messages.inc ; bob
