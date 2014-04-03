    .encoding koi8-r
    .nodump
    .org $100
    mvi c, 9
    lxi d, hello
    call 5
    ret
    .include messages.inc