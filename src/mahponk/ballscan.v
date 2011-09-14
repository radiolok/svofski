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
// Design File: ballscan.v, instantiated by tehgame.v
// Generates ball scan signal.
//
// Pins description:
//	clk			input	master clock
//	screenx		input	x-scan coordinate
//	screeny 	input	y-scan coordinate
//	ball_x		input	ball position
//	ball_y		input	ball position
//	ball_scan	output	ball scan signal
//
module ballscan(clk, screenx, screeny, ball_x, ball_y, ball_scan);
	parameter BALLSIZE=8;
	input clk;
	input [9:0] screenx;
	input [9:0] screeny;
	input [9:0] ball_x;
	input [9:0] ball_y;
	output ball_scan;

	reg [3:0] ballscanX;
	reg 	  ballscanY;

	always @(posedge clk) begin
		if (screenx == ball_x-BALLSIZE/2)
			ballscanX = 12;
		if (ballscanX > 0)
			ballscanX = ballscanX - 1'b1;
	end
	always @(posedge clk) begin
		if (screeny == ball_y-BALLSIZE/2)
			ballscanY = 1;
		if (screeny == ball_y+BALLSIZE/2)
			ballscanY = 0;
	end
	assign ball_scan = ballscanX != 0 && ballscanY;
endmodule

// $Id: ballscan.v,v 1.4 2007/08/27 22:14:47 svo Exp $
