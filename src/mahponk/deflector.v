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
// Design File: deflector.v, instantiated by tehgame.v
// Generates paddle deflection @posedge collide.
//
// Pins description:
//	collide		input	trigger
//	ball_y		input	ball y-coordinate
//	paddle_y	input	paddle y-coordinate
//	deflect		output	deflection "angle", abs + sign
//
module deflector(collide, ball_y, paddle_y, deflect);
	input collide;
	input [9:0] ball_y;
	input [9:0] paddle_y;
	output reg[9:0] deflect;
	
	always @(posedge collide) begin
		deflect = ball_y - paddle_y;
		if (deflect[9]) 
			deflect[8:0] = ~deflect[8:0] + 1;
	end
endmodule

// $Id: deflector.v,v 1.4 2007/08/27 22:14:49 svo Exp $
