;===================================================
;===================================================
;=
;=  Attempt to port to Mega168 ...
;=
;=  A complete mess written by me for a Mega8
;=  Started out as a simple SPM test ...
;=  God knows where it will end ... bootloader maybe ...
;=
;=  Ok, got the bootloader working ... :-)
;=  Even managed to squeeze in the watcdog timer ...
;=
;=  "Protocol description":
;=  ----------------------- 
;=  PC           AVR
;=               O<-
;=  ->S              
;= - - - - - - - - - - - - - - - - - - - - - 
;= Startup handshake OK, start pagetransfer
;= - - - - - - - - - - - - - - - - - - - - - 
;=  ->P
;=                p<-
;=  ->addr_H      if 0xFF then goto END ...
;=           addr_H<-
;=  ->addr_L
;=           addr_L<-      
;=
;=  -> 64 bytes data PC-><-AVR ... send/reply
;=  ** Page received, AVR clears/writes page and ...
;=  ** starts the next pagetransfer, ie. waits for 'P'
;=  
;=  After all pages are written the AVR waits in a loop 
;=  until a 'G' is transmitted from the PC  
;=  Unless the watchdog times out ... 
;===================================================
;===================================================


#include <avr/io.h>

F_OSC    = 11059200
B_1152   = (F_OSC/(16*115200))-1
B_19200  = (F_OSC/(16*19200))-1

#define flash_addr_l 4
#define flash_addr_h 5

#define tempreg      16
#define spmcr_temp   17

#define sysreg       18
#define pollreg      19       /*<-- Used in uart tx&rx ...*/

;**********************************************
;
; CODE 
;
;**********************************************
.section .bootloader,"ax",@progbits
main:
  ldi   tempreg, lo8(RAMEND)
  out   _SFR_IO_ADDR(SPL), tempreg
  ldi   tempreg, hi8(RAMEND)
  out   _SFR_IO_ADDR(SPH), tempreg

; Check for watchdog reset ...
; Watchdog reset --> No bootloading --> bail_out

  wdr
  in    tempreg, _SFR_IO_ADDR(MCUSR)
  sbrc  tempreg, WDRF
  rjmp  bail_out

  rcall init_uart

;
; Initial "handshake" ... AVR sends 'O', PC replies with 'S'
;

  ldi   tempreg, 'O'
  rcall uart_tx_byte

; Setup watchdog for longest timeout
; If PC doesn't reply within WD period, a WD reset occurs --> bail_out

  wdr
  ldi   tempreg, _BV(WDCE)|_BV(WDE)
  sts   _SFR_MEM_ADDR(WDTCSR), tempreg
  ldi   tempreg, _BV(WDE)|_BV(WDP0)|_BV(WDP1)|_BV(WDP2)
  sts   _SFR_MEM_ADDR(WDTCSR), tempreg

;
; Wait for 'S'
; 

start_loop:
  lds   tempreg, _SFR_MEM_ADDR(UCSR0A)
  sbrs  tempreg, RXC0
  rjmp  start_loop
  lds   tempreg, _SFR_MEM_ADDR(UDR0)
  cpi   tempreg, 'S'
  brne  start_loop

;
; So far, so good ...
;

ok_to_continue:
  clr   sysreg                ; bld_receive_page sets sysreg to 0x01 after last page
receive_page_loop:
  wdr
  rcall bld_receive_page
  cpi   sysreg, 0x01
  breq  jump_to_application   ; And off we go into the unknown ...
  wdr
  rcall bld_erase_page
  rcall bld_write_page
  wdr
  rjmp  receive_page_loop

jump_to_application:

;
; As a las token of gratitude, the PC sends us a 'G' ...
;


poll_stop_char:
  rcall uart_rx_byte
  cpi   tempreg, 'G'
  brne  poll_stop_char

bail_out:

;
; Clear resetflags & Disable watchdog
;

  clr   tempreg
  out   _SFR_IO_ADDR(MCUSR), tempreg             
  lds   tempreg, _SFR_MEM_ADDR(WDTCSR)
  ori   tempreg, _BV(WDCE)|_BV(WDE)
  sts   _SFR_MEM_ADDR(WDTCSR), tempreg
  clr   tempreg
  sts   _SFR_MEM_ADDR(WDTCSR), tempreg

;
; restore registers to 'power on reset' state
;

  clr   r0
  clr   r1
  clr   tempreg
  clr   flash_addr_l          
  clr   flash_addr_h

  ldi   tempreg, _BV(TXC0)  
  sts   _SFR_MEM_ADDR(UCSR0A), tempreg
  clr   tempreg
  sts   _SFR_MEM_ADDR(UCSR0B), tempreg
  ldi   tempreg, 0x86
  sts   _SFR_MEM_ADDR(UCSR0C), tempreg
  clr   tempreg
  sts   _SFR_MEM_ADDR(UBRR0H), tempreg
  clr   tempreg
  sts   _SFR_MEM_ADDR(UBRR0L), tempreg
  clr   tempreg
  out   _SFR_IO_ADDR(SPMCSR), tempreg

  jmp   0x0000                          ; Lost in code space ...

main_loop:
  rjmp  main_loop


;**********************************************
;
; UART Driver
;
;**********************************************

init_uart:
  ldi   tempreg, hi8(B_1152)
  sts   _SFR_MEM_ADDR(UBRR0H), tempreg
  ldi   tempreg, lo8(B_1152)
  sts   _SFR_MEM_ADDR(UBRR0L), tempreg

  ldi   tempreg, _BV(RXEN0)|_BV(TXEN0)
  sts   _SFR_MEM_ADDR(UCSR0B), tempreg
ret

uart_tx_byte:
  lds   pollreg, _SFR_MEM_ADDR(UCSR0A)
  sbrs  pollreg, UDRE0
  rjmp  uart_tx_byte
  sts   _SFR_MEM_ADDR(UDR0), tempreg
ret

uart_rx_byte:
  lds   pollreg, _SFR_MEM_ADDR(UCSR0A)
  sbrs  pollreg, RXC0
  rjmp  uart_rx_byte
  lds   tempreg, _SFR_MEM_ADDR(UDR0)
ret

;**********************************************
;
; Bootloader stuff
;
;**********************************************

bld_receive_page:

;
; Wait for 'OK to send page? char' ...
;

  rcall    uart_rx_byte
  cpi      tempreg, 'P'
  brne     bld_receive_page

;
; Transmit 'OK to send page char' ...
;

  ldi   tempreg, 'p'
  rcall uart_tx_byte

;  
; High byte of address
;

  rcall uart_rx_byte
  rcall uart_tx_byte

;
; Check for address flash_addr_h = 0xFF
; if 0xFF then "bail out" ...
;

  cpi   tempreg, 0xFF
  breq  bld_last_page_received

  mov   flash_addr_h, tempreg

;
; Low byte of address
;

  rcall uart_rx_byte
  rcall uart_tx_byte
  mov   flash_addr_l, tempreg

;
; Data loop
; ie. Page Load ...
;

  clr   ZL
  clr   ZH
bld_rec_page_loop:
  rcall uart_rx_byte
  rcall uart_tx_byte
  mov   r0, tempreg
  rcall uart_rx_byte
  rcall uart_tx_byte
  mov   r1, tempreg
  ldi   spmcr_temp, _BV(SPMEN)
  rcall do_spm
  adiw  ZL, 0x2
  cpi   ZL, SPM_PAGESIZE
  brne  bld_rec_page_loop
  rjmp  bld_receive_page_end

bld_last_page_received:
  ldi   sysreg, 0x01          ; sysreg = 0x01 --> "bail out" ...
bld_receive_page_end:
ret

bld_erase_page:

  mov   ZL, flash_addr_l
  mov   ZH, flash_addr_h

  ldi   spmcr_temp, _BV(SPMEN)|_BV(PGERS)
  rcall do_spm
ret

bld_write_page:

  mov   ZL, flash_addr_l
  mov   ZH, flash_addr_h

  ldi   spmcr_temp, _BV(SPMEN)|_BV(PGWRT)
  rcall do_spm
  rcall enable_rww
ret

enable_rww:
  ldi   spmcr_temp, _BV(SPMEN)|_BV(RWWSRE)
do_spm:
poll_spmen:
  in    tempreg, _SFR_IO_ADDR(SPMCSR)
  sbrc  tempreg, SPMEN
  rjmp  poll_spmen
  out   _SFR_IO_ADDR(SPMCSR), spmcr_temp
  spm
ret

.end


