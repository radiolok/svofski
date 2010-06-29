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

module robohand(reset, advance, ball_y, paddle_y);
	// paddle Y location (0-480)
	parameter PADDLESIZE   = 10'd0;
	parameter SCREENHEIGHT = 10'd0;

	input reset;
	input advance;
	input [9:0] ball_y;
	output reg[9:0] paddle_y;

	// paddle move direction 0 = +, 1 = -
	reg paddleDir;
	reg [9:0] ptop;
	reg [9:0] pbot;
	reg [2:0] increment;
	
	always @(negedge advance or posedge reset) begin
		if (reset) begin
			paddle_y <= SCREENHEIGHT/2;
		end
		else begin
			if (ball_y < ptop || ball_y > pbot) 
				paddle_y <= paddlelimiter(paddleDir == 1'b0 ? paddle_y + 1'b1 : paddle_y - 1'b1);
		end
	end
	
	always @(posedge advance) begin
		ptop <= paddle_y - PADDLESIZE/2;
		pbot <= paddle_y + PADDLESIZE/2;
		paddleDir <= ball_y < paddle_y;
	end
	
	function [9:0] paddlelimiter;
	input [9:0] py;
	begin
		if (py < PADDLESIZE/2) 
			paddlelimiter = PADDLESIZE/2;
		else 
		if (py > SCREENHEIGHT-PADDLESIZE/2)
			paddlelimiter = SCREENHEIGHT-PADDLESIZE/2;
		else
			paddlelimiter = py;
	end
	endfunction
	
endmodule


// $Id: robohand.v,v 1.4 2007/08/27 22:14:50 svo Exp $
