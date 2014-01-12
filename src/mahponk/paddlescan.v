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
// Design File: paddlescan.v, instantiated by tehgame.v
// Generates paddle scan signal.
//
// Pins description:
//	clk			input	master clock
//	ptop		input	paddle top coordinate
//	pbot		input	paddle bottom coordinate
//	x, y		input	screen x,y
// 	scan		ouput	paddle scan
//
module paddlescan(clk, ptop, pbot, x, y, scan);
	parameter H = 0;
	parameter X = 0;
	input clk;
	input [9:0] ptop;
	input [9:0] pbot;
	input [9:0] x;
	input [9:0] y;
	output scan;

	reg [4:0] scanX;
	reg scanY;
	
	always @(posedge clk) begin
		if (x == X) 
			scanX = 16;
		if (scanX > 0)
			scanX = scanX - 1'b1;
	end
	always @(posedge clk) begin
		if (y == ptop)
			scanY = 1;
		if (y >= pbot)
			scanY = 0;
	end
	assign scan = scanX != 0 && scanY;
endmodule

// $Id: paddlescan.v,v 1.4 2007/08/27 22:14:49 svo Exp $
