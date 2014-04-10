    .encoding koi8-r
    .nodump
    .hexfile sample.hex
    .org $100
    mvi c, 9
    lxi d, hello
    call 5
    mov e, c
    ret
    lxi h, nonexistent
babor: nop
    .include messages.inc ; bob
