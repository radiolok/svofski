ColdStart:		equ 0
CurrentDMAAddr: equ $F23D
CAPST:			equ $FCAB
GETPNT:			equ $F3FA
PUTPNT:			equ $F3F8
STORESP:		equ $F304
SLTSL:			equ 0FFFFh		; secondary slot select

DEBUG:			equ 0


				org $E900


EntryPoint:     jp EntryPoint_
				jp Init
EntryPoint_:               
                ld      (SaveDE), de

                ld		(STORESP), sp
                ;ld		(SaveSP), sp
                ld 		sp, $DC00 

;                di
;                push af
;                ld      a, 0AAh         ; Secondary slot select register
;                ld      (SLTSL), a      ; Select expansion slot 2 in all 4 pages
;                ld      a, 0FFh
;                out     (0A8h), a       ; Primary slot register
;                pop af


                ; put local return address on stack
                ld		de, ExitPoint
                push 	de
                ld		de, (SaveDE)

                push    bc
                ld      b, a
                ld      a, 0Dh
                cp      c               ; 0d - c
                ld      a, b
                pop     bc
                jp      nc, CustomDispatch ; Dispatch BDOS function in C to custom hooks from DispatchTable
                push    bc
                ld      b, a
                ld      a, 31h ; '1'    ; function less than GET DISK PARAMETERS (31H)
                cp      c
                pop     bc
                ret     c               ; unsupported func, return with C
                push    af
                ld      a, c
                cp      1Ah             ; Set disk transfer address
                jr      z, NoDiskDispatch
                cp      2Ah ; '*'       ; Get date
                jr      z, NoDiskDispatch
                cp      2Bh ; '+'       ; Set date
                jr      z, NoDiskDispatch
                cp      2Ch ; ','       ; Get time
                jr      z, NoDiskDispatch
                cp      2Dh ; '-'       ; Set time
                jr      z, NoDiskDispatch
                cp      2Eh ; '.'       ; Set/reset verify flag
                jr      z, NoDiskDispatch
                pop     af
                jr      DiskIOFunc

; ---------------------------------------------------------------------------
ExitPoint:
				;ld 		sp, (SaveSP)
				ld 		sp,(STORESP)
				ret
; ---------------------------------------------------------------------------


NoDiskDispatch:                   
                                        
                pop     af
                jp      CustomDispatch ; Dispatch BDOS function in C to custom hooks from DispatchTable
; ---------------------------------------------------------------------------

DiskIOFunc:                       
                push    af
                push    de
                push    hl
                push    bc
                ld      a, (ADR1_SendersNumber)
                or      a
                jr      nz, SendersAddressKnown
                ld      a, 8Fh ; 'П'
                ld      hl, 7C9Dh
; ---------------------------------------------------------------------------
                rst     30h             ; RDSLT
                                        ;   Slot =0x8F, NetBIOS
                                        ;   Addr = 0x7C9D: ADR1: Sender's number
                                        ; Return A = Value
                db 70h
                dw 0Ch
; ---------------------------------------------------------------------------
                ld      (ADR1_SendersNumber), a

SendersAddressKnown:              
                call    WaitUntilByteReceived

WaitUntilCalledByServer:          
                                        
                ld      a, 7
; ---------------------------------------------------------------------------
                rst     30h             ; SNSMAT Returns the value of the specified line from the keyboard matrix
                                        ; Input    : A  - for the specified line
                                        ; Output   : A  - for data (the bit corresponding to the pressed key will be 0)
                db 70h
                dw 141h
; ---------------------------------------------------------------------------
                bit     2, a
                jr      z, ClearKeyboardBuffer
                call    FIFO_ReceiveByteImmediate
                or      a
                jr      z, WaitUntilCalledByServer ; Nothing received, wait more
                ld      hl, ADR1_SendersNumber
                cp      (hl)
                jr      nz, WaitUntilCalledByServer ; message not for us, wait more
                call    FIFO_ReceiveByteWait
                cp      0F4h ; 'Ї'      ; PING by server
                jr      nz, WaitUntilCalledByServer
                ld      d, 0FFh         ; Reply PONG
                call    FIFO_SendByte
                di
                call    WaitUntilByteReceived
                pop     bc
                push    bc              ; c = function number
                ld      a, c
                sub     0Eh
                ld      d, a
                call    FIFO_SendByte ; send to server BDOS function number - 0x0E
                pop     bc
                pop     hl
                pop     de
                pop     af
                call    UpdateCapsLight
                call    CustomDispatch ; Dispatch BDOS function in C to custom hooks from DispatchTable
                call    UpdateCAPS
                ei
                ret
; ---------------------------------------------------------------------------

ClearKeyboardBuffer:              
                pop     bc
                pop     hl
                pop     de
                pop     af
                rst     30h             ; CHGET (Waiting)
                db 70h
                dw 9Fh
                ld      hl, (PUTPNT)    ; points to adress to write in the key buffer
                ld      (GETPNT), hl    ; points to adress to write in the key buffer
                ld      a, 0FFh
                ret

; ****************************************
; * UpdateCapsLight 
; **************************************** 
UpdateCapsLight:                  
                ex      af, af'
                ld      a, (CAPST)      ; capital status ( 00# = Off / FF# = On )
                or      a
                jr      nz, capslock_on
                jr      capslock_off


; ****************************************
; * UpdateCAPS 
; **************************************** 
UpdateCAPS:                       
                ex      af, af'
                ld      a, (CAPST)
                or      a
                jr      z, capslock_on

capslock_off:                     
                ld      (CAPST), a
                ld      a, 0Ch
                out     (0ABh), a
                ex      af, af'
                ret
capslock_on:                      
                                        ; UpdateCAPS+5
                ld      (CAPST), a
                ld      a, 0Dh
                out     (0ABh), a
                ex      af, af'
                ret


;
; Dispatch BDOS function in C to custom hooks from DispatchTable
;
CustomDispatch:                   
                ei
                push    af
                push    bc

                ld      a, c
                add     a, a
                ld      ix, DispatchTable
                ld      b, 0
                ld      c, a
                add     ix, bc
                ld      b, (ix+1)
                ld      c, (ix+0)
                push    bc
                pop     ix
                pop     bc
                pop     af

                jp      (ix)

; ---------------------------------------------------------------------------

Func0_ProgramTerminate:           
                jp      ColdStart
; ---------------------------------------------------------------------------

Func1_ConsoleInput:               
                rst     30h             ; CHSNS Tests the status of the keyboard buffer
                db 70h
                dw 9Ch

                jr      z, Func1_ConsoleInput ; CHSNS Tests the status of the keyboard buffer
                call    Func7_ConsoleInput
                cp      3               ; Ctrl+C
                jr      z, Func0_ProgramTerminate
                ld      e, a

Func2_ConsoleOutput:              
                rst 	30h
				db 70h
				dw 9ch

                jr      z, CHGET_waiting_bufferfull
                call    Func7_ConsoleInput
                cp      3
                jr      z, Func0_ProgramTerminate

CHGET_waiting_bufferfull:         
                ld      a, e
                rst     30h             ; CHPUT
                db 70h
                dw 0A2h
                ret
; ---------------------------------------------------------------------------

Func3_AUXin:                      
                ret                     ; AUX in
; ---------------------------------------------------------------------------

Func4_AUXout:                     
                ld      a, e
                rst     30h             ; Output to current output channel (printer, diskfile, etc.)
                db 70h
                dw 18h
locret_0_EA05:
                ret
; ---------------------------------------------------------------------------

Func5_PrinterOutput:              
                ld      a, e
                rst     30h
                db 70h
                dw 0A5h
                ret

Func6_ConsoleIO:                  
                ld      a, 0FFh
                cp      e
                ld      a, e
                jr      z, Func8_ConsoleInputNoEcho
                jr      Func2_ConsoleOutput ; CHSNS Tests the status of the keyboard buffer
                                        ; Z-flag set if buffer is filled

; ****************************************
; * Func7_ConsoleInput 
; **************************************** 
Func7_ConsoleInput:               
                                        
                rst     30h
                db 70h
                dw 9Fh
                ret
; End of function Func7_ConsoleInput

; ---------------------------------------------------------------------------

Func8_ConsoleInputNoEcho:         
                                        
                rst     30h
                db 70h
                dw 9Ch
                jr      z, loc_0_EA27
                call    Func7_ConsoleInput
                cp      3
                jp      z, Func0_ProgramTerminate

loc_0_EA27:                             
                rst     30h
                ld      (hl), b
                sbc     a, a
                nop
                ret
; ---------------------------------------------------------------------------

Func9_StringOutput:               
                                        
                ld      a, (de)
                cp      24h ; '$'
                ret     z
                push    de
                ld      e, a
                call    Func2_ConsoleOutput ; CHSNS Tests the status of the keyboard buffer
                                        ; Z-flag set if buffer is filled
                pop     de
                inc     de
                jr      Func9_StringOutput
; ---------------------------------------------------------------------------

FuncA_BufferedLineInput:          
                ret

; CHSNS Tests the status of the keyboard buffer
FuncB_ConsoleStatus: 
                rst     30h
                db 70h
                dw 9Ch
                jr      z, FuncB_ConsoleStatus_clra
                ld      a, 0FFh
                ret
; ---------------------------------------------------------------------------

FuncB_ConsoleStatus_clra:                              
                xor     a
                ret
; End of function FuncB_ConsoleStatus

; ---------------------------------------------------------------------------

FuncC_VersionNumber:              
                ld      hl, 22h ; '"'
                ret
; ---------------------------------------------------------------------------

FuncD_DiskReset:                  
                ld      de, 80h ; 'А'   ; Set DMA to 0x0080
                jr      Func1A_SetDMA
; ---------------------------------------------------------------------------

FuncE_SelectDisk:                 
                ld      d, e
                jp      FIFO_SendByte
; ---------------------------------------------------------------------------

FuncF_OpenFile:                   
                call    SendFCB
                call    ReceiveDataToArg
                jp      FIFO_ReceiveByteWait
; ---------------------------------------------------------------------------

Func10_CloseFile:                 
                call    SendFCB
                jp      FIFO_ReceiveByteWait
; ---------------------------------------------------------------------------

Func11_SearchFirst:               
                                        
                call    SendFCB
                call    ReceiveChunkToDMA
                jp      FIFO_ReceiveByteWait
; ---------------------------------------------------------------------------

Func12_SearchNext:                
                jr      Func11_SearchFirst
; ---------------------------------------------------------------------------

Func13_DeleteFile:                
                call    SendFCB
                jp      FIFO_ReceiveByteWait
; ---------------------------------------------------------------------------

Func14_SequentialRead:            
                call    SendFCB
                call    ReceiveChunkToDMA
                call    ReceiveDataToArg
                jp      FIFO_ReceiveByteWait
; ---------------------------------------------------------------------------

Func15_SequentialWrite:           
                push    bc
                ld      bc, 1388h
                call    DelayBC
                pop     bc
                call    SendFCB
                push    bc
                ld      bc, 1388h
                call    DelayBC
                pop     bc
                call    Send128BytesFromDMA
                call    ReceiveDataToArg
                jp      FIFO_ReceiveByteWait
; ---------------------------------------------------------------------------

Func16_CreateFile:                
                call    SendFCB
                call    ReceiveDataToArg
                jp      FIFO_ReceiveByteWait
; ---------------------------------------------------------------------------

Func17_RenameFile:                
                ld      h, d
                ld      l, e
                ld      bc, 20h ; ' '
                call    bdos_SendDataChunk ; Send data chunk &HL, BC = Length
                jp      FIFO_ReceiveByteWait
; ---------------------------------------------------------------------------

Func18_GetLoginVector:            
                call    FIFO_ReceiveByteWait
                ld      h, a
                call    FIFO_ReceiveByteWait
                ld      l, a
                ret
; ---------------------------------------------------------------------------

Func19_GetCurrentDrive:           
                jp      FIFO_ReceiveByteWait
; ---------------------------------------------------------------------------

Func1A_SetDMA:                    
                                        
                ld      (CurrentDMAAddr), de
                ret
; ---------------------------------------------------------------------------

Func1B_GetAllocInfo:              
                ld      d, e
                call    FIFO_SendByte
                call    FIFO_ReceiveByteWait
                ld      h, a
                call    FIFO_ReceiveByteWait
                ld      l, a
                push    hl
                pop     ix
                call    ReceiveDataChunk ; Receive data chunk to &HL, size not returned
                call    FIFO_ReceiveByteWait
                ld      h, a
                call    FIFO_ReceiveByteWait
                ld      l, a
                push    hl
                pop     iy
                call    ReceiveDataChunk ; Receive data chunk to &HL, size not returned
                call    FIFO_ReceiveByteWait
                ld      b, a
                call    FIFO_ReceiveByteWait
                ld      c, a
                call    FIFO_ReceiveByteWait
                ld      d, a
                call    FIFO_ReceiveByteWait
                ld      e, a
                call    FIFO_ReceiveByteWait
                ld      h, a
                call    FIFO_ReceiveByteWait
                ld      l, a
                jp      FIFO_ReceiveByteWait
; ---------------------------------------------------------------------------

Func21_RandomRead:          
				call 	SendDebugBlock      
                call    SendFCB
                call    ReceiveChunkToDMA
                call    ReceiveDataToArg
                jp      FIFO_ReceiveByteWait
; ---------------------------------------------------------------------------

Func22_RandomWrite:               
                call    SendFCB
                call    Send128BytesFromDMA
                jp      FIFO_ReceiveByteWait
; ---------------------------------------------------------------------------

Func23_GetFileSize:               
                call    SendFCB
                call    ReceiveDataToArg
                jp      FIFO_ReceiveByteWait
; ---------------------------------------------------------------------------

Func24_SetRandomRecord:           
                call    SendFCB
                jp      ReceiveDataToArg
; ---------------------------------------------------------------------------

Func26_RandomBlockWrite:          
;                push    bc
;                ld      bc, 3E8h
;                call    DelayBC
;                pop     bc
;                ld      d, h
;                call    FIFO_SendByte
;                ld      d, l
;                call    FIFO_SendByte
;                push    bc
;                ld      bc, 2710h
;                call    DelayBC
;                pop     bc
;                ld      ix, (SaveDE)
;                ld      a, (ix+0Eh)
;                ex      de, hl
;                call    Mul_DE_by_A
;                ld      b, h
;                ld      c, l
;                ld      hl, (CurrentDMAAddr)
;                call    bdos_SendDataChunk ; Send data chunk &HL, BC = Length
;                push    bc
;                ld      bc, 2710h
;                call    DelayBC
;                pop     bc
;                call    SendFCB
;                call    ReceiveDataToArg
;                jp      FIFO_ReceiveByteWait
; ---------------------------------------------------------------------------

Func27_RandomBlockRead:  
				call SendDebugBlock         
                push    bc
                ;ld      bc, 2710h
                ld      bc, 32h
                call    DelayBC
                pop     bc
                ld      d, h
                call    FIFO_SendByte
                ld      d, l
                call    FIFO_SendByte
                push    bc
                ;ld      bc, 2710h
                ld      bc, 32h
                call    DelayBC
                pop     bc
                call    SendFCB
                call    FIFO_ReceiveByteWait
                ld      h, a
                call    FIFO_ReceiveByteWait
                ld      l, a
                call    ReceiveChunkToDMA

if DEBUG
                ld a, '@'
                out (98h), a
endif
                call    ReceiveDataToArg

if DEBUG 
                ld a, '#'
                out (98h), a
endif
                jp      FIFO_ReceiveByteWait
; ---------------------------------------------------------------------------

Func28_RandomWriteWithZeroFill:   
                call    SendFCB
                call    Send128BytesFromDMA
                call    ReceiveDataToArg
                jp      FIFO_ReceiveByteWait
; ---------------------------------------------------------------------------

Func2A_GetDate:          
				xor a, a   
				ld h, a
				ld l, a      
                ld d, a
                ld e, a
                ret
; ---------------------------------------------------------------------------

Func2B_SetDate:                   
                ret                     ; nop
; ---------------------------------------------------------------------------

Func2C_GetTime:  
                ld      h, 0            ; Always return 0
                ld      l, h
                ld      d, h
                ld      e, h
                ret
; ---------------------------------------------------------------------------

Func2D_SetTime:                   
                ret                     ; nop
; ---------------------------------------------------------------------------

Func2E_SetResetVerifyFlag:        
                ret                     ; nop
; ---------------------------------------------------------------------------

Func2F_AbsoluteSectorRead:        
;                call    FIFO_SendByte
;                ld      d, e
;                call    FIFO_SendByte
;                ld      d, h
;                call    FIFO_SendByte
;                ld      d, l
;                call    FIFO_SendByte
;                jp      ReceiveChunkToDMA
; ---------------------------------------------------------------------------

Func30_AbsoluteSectorWrite: 
;                push    bc
;                ld      bc, 3E8h
;                call    DelayBC
;                pop     bc
;                call    FIFO_SendByte
;                ld      d, e
;                call    FIFO_SendByte
;                ld      d, h
;                call    FIFO_SendByte
;                ld      d, l
;                call    FIFO_SendByte
;                push    bc
;                ld      bc, 2710h
;                call    DelayBC
;                pop     bc
;                ld      de, 200h
;                ld      a, h
;                call    Mul_DE_by_A
;                ld      b, h
;                ld      c, l
;                ld      hl, (CurrentDMAAddr)
;                call    bdos_SendDataChunk ; Send data chunk &HL, BC = Length
                ret
SendWord:
				ld 		d, h
				call    FIFO_SendByte
				ld 		d, l
				call 	FIFO_SendByte
                push    bc
                ld      bc, 400h
                call    DelayBC
                pop     bc
				ret

SendDebugBlock:
				exx
				ld hl, (CurrentDMAAddr)
				;ld hl, ($fffe)
				;in a, (0a8h)
				;ld l, a
				call SendWord
				ld hl, 0
				add hl, sp
				call SendWord
				ld	hl, (STORESP)
				call SendWord
				push ix
				ld   ix, (STORESP)
				ld   l, (ix+0)
				ld   h, (ix+1)
				call SendWord
				ld   l, (ix+2)
				ld   h, (ix+3)
				call SendWord
				ld   l, (ix+4)
				ld   h, (ix+5)
				call SendWord
				pop  ix
				exx
				ret


SendFCB:                          
                exx
                ld      hl, (SaveDE)
                ld      bc, 2Ch ; ','
                call    bdos_SendDataChunk ; Send data chunk &HL, BC = Length
                exx
                ret

ReceiveDataToArg:                 
                                        
                exx
                ld      hl, (SaveDE)
                call    ReceiveDataChunk ; Receive data chunk to &HL, size not returned
                exx
                ret

; ****************************************
; * Send128BytesFromDMA 
; **************************************** 
Send128BytesFromDMA:              
                exx
                ld      hl, (CurrentDMAAddr)
                ld      bc, 80h ; 'А'
                call    bdos_SendDataChunk ; Send data chunk &HL, BC = Length
                exx
                ret


; ****************************************
; * ReceiveChunkToDMA 
; **************************************** 
ReceiveChunkToDMA:                
                                        
                exx
                ld      hl, (CurrentDMAAddr)
                call    ReceiveDataChunk ; Receive data chunk to &HL, size not returned
                exx
                ret

; ****************************************
; * FIFO_SendByte 
; **************************************** 
FIFO_SendByte:
                ld      a, 5
                out     (9), a
FIFO_SendByte_waitrx:                     
                in      a, (0Ch)
                and     41h ; 'A'
                cp      40h ; '@'
                jr      nz, FIFO_SendByte_waitrx
                ld      a, d
                out     (0Eh), a
                ret

; ****************************************
; * FIFO_ReceiveByteWait 
; **************************************** 
FIFO_ReceiveByteWait:             
                                        
                ld      a, 3
                out     (9), a

loc_0_EC26:                             
                in      a, (0Ch)
                and     83h ; 'Г'
                cp      80h ; 'А'
                jr      nz, loc_0_EC26
                in      a, (0Eh)
                ret

; ****************************************
; * FIFO_ReceiveByteImmediate 
; **************************************** 
FIFO_ReceiveByteImmediate:        
                ld      a, 3
                out     (9), a
                ld      b, 0FFh
                in      a, (0Ch)
                and     83h ; 'Г'
                cp      80h ; 'А'
                jr      z, FIFO_ReceiveByteImmediate_wtf

FIFO_ReceiveByteImmediate_wtf:
                in      a, (0Eh)
                ret

; Send data chunk &HL, BC = Length
bdos_SendDataChunk:
                ld      d, b
                call    FIFO_SendByte
                ld      d, c
                call    FIFO_SendByte

bdos_SendDataChunk_sendloop: 
                push    bc
                ld      bc, 32h ; '2'
                call    DelayBC
                pop     bc
                ld      d, (hl)
                call    FIFO_SendByte
                inc     hl
                dec     bc
                ld      a, b
                or      c
                jr      nz, bdos_SendDataChunk_sendloop
                ret


; Receive data chunk to &HL, size not returned
ReceiveDataChunk:                 
                                        
                call    FIFO_ReceiveByteWait
                ld      b, a
                call    FIFO_ReceiveByteWait
                ld      c, a

if DEBUG
                push hl
                push bc
                push de
                ld h, b
                ld l, c
                call DispHLhex
                pop de
                pop bc
                pop hl

                ld      a, '('
                out     (98h), a        ; VRAM data read/write
endif

ReceiveDataChunk_recvloop:

                call    FIFO_ReceiveByteWait
                ld      (hl), a
                inc     hl
                dec     bc

if DEBUG
                ld a, c
                or a
                jr nz, nodebugprint
                ld      a, '.'
                out     (98h), a        ; VRAM data read/write
nodebugprint:          
endif
                ld      a, b
                or      c
                jr      nz, ReceiveDataChunk_recvloop

if DEBUG
                ld      a, ')'
                out     (98h), a        ; VRAM data read/write
endif
                ret
; ****************************************
; * WaitUntilByteReceived 
; **************************************** 
WaitUntilByteReceived:            
                                        ; E972
                ex      af, af'
                exx

WaitUntilByteReceived_waitrx:                               
                call    FIFO_ReceiveByteImmediate
                or      a
                jr      nz, WaitUntilByteReceived_waitrx
                ex      af, af'
                exx
                ret

; ****************************************
; * DelayBC 
; **************************************** 
DelayBC:                          
                                        
                dec     bc
                ld      a, b
                or      c
                ret     z
                jr      DelayBC

; ****************************************
; * Mul_DE_by_A 
; **************************************** 
Mul_DE_by_A:                      
                ld      hl, 0
                ld      c, 8

rbmulmm:                                    
                add     hl, hl
                rla
                jr      nc, rbmulll
                add     hl, de

rbmulll:                                    
                adc     a, 0
                dec     c
                jr      nz, rbmulmm
                ret

; ---------------------------------------------------------------------------
ADR1_SendersNumber:db 0           
                                        
SaveDE:   		dw 0                    
                        
SaveSP:			dw 0                 
;CAPST:    db 0                    
                                        ; UpdateCAPS:capslock_off ...
DispatchTable:dw Func0_ProgramTerminate
                                        
                dw Func1_ConsoleInput ; CHSNS Tests the status of the keyboard buffer
                dw Func2_ConsoleOutput ; CHSNS Tests the status of the keyboard buffer
                                        ; Z-flag set if buffer is filled
                dw Func3_AUXin    ; AUX in
                dw Func4_AUXout
                dw Func5_PrinterOutput
                dw Func6_ConsoleIO
                dw Func7_ConsoleInput
                dw Func8_ConsoleInputNoEcho
                dw Func9_StringOutput
                dw FuncA_BufferedLineInput
                dw FuncB_ConsoleStatus ; CHSNS Tests the status of the keyboard buffer
                dw FuncC_VersionNumber
                dw FuncD_DiskReset ; Set DMA to 0x0080
                dw FuncE_SelectDisk
                dw FuncF_OpenFile
                dw Func10_CloseFile
                dw Func11_SearchFirst
                dw Func12_SearchNext
                dw Func13_DeleteFile
                dw Func14_SequentialRead
                dw Func15_SequentialWrite
                dw Func16_CreateFile
                dw Func17_RenameFile
                dw Func18_GetLoginVector
                dw Func19_GetCurrentDrive
                dw Func1A_SetDMA
                dw Func1B_GetAllocInfo
                dw 0
                dw 0
                dw 0
                dw 0
                dw 0
                dw Func21_RandomRead
                dw Func22_RandomWrite
                dw Func23_GetFileSize
                dw Func24_SetRandomRecord
                dw 0
                dw Func26_RandomBlockWrite
                dw Func27_RandomBlockRead
                dw Func28_RandomWriteWithZeroFill
                dw 0
                dw Func2A_GetDate ; Always return 0
                dw Func2B_SetDate ; nop
                dw Func2C_GetTime ; Always return 0
                dw Func2D_SetTime ; nop
                dw Func2E_SetResetVerifyFlag ; nop
                dw Func2F_AbsoluteSectorRead
                dw Func30_AbsoluteSectorWrite

Init:			call PatchBIOSCalls
				ret

PatchBIOSCalls:
				ld hl, ($0001) 	; points to dc03: jmp WARMBOOT
				ld bc, 3
				add hl, bc
				; HL points to CONST vector

				ex de, hl 		; DE = BIOS Jump Table
				ld hl, PatchJumpTable
				ld bc, PatchJumpTable_End - PatchJumpTable
				ldir
				ret

PatchJumpTable:	;jp WARMBOOT
				jp CONST 
				jp CONIN
				jp CONOUT
				jp CONOUT ; LIST
				jp CONOUT ; PUNCH 
				jp CONIN ; READER				
				;
PatchJumpTable_End:

WARMBOOT:		jp $

CONST:			
                ld		(STORESP), sp
                ld 		sp, $DC00 
				call 	FuncB_ConsoleStatus
				ld 		sp, (STORESP)
				ret
CONIN:			
                ld		(STORESP), sp
                ld 		sp, $DC00 
				call 	Func8_ConsoleInputNoEcho
				ld 		sp, (STORESP)
				ret

CONOUT:			
                ld		(STORESP), sp
                ld 		sp, $DC00 
				ld 		e, c
				call 	Func2_ConsoleOutput
				ld 		sp, (STORESP)
				ret


;Display a 16- or 8-bit number in hex.
DispHLhex:
; Input: HL
   ld  c,h
   call  OutHex8
   ld  c,l
OutHex8:
; Input: C
   ld  a,c
   rra
   rra
   rra
   rra
   call  Conv
   ld  a,c
Conv:
   and  $0F
   add  a,$90
   daa
   adc  a,$40
   daa
   
   out (98h), a

   ret

DispSpace:
	ld a, ' '
	out (98h), a
	ret

DispCRLF:
	ld a, 0dh
	out (98h), a
	ld a, 0ah
	out (98h), a
	ret

DiagnosticsDeath:
	push hl
	push de
	push bc
	push af
	pop hl
	call DispHLhex
	call DispSpace
	pop hl
	call DispHLhex
	call DispSpace
	pop hl
	call DispHLhex
	call DispSpace
	pop hl
	call DispHLhex
	call DispSpace

	call DispSpace
	ld hl, (CurrentDMAAddr)	
	call DispHLhex
	call DispSpace

	ld hl, (CurrentDMAAddr)	
	ld d, $ff
ddloop:
	ld c, (hl)
	call OutHex8
	call DispSpace
	inc hl
	dec d
	jp nz,ddloop

	jr $


