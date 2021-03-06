#include "LPC17xx_asm.h"
#include "vectrexbus.h"

    .syntax unified
    .thumb

    .section ".text"
    .thumb_func
    .globl VectrexBusSlave
    .type  VectrexBusSlave, %function

    // preconditions:
    //  * all input pins should be initialized as inputs, function 0 
    //  * A2A3 should be enabled
    //  * ROMBase should point to the ROM image
 VectrexBusSlave:
    .fnstart

    push {r4,r5,r6,r7,r8,r9,r10,r11}
    
    cpsid i                 // we cannot handle interrupts in this tight loop

    // setup FIOMASK for ABUS (zero = access enabled)
    ldr r0, =~(ABUS_ALLBV | (1<<CART_N) | (1<<CE_N) | (1<<OE_N) | (1<<RW_N) | (1<<NMI_N) | (1<<HALT_N) | (1<<IRQ_N))
    ldr r1, =FIOMASK_ABUS
    str r0, [r1]

    // setup FIOMASK for B6 
    ldr r0, =~(1<<B6)
    ldr r1, =FIOMASK_B6
    str r0, [r1]
    
    // setup FIOMASK for DBUS (zero = access enabled)
    ldr r1, =FIOMASK_DBUS
    ldr r0, =~0xff
    str r0, [r1]

    // prepare long life registers
    ldr r5, =ROMBase        
    ldr r5, [r5]            // r5 = rom base
    ldr r6, =FIOPIN_ABUS    // r6 = FIOPIN for ABUS, cart_n, oe_n
    ldr r7, =FIODIR_DBUS    // r7 = FIODIR for DBUS
    ldr r8, =FIOPIN_DBUS    // r8 = FIOPIN for DBUS
    ldr r9, [r7]            // r9 = [FIODIR_DBUS]
    ldr r10, =FIOPIN_B6

 .forever:
    // CE_N = A15 on CPU (not the A15 that would select the bank in cartridge)
    // wait for CE_N to go low: ~ 300ns before CART_N goes down
 .l_wait_ce_n:
    ldr r1, [r6]
    tst r1, (1<<CE_N)
    bne .l_wait_ce_n
    
    ldr r1, [r6]            // sample ABUS once again to be sure that it's settled
    mov r11, r1 

    // combine address from bit fields A={r1[17:13],r1[11,0]}
    ubfx r2, r1, #0, #12    // extract bit field [11:0]
    and r1, r1, 0x38000     // bits [17:15]
    orr r2, r2, r1, lsr #3  // destination, first op, second op, shift: r2 = r2 | (r1>>3)

    
    ldr r1, [r10]           // get A15 in bit r1[30] (known as B.6)
    eor r1, r1, 1 << B6
    orr r2, r2, r1, lsr #15 // apply A15 to the result
    add r4, r5, r2          // r4 = byte address

    tst r11, (1<<RW_N)      // write cycle?
    bne .l_read_cycle
    //b .l_read_cycle
.l_write_cycle:
                            // check if it's a ROM select 
    cbz r4, .l_romselect    // if write address == 0, data = ROM selection
                            // otherwise just skip the cycle
    bne .l_prepare_dbus_tristate
.l_romselect:
    ldrb r0, [r8]           // return value = DBUS, this is the ROM index
    cpsie i                 // enable interrupts
    b .l_return              // return to main, where the ROM will be selected

.l_read_cycle:
                            // load byte into r3
    ldrb r3, [r4]           // r3 = *(ROMBase + A), our byte
    
    strb r3, [r8]           // send data to DBUS (which is still tristated)
    orr r9, r9, #0xff       // prepare DBUS port [7:0] to be configured as outputs

    // sample ABUS and expect CART_N to go low
 .l_wait_cart_n:
    ldr r1, [r6]            // sample cartridge port
    tst r1, (1<<CART_N)     // test CART_N
    bne .l_wait_cart_n
                            // make DBUS output
    str r9, [r7]            // [FIODIR_DBUS] = [FIODIR_DBUS] | 0xff: switch DBUS direction

.l_prepare_dbus_tristate:
    bic r9, r9, #0xff       // prepare r9 to tristate DBUS port pins

.l_wait_until_end:
    ldr r2, [r6]            // sample cartridge port
    tst r2, #(1<<CE_N)|(1<<CART_N)
    beq .l_wait_until_end

    str r9, [r7]            // tristate DBUS

    b .forever              // wait for the next occasion
.l_return:
    pop {r4,r5,r6,r7,r8,r9,r10,r11}
    bx  lr
    .cantunwind
    .fnend
    .size VectrexBusSlave, .-VectrexBusSlave
