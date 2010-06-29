// ====================================================================
//                              MAH PONK
//
// Copyright (C) 2007, Viacheslav Slavinsky
// This design and core is distributed under modified BSD license. 
// For complete licensing information see LICENSE.TXT.
// -------------------------------------------------------------------- 
// An open table tennis game for VGA displays.
//
// Author: Viacheslav Slavinsky, http://sensi.org/~svo
// 
// Design File: vgascan.v, instantiated by vga.v
// Generate VGA sync and provide screen coordinates + active flag.
//
// Pins description:
//	clk			input	master clock input, 25.178MHz for 60Hz 640x480
//	hsync		output	VGA horizontal sync
//	vsync		output	VGA vertical sync
// 	realx		output	user X coordinate
// 	realy  		output 	user Y coordinate
//	videoActive	output	1 when scanning display area
//	pre_xstart	output	used for border scan
//	pre_xend	output	used for border scan
//
//  Unlike typical VGA scanning example, this one is optimized to reduce
//	the amount of arithmetic comparisons and optimize the scan for
// 	slower FPGA's such as FLEX 6000.
//
//	Inside there are 2 state machines, for X and for Y scan. Machine
// 	states correspond to the area the beam is in: front/back porch, sync pulse, 
// 	scan, etc. Each state sets up following state time.
//
//  For detailed description of VGA signals and timing see:
// 		http://www.epanorama.net/documents/pc/vga_timing.html
//		http://server.oersted.dtu.dk/personal/sn/31002/?Materials/vga/main.html
//
module vgascan(clk, hsync, vsync, realx, realy, videoActive, pre_xstart, pre_xend);

parameter SCREENWIDTH = 640;	// constants, don't even try using other values
parameter SCREENHEIGHT = 480;

input clk;
output hsync;
output vsync;
output reg[9:0] realx;  
output reg[9:0] realy;
output videoActive;
output pre_xstart, pre_xend;

reg[2:0] scanxx_state;		// x-machine state
reg[2:0] scanyy_state;		// y-machine state
reg[9:0] scanxx;			// x-state timer/counter
reg[9:0] scanyy;			// y-state timer/counter

reg videoActiveX;			// 1 == X is within visible area
reg videoActiveY;			// 1 == Y is within visible area
wire videoActive = videoActiveX & videoActiveY;
reg pre_xstart, pre_xend;

parameter state0 = 3'b000, state1 = 3'b001, state2 = 3'b010, state3 = 3'b011, state4 = 3'b100;

assign hsync = !(scanxx_state == state2);
assign vsync = !(scanyy_state == state2);

reg scanyy_minus;			// todo: investigate if this is still necessary

always @(posedge clk) begin
		if (scanyy == 0) begin 
			case (scanyy_state)
			state0:
					begin
						scanyy <= 11;
						scanyy_state <= state1;
						videoActiveY <= 0;
					end
			state1: 
					begin
						scanyy <= 2;
						scanyy_state <= state2;
					end
			state2:
					begin
						scanyy <= 32;
						scanyy_state <= state3;
					end
			state3:
					begin
						scanyy <= SCREENHEIGHT;
						scanyy_state <= state0;
						videoActiveY <= 1;
						realy <= 0;
					end
			default:
					begin
						scanyy_state <= state0;
					end
			endcase
		end 
		else if (scanyy_minus) begin
			scanyy_minus <= 1'b0;
			scanyy <= scanyy - 1'b1;
		end

		if (scanxx == 0) begin	
			case (scanxx_state) 
			state0: 	
					begin
						// here, commented out is time correction for different clock rate
						scanxx <= 10'd16 /*+ 10'd74*/ - 1'b1;		
						scanxx_state <= state1;
						realy <= realy + 1'b1;
						scanyy_minus <= 1'b1;
						videoActiveX <= 1'b0;
					end
			state1: 
					begin
						scanxx <= 10'd96 - 1'b1; 
						scanxx_state <= state2;
					end
			state2:	// before scan
					begin
						scanxx <= 10'd48 - 1'b1;
						scanxx_state <= state3;
					end
			state3:	
					begin
						pre_xstart <= 1'b1;
						videoActiveX <= 1'b1;
						realx <= 1'b0;
						scanxx <= SCREENWIDTH - 2;
						scanxx_state <= state4;
					end
			state4:
					begin
						pre_xend <= 1'b1;
						//scanxx <= 0;
						scanxx_state <= state0;
					end
			default: 
					begin
						scanxx_state <= state0;
					end
			endcase
		end 
		else scanxx <= scanxx - 1'b1;
		
		if (pre_xstart != 0) pre_xstart <= pre_xstart - 1'b1;
		if (pre_xend != 0) pre_xend <= pre_xend - 1'b1;

		if (videoActiveX) 
			realx <= realx + 1'b1;
end
	
endmodule

// $Id: vgascan.v,v 1.12 2007/08/27 22:14:53 svo Exp $
