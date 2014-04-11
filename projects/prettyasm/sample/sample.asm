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
babor: nop
    .include messages.inc ; bob
