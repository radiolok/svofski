    ; i8080 assembler code

    .binfile asaltodelrio.rom

    .nodump

stacksave	equ $20

BOTTOM_HEIGHT 		equ 60
TOP_HEIGHT			equ 16
SCREEN_WIDTH_BYTES	equ 32

FOE_MAX				equ 8  
BLOCKS_IN_LEVEL 	equ 16
BLOCK_HEIGHT		equ 64
HALFBRIDGE_WIDTH	equ 4
NARROWEST			equ 4 	; the narrowest passage around an island
ENOUGH_FOR_ISLAND	equ 9   ; if water this wide, island fits
	 
	.org $100

clrscr:
    di
	lxi h, $8000
	; good for seeing sprite scratch area 
	; mvi b, $ff
	mvi b, 0
clearscreen:
	xra a
	mov m, b
	inx h
	ora h
	jnz clearscreen

	; init stack pointer
	lxi sp, $100

	; enable interrupts

	; write ret to rst 7 vector	
	mvi a, $c9
	sta $38

	; initial stuff
	ei
	hlt
	call setpalette
	call showlayers

jamas:
	mvi a, 4
	out 2
	ei
	hlt
	xra a
	out 2

	; scroll
	mvi a, 88h
	out 0
	lda frame_scroll
	out 3

	; keep interrupts enabled to make errors obvious
	ei
	; prepare animated sprites
	call foe_animate

	lxi d, foe_1
	call foe_in_de
	lxi d, foe_2
	call foe_in_de
	lxi d, foe_3
	call foe_in_de
	lxi d, foe_4
	call foe_in_de
	lxi d, foe_5
	call foe_in_de
	lxi d, foe_6
	call foe_in_de
	lxi d, foe_7
	call foe_in_de
	lxi d, foe_8
	call foe_in_de

	call clearblinds
	call update_line
	call drawblinds_bottom

	lxi h, frame_number
	inr m

	lxi h, frame_scroll
	inr m

	;di
	;jmp .

	jmp jamas

frame_number:
	db 0
frame_scroll:
	db $ff; $10

;;
;; Process foe with descriptor in HL
;;
foe_in_de:
	push d

copyfoe_y:
	lxi h, 0 				; 12
	dad sp 					; 12
	shld copyfoe_restoresp+1	; 16
	xchg					; 8
	sphl					; 8  = 64

	pop h       			; 12
	shld foeBlock 			; 16
	pop h       			; 12
	shld foeBlock + 2 		; 16
	pop h       			; 12
	shld foeBlock + 4 		; 16
	pop h       			; 12
	shld foeBlock + 6 		; 16  = 68
copyfoe_restoresp:
	lxi sp, 0

	call foe_byId
	pop d

	; only the first 4 bytes of foeBlock need to be copied back
copyback_y:
	lxi h, 0 	
	dad sp 		
	shld copyback_return+1 
	xchg		
	lxi d, 4 				; foeBlock size
	dad d
	sphl		

	;lhld foeBlock + 6 		
	;push h
	;lhld foeBlock + 4
	;push h
	lhld foeBlock + 2
	push h
	lhld foeBlock
	push h           

copyback_return:
	lxi sp, 0
	ret


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

;------------------------------------------------------
; Draw 2 lines of black at the bottom of game field
; Clear them at the top, leave 16+16 of black at sides
;------------------------------------------------------
drawblinds_bottom:
	lxi h, $8000 + BOTTOM_HEIGHT - 22 ; 22 ~ enemy height
	mvi e, $00
	; wipe the first 3 layers with zeroes...
	mvi c, 16*3 
	lda frame_scroll
	add l
	mov l, a
	call drawblinds_fill
	
	; ...and the black layer with $ff
	mvi e, $ff 
	mvi c, 16
	mov a, l
	adi 22
	mov l, a

	; fill 2 lines in a meander-like pattern y,y+1,x+1,y+1
drawblinds_fill:
	mov m, e
	inr l    
	mov m, e 
	inr h
	mov m, e
	dcr l
	mov m, e
	inr h
	dcr c 	
	jnz drawblinds_fill
	ret	

clearblinds:
	lxi h, $e2ff-TOP_HEIGHT
	mvi e, 0
	mvi c, 14 ; 28
clearblinds_entry2:
	lda frame_scroll
	add l
	mov l, a
	jmp drawblinds_fill

terrain_current:
terrain_left:				db 4
terrain_water: 				db 12 ; 24
terrain_islandwidth: 		db 0

terrain_next:
terrain_next_left: 			db 4
terrain_next_water: 		db 24
terrain_next_islandwidth: 	db 0
terrain_islandcould: 		db 0
terrain_prev_water:			db 0

pf_blockcount:				db 1; current block in level (max=BLOCKS_IN_LEVEL)
pf_bridgeflag:				db 0; 
pf_roadflag:				db 0
pf_blockline:				db 0


foe_left: db 0
foe_water: db 0
foe_right: db 0

	;; ---------------
	;; Create new foe
	;; ---------------
create_new_foe:
	push psw

	lda randomHi
	mov b, a
	ani $4
	jz  cnf_return
	;lda pf_roadflag
	;ora a
	;jnz cnf_return


	; update bounce boundaries
	lxi h, pf_tableft
	lda frame_scroll
	add l
	sui 8 ; offset to where the terrain is already generated
	mov l, a
	mov a, m
	mov d, a
	sta foe_left
	inr h ; --> pf_tabwater
	mov a, m
	mov e, a
	sta foe_water

	; island?
	add d
	cpi SCREEN_WIDTH_BYTES/2
	jz ubb_doublewater
	; foe_left = width - (left+water)
	mov d, a
	mov a, b
	lda randomLo
	ani $2
	jz cnf_leftandwaterskip

	mov a, d
	cma
	inr a
	adi SCREEN_WIDTH_BYTES
	sta foe_left
	jmp cnf_leftandwaterskip
ubb_doublewater:
	mov a, e
	ora a
	ral
	sta foe_water

	jmp cnf_leftandwaterskip

cnf_leftandwaterskip:

	; get current foe index
	lxi h, foeTableIndex
	mov a, m
	mov b, a
	inr a  			; advance the index
	cpi FOE_MAX
	jnz create_new_foe_L1
	xra a
create_new_foe_L1:
	mov m, a
	; use the original foeTableIndex
	mov a, b
	; get offset foe_1 + foeTableIndex*8
	lxi h, foe_1
	ora a
	ral
	ral
	ral 
	mvi b, 0
	mov c, a
	dad b 		; hl = foe[foeTableIndex]: Id, Column, Index, Direction, Y, Left, Right

	lda pf_roadflag
	ora a
	jz cnf_4

	; create bridge
	lda pf_blockline
	cpi 32
	jnz cnf_return

	mvi m, FOEID_BRIDGE
	inx h
	mvi m, 13 ; column
	inx h
	mvi m, 0 ; index
	inx h
	mvi m, 0 ; direction
	inx h
	lda frame_scroll
	;adi 10 
	mov m, a ; y
	jmp cnf_return

cnf_4:
	lda randomHi
	mov b, a
	ani $3
	inr a
	mov d, a 	; d = foe id

cnf_3:
	; width
	lda foe_water
	mov c, a
	mov a, d
	cpi FOEID_SHIP
	mov a, c
	jnz cnf_width_2
	sui 2
cnf_width_2:
	sui 1
	mov e, a
	; now a = available width

	; get normalized random 0 <= rand < a
	;dcr a
	sui 2
	jz cnf_return  ; bad luck: passage too narrow for this foe
	jm cnf_return 
	mov m, d 	; should fit: store foe id 
	inx h
	call randomNormA

	; Column
	lda foe_left
	;add c         ; offset the foe right
	mov c, a
	mov m, a
	inx h

	; Index = 0
	mvi m, 0
	inx h

	; Direction = 1
	mvi m, 1
	inx h

	; Y = current
	lda frame_scroll
	mov b, a
	;sui TOP_HEIGHT-16 ; make foes appear above the top curtain
	mov m, a
	inx h
	; Left
	lda foe_left
	dcr a
	mov m, a
	mov c, a
	inx h

	; Right
	mov a, e
	add c
	mov m, a
cnf_return:
	pop psw
    ret

    ;;--------------------------------
    ;; Update line: terrain formation
    ;;--------------------------------

update_line:
	; random should be moved outside to make sure that it gets called every frame
	call nextRandom16 ; result in randomHi/randomLo and HL
	; update block line count
	lda pf_blockline
	dcr a
	jp updl_1
	mvi a, BLOCK_HEIGHT
updl_1:
	sta pf_blockline

	lda pf_blockline
	ora a
	jz update_next_block
	mov b, a
	; avoid creating sprites on roll overlap
	lda frame_scroll
	cpi $10
	jc updl_2
	mov a, b
	; create new foe every 32 lines
	ani $f
	cz create_new_foe
updl_2:
	ani $1
	jz update_step
	jmp ustep_out

update_next_block:
	; update block count
	xra a
	sta pf_bridgeflag
	lda pf_blockcount
	dcr a
	sta pf_blockcount
	jz unb_setbridge
	cpi 1
	jnz unb_nosetbridge
	sta pf_bridgeflag
	jmp unb_nosetbridge

unb_setbridge:
	; end the island and set passage to bridge width
	mvi a, BLOCKS_IN_LEVEL
	sta pf_blockcount
	mvi a, HALFBRIDGE_WIDTH
	sta terrain_next_water
	mvi a, SCREEN_WIDTH_BYTES/2 - HALFBRIDGE_WIDTH
	sta terrain_next_left
	jmp updateblock_out

unb_nosetbridge:
	; 
	lda terrain_next_water
	sta terrain_prev_water
	mov a, h
	ani $7
	adi 3
	;mvi a, 2
	sta terrain_next_left
	ral
	mov b, a
	mvi a, SCREEN_WIDTH_BYTES
	sub b
	; divide water/2
	ora a
	rar 
	sta terrain_next_water

	; if we're to terminate the island, make sure the passage is wide enough
	; water - island - 6 >= 0
	mov b, a  ; b = terrain_next_water
	lda terrain_islandwidth
	ora a
	jz unp_A
	mov c, a  ; c = terrain_islandwidth
	mov a, b
	sub c
	sbi NARROWEST
	jp  unp_A 
	; terrain_next_left = screen/2 - island - 6
	; water_next_left = 6
	mvi a, NARROWEST
	sta terrain_next_water
	lda terrain_next_left
	mvi a, 16
	sub c
	sbi NARROWEST
	sta terrain_next_left
unp_A:
	; force terminate the island before the bridge
	lda pf_bridgeflag
	ora a
	jnz updateblock_noisland
	; if not enough water, no island
	lda terrain_next_water
	cpi ENOUGH_FOR_ISLAND ;9
	jc updateblock_noisland

	; we have enough room for island
updateblock_makeisland:
	mov a, l
	rar
	rar
	ani $7
	adi $2
	; make sure that water - island > NARROWEST
	mov b, a
	lda terrain_water 
	sub b
	sbi NARROWEST
	jm  unb_island_morewater
	jz  unb_island_morewater
	lda terrain_next_water
	sub b
	sbi NARROWEST
	jm  unb_island_morewater
	jz  unb_island_morewater
	; the passage needs no tuning
	jmp unb_island_ok
unb_island_morewater:
	; d = max(terrain_left, terrain_next_left)
	lda terrain_left 
    mov d, a
    lda terrain_next_left
    cmp d
    jm .+4 
    mov d, a
 
	; make passage NARROWEST wide: 
	; water = NARROWEST; island = screen/2 - (water+left)
	mvi a, NARROWEST
	sta terrain_next_water
	mov c, a
	mov a, d
	add c   
	mov c, a  ; c = next_water + current_left
	mvi a, 16
	sub c 
	mov b, a
unb_island_ok:
	; check that island width is > 0
	mov a, b
	ora a
	jm updateblock_noisland
	sta terrain_next_islandwidth
	jmp updateblock_out

updateblock_noisland:
	xra a
	sta terrain_next_islandwidth
	sta terrain_islandcould
	jmp updateblock_out

updateblock_out:
	; -- fall through to update_step

	;; -------------------------------------------
	;; Update one scanline of terrain
	;; interpolate between current and next values
	;; -------------------------------------------
update_step:
	; check if we need to set the road flag
	lda pf_blockcount
	cpi BLOCKS_IN_LEVEL
	mvi a, 0
	jnz uss_x1
	inr a
uss_x1:
	sta pf_roadflag

	; widen/narrow the banks
	lhld terrain_next
	xchg
	lhld terrain_current	; d = next_left, e = next_width
	mov a, l
	cmp e
	jz uss1
	jm uss2
	dcr l 		; move left bank 1 left
	inr h 		; make water 1 wider
	shld terrain_current
	jmp uss1
uss2:
	inr l 		; move left bank 1 right
	dcr h 		; make water 1 narrower 
	shld terrain_current
uss1:
	; do the same with the island
	lda terrain_next_islandwidth
	mov b, a
	lda terrain_islandwidth
	cmp b
	jz  uss4 ; next == current, no update
	jp  uss5
	; move island wider, water narrower
	dcr h  ; less water
	inr a  ; more island
	shld terrain_current
	sta terrain_islandwidth
	jmp uss4
uss5:
	inr h  ; more water
	dcr a  ; less island
	shld terrain_current
	sta terrain_islandwidth
uss4:

ustep_out:
	; update boundary tables
	lxi h, pf_tableft
	lda frame_scroll
	;sui 4 ; offset the index hoping that it will be ~ at the middle of foe height
	add l
	mov l, a
	lda terrain_left
	mov m, a
	inr h
	lda terrain_water
	mov m, a

produce_line_main:
	; if no road, just produce regular line
	lda pf_roadflag
	ora a
	jz produce_line

	lda pf_blockline
	cpi BLOCK_HEIGHT-20 	; bottom edge
	jz produce_line_road
	jp produce_line
	cpi BLOCK_HEIGHT-54 	; top edge
	jp produce_line_road
	jz produce_line_road
	jmp produce_line

produce_line_road:
	; if at the border
	jz plr_border
	cpi BLOCK_HEIGHT-(20+(34/2)-1) ; bottom divider line
	jp plr_asphalt
	cpi BLOCK_HEIGHT-(20+(34/2)+1) ; top divider line
	jm plr_asphalt
	jmp plr_yellow
plr_asphalt:
	lxi h, $c0ff+2   ; c+8 = cyan, c+a = near black
	mvi d, $c0+SCREEN_WIDTH_BYTES
	call produce_line_e2
	lxi h, $80ff+2
	mvi d, $80+SCREEN_WIDTH_BYTES
	call produce_line_e2
	ret
plr_yellow:
	lxi h, $80ff+2 	; 8+a = yellow
	mvi d, $80+SCREEN_WIDTH_BYTES
	call produce_line_e2
	lxi h, $a0ff+2
	mvi d, $a0+SCREEN_WIDTH_BYTES
	call produce_line_e2
	ret

plr_border:
	lxi h, $c0ff+2 	
	mvi d, $c0+SCREEN_WIDTH_BYTES
	call produce_line_e2
	lxi h, $a0ff+2
	mvi d, $a0+SCREEN_WIDTH_BYTES
	call produce_line_e2
	ret


produce_line:
	lxi h, $80ff+2; $80ff-TOP_HEIGHT+2
	mvi d, $80+SCREEN_WIDTH_BYTES
produce_line_e2:
	lda frame_scroll
	add l
	mov l, a

	lxi b, $ff00
	lda terrain_left
	dcr a
produce_loop_leftbank:
	mov m, b
	inr h
	dcr a
	jnz produce_loop_leftbank

	lda terrain_water
	ora a
	jz produce_island
produce_loop_leftwater:
	mov m, c
	inr h
	dcr a
	jnz produce_loop_leftwater

produce_island:
	lda terrain_islandwidth
	ora a
	jz produce_rightwater
	ral
produce_loop_island:
	mov m, b
	inr h
	dcr a
	jnz produce_loop_island
produce_rightwater:
	lda terrain_water
	ora a
	jz produce_rightbank
produce_loop_rightwater:
	mov m, c
	inr h
	dcr a
	jnz produce_loop_rightwater

	;ret
produce_rightbank:
	;mvi a, $80+SCREEN_WIDTH_BYTES
	mov a, d
	;add SCREEN_WIDTH_BYTES
	;mov a, d
	sub h
produce_loop_rightbank:
	mov m, b
	inr h
	dcr a
	jnz produce_loop_rightbank

	ret

FOEID_NONE 		equ 0
FOEID_SHIP 		equ 1
FOEID_COPTER 	equ 2
FOEID_RCOPTER	equ 3
FOEID_JET 		equ 4
FOEID_BRIDGE	equ 16

	;; foe class
foeId 			equ 0 			; id: 0 = none, 1 = ship, 2 = copter
foeColumn		equ 1 			; X column
foeIndex		equ 2 			; X offset 0..7
foeDirection	equ 3 			; 1 = LTR, -1 RTL, 0 = not moving
foeY			equ 4			; Y position of sprite start
foeLeftStop		equ 5			; column # of left limit
foeRightStop	equ 6 			; column # of right limit
foeBounce		equ 7 			; bounce flag
foeSizeOf 		equ 8

	;; foe 0 descriptor
foeBlock:
	db 5,0,1,0,$10,3,15
	db 0 						; id
foeBlock_LTR:	
	dw 0 						; dispatch to sprite LTR 
foeBlock_RTL:
	dw 0 						; dispatch to sprite RTL

foePropeller_LTR:
	dw 0
foePropeller_RTL:
	dw 0

foeTableIndex:
	db 0

foe_1:
	db 0 ;FOEID_SHIP
	db 5,0,1,$10,	3,8,0
foe_2:
	db 0 ; FOEID_COPTER
	db 6,0,1,$30,	3,10,0
foe_3:
	db 0; FOEID_JET
	db 5,0,$ff,$50,	3,25,0
foe_4:
	db 0; FOEID_RCOPTER
	db 8,0,1,$70,	3,10,0
foe_5:
	db 0; FOEID_SHIP
	db 19,0,1,$90,	19,24,0
foe_6:
	db 0; FOEID_JET
	db 5,0,1,$b0,	0,0,0
foe_7:
	db 0; FOEID_SHIP
	db 21,0,1,$d0,	19,24,0
foe_8:
	db 0; FOEID_COPTER
	db 9,0,1,$f0,	3,10,0

	;; animate sprites
	;; should be called once per frame, before the first sprite
foe_animate:
	; animate the propeller
	lda frame_number
	ani $2
	jz  foe_byId_propA
	lxi h, propellerB_ltr_dispatch
	lxi d, propellerB_rtl_dispatch
	jmp foe_byId_propC
foe_byId_propA:
	lxi h, propellerA_ltr_dispatch
	lxi d, propellerA_rtl_dispatch
foe_byId_propC:
	shld foePropeller_LTR
	xchg
	shld foePropeller_RTL
	ret

foe_byId:
	lda foeBlock + foeId
	; if (foe.Id == 0) return;
	ora a
	rz
	mov b, a	

	; check Y and clear the foe if below the bottom line
	lda frame_scroll
	mov l, a
	lda foeBlock + foeY 			 
	sub l
	cpi BOTTOM_HEIGHT
	jnc foe_infield
	xra a
	sta foeBlock + foeId
	ret

foe_infield:
	; prepare dispatches
	mov a, b
	dcr b
	jz  foe_byId_ship		; 1 == ship
	dcr b
	jz  foe_byId_copter		; 2 == copter
	dcr b
	jz  foe_byId_rcopter	; 3 == redcopter
	dcr b
	jz 	foe_byId_jet 		; 4 == jet
	; specials
	cpi FOEID_BRIDGE
	jz  foe_byId_bridge
	; default: return  
	ret

foe_byId_bridge:
	lxi h, bridge_ltr_dispatch
	shld foeBlock_LTR
	shld foeBlock_RTL
	jmp foe_frame

foe_byId_ship:
	lxi h, ship_ltr_dispatch
	shld foeBlock_LTR
	lxi h, ship_rtl_dispatch
	shld foeBlock_RTL
	jmp foe_frame

foe_byId_jet:
	lxi h, jet_ltr_dispatch
	shld foeBlock_LTR
	lxi h, jet_rtl_dispatch
	shld foeBlock_RTL
	jmp jet_frame 			; jet has a separate handler

foe_byId_rcopter:
	; draw the copter body
	lxi h, redcopter_ltr_dispatch
	shld foeBlock_LTR
	lxi h, redcopter_rtl_dispatch
	shld foeBlock_RTL
	call foe_frame
	jmp foe_byId_copterpropeller
foe_byId_copter:
	; draw the copter body
	lxi h, copter_ltr_dispatch
	shld foeBlock_LTR
	lxi h, copter_rtl_dispatch
	shld foeBlock_RTL
	call foe_frame
foe_byId_copterpropeller:
	; load animated propeller
	lhld foePropeller_LTR
	shld foeBlock_LTR
	lhld foePropeller_RTL
	shld foeBlock_RTL

	; offset propeller Y position
	lda foeBlock + foeY ; 16
	adi 4 				; 8
	sta foeBlock + foeY ; 16
	push psw 			; 12
	; draw propeller
	call foe_paint_preload 
	; restore copter Y position in the foe block
	pop psw 			; 12
	sta foeBlock + foeY ; 16 = 64
	ret


;; --------------------------
;; Jet frame
;; --------------------------
jet_frame:
	; load Column to e

	lhld foeBlock + foeColumn
	mov e, l ; e = foeColumn
	mov b, h ; b = foeIndex
	mvi h, 0 ; bounce = 0

	lda foeBlock + foeDirection
	mov c, a   ; keep direction in c
	ral
	mvi a, 4
	jnc jet_move_diradd
	mvi a, $fc ; -4
jet_move_diradd:
	add b
	mov b, c   ; restore direction in b
	; if (Index < 0  
	ora a
	jm jet_move_indexoverrun
	;     || Index == 8)
	cpi $8
	jz jet_move_indexoverrun

	; for right screen limit 
	; a here can be only 0 or 4 (we're looking for 4)
	; so check if a + 30 == 34
	mov c, a
	add e
	cpi 34
	jnz jet_move_normal
	xra a
	jmp jet_move_reset_to_a

jet_move_normal:
	; index within column boundary
	; save Index, update c
	mov a, c
	sta foeBlock + foeIndex
	; all done, paint
	jmp foe_paint

jet_move_indexoverrun:
	; {
	; Index = Index % 8
	ani $7
	; save Index, update c
	sta foeBlock + foeIndex
	mov c, a
	; Column = Column + Direction 
	; column in e
	mov a, b
	add e
	sta foeBlock + foeColumn
	mov e, a
	; }
jet_move_checklimits:
	; check for left screen limit
	; we are here if Index == 0 || Index == 7
	; e = Column
	ora a 		; if (a >= 0) 
	mvi a, 29
	jp jet_move_continue ; -->
jet_move_reset_to_a:
	sta foeBlock + foeColumn
	mov e, a
jet_move_continue:
	jmp foe_paint

;; ----------------------------------------------
;; Frame routine for a regular foe: ship, copters
;; ----------------------------------------------
foe_frame:
	mvi h, 0 ; bounce flag in h
foe_Move:
	; load Column to e
	lda foeBlock + foeColumn
	mov e, a 	
	; index = index + direction
	lda foeBlock + foeDirection
	mov b, a
	lda foeBlock + foeIndex
	add b
	; if (Index == -1  
	ora a
	jm foe_move_indexoverrun
	;     || Index == 8)
	cpi $8
	jz foe_move_indexoverrun

	; index within column boundary
	; save Index, update c
	sta foeBlock + foeIndex
	mov c, a
	; not at column boundary -> skip bounce check
	jmp foe_paint

foe_move_indexoverrun:
	; {
	; Index = Index % 8
	ani $7
	; save Index, update c
	sta foeBlock + foeIndex
	mov c, a
	; Column = Column + Direction
	; column in e
	mov a, b
	add e
	sta foeBlock + foeColumn
	mov e, a
	; }
foe_move_CheckBounce:
	; check for bounce
	; we are here if Index == 0 || Index == 7
	; e = Column
	lda foeBlock + foeRightStop
	cmp e
	jz foe_move_yes_bounce

	; emergency door stopper
	jp fm_noemergency
	dcr e
	mov a, e
	sta foeBlock + foeColumn
	xra a
	sta foeBlock + foeDirection
	sta foeBlock + foeIndex
fm_noemergency:
	lda foeBlock + foeLeftStop
	cmp e
	jnz foe_paint
	; yes, bounce
foe_move_yes_bounce:
	; Bounce = 1
	mvi h, 1 
	; Direction = -Direction
	mov a, b
	cma 
	inr a
	sta foeBlock + foeDirection
	; do the Move() once again
	jmp foe_Move

	;; additional entry point for sprites with precalculated position
	;; used for propellers
foe_paint_preload:

	lhld foeBlock + foeColumn
	mov e, l ; e = foeColumn
	mov c, h ; c = foeIndex
	mvi h, 0 ; bounce = 0
	lda foeBlock + foeDirection
	mov b, a
	mvi h, 1 ; 
	;; foeBlock movement calculation ends here

	;; actual paint
foe_paint:
	;; paint foe
	; e contains column
	mov a, e

	; de = base addr ($8000 + foe.Y)
	adi $80
	mov d, a
	lda foeBlock + foeY
	mov e, a

	; de == base address
	; c == index
	; b == Direction
	xra a
	ora b
	jm  sprite_rtl
	; fallthrough to sprite_ltr

	;; c = Index
	;; de = column base address
	;; h = bounce
sprite_ltr:
	; if (index != 0 || Bounce) regular();
	xra a
	ora c
	ora h
	jnz sprite_ltr_regular

	; Index == 0, no bounce -> wipe previous column
	dcr d
	; load dispatch table	
	lhld foeBlock_LTR
	jmp sprite_ltr_rtl_dispatchjump

	;; Draw ship without prepending column for wiping
sprite_ltr_regular:
	; load dispatch table	
	lhld foeBlock_LTR
	; index 0..7 -> 1..8
	inr c
	jmp sprite_ltr_rtl_dispatchjump

	;; c = offset
	;; de = column base address
sprite_rtl:
sprite_rtl_regular:
	lhld foeBlock_RTL

sprite_ltr_rtl_dispatchjump:
	mvi b, 0
	mov a, c
	rlc
	mov c, a
	dad b 		
	;jmp [h]
	shld .+4
	lhld 0000	
	pchl

sprites_scratch:	dw 0 	; saved SP for various stack-abusing routines

    .include ship.inc
    .include random.inc

c_black		equ $00
c_blue		equ $c1
c_green 	equ $73 ; 01 110 011
c_yellow 	equ $bf ; 
c_magenta 	equ $94 ; $8d ;
c_white		equ $f6
c_grey		equ $09 ; 00 010 010  -- also $52 the darkest neutral gray
c_cyan		equ $ab ; $ad ; $f4	; 10 011 001 - $ad is neutral gray
c_dkblue	equ $81	; 10 010 001

palette_data:
	db c_blue,   	c_black,  c_white,  	c_black
	db c_magenta, 	c_black,  c_grey,		c_black
	db c_green, 	c_black,  c_cyan,  		c_black
	db c_yellow,    c_black,  c_dkblue, 	c_black


	;; Depuración y basura

	; pintar los colores
showlayers:
	lxi h, $ffff
	; 1 0 0 0
	shld $81fe 

	; 0 1 0 0 
	shld $a2fe

	; 1 1 0 0
	shld $83fe
	shld $a3fe

	; 0 0 1 0
	shld $c4fe

	; 1 0 1 0
	shld $85fe	
	shld $c5fe

	; 0 1 1 0
	shld $a6fe	
	shld $c6fe

	; 1 1 1 0
	shld $87fe	
	shld $a7fe
	shld $c7fe

	; x x x 1
	shld $e8fe	

	; test bounds
	shld $8306
	shld $8308
	shld $830a
	shld $830c
	shld $830e
	shld $8310
	shld $8312

	shld $9306
	shld $9308
	shld $930a
	shld $930c
	shld $930e
	shld $9310
	shld $9312

	; time marks
	lxi h, $c0c0
	shld $e000
	shld $e010
	shld $e020
	shld $e030
	shld $e040
	shld $e050
	shld $e060
	shld $e070
	shld $e080
	shld $e090
	shld $e0a0
	shld $e0b0
	shld $e0c0
	shld $e0d0
	shld $e0e0
	shld $e0f0
	shld $e0fe
	ret

pf_tableft		equ $7800
pf_tabwater		equ $7900
;pf_tableft:					
;							.org .+$100
;pf_tabwater:
;							.org .+$100
