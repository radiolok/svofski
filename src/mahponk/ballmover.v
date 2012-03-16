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
// Design File: ballmover.v, instantiated by tehgame.v
// Calculates ball movement thoroughout the game.
//
// Pins description:
//	clk				input	master clock
//	reset			input	reset the ball and inner state
//	advance			input	ball advance clock
//	ball_x			input	ball x-coordinate
//	ball_y			input	ball y-coordinate
//	collide			input	paddle collision signal
//	collidereset	output	collision acknowledge/reset signal
//	deflect			input	deflection angle
//	set_side		input 	ball service side
//	outA, outB		output	ball out left and right signals, mutually exclusive
//	wall			output	up/down wall hit signal
//
module ballmover(clk, reset, advance, ball_x, ball_y, collide, collidereset, deflect, set_side, outA, outB, wall);
	parameter SCREENWIDTH = 640;
	parameter SCREENHEIGHT = 480;
	parameter BALLSIZE = 8;
	
	input clk;
	input reset;
	input advance;
	output reg [9:0] ball_x;
	output reg [9:0] ball_y;
	input collide;
	output reg collidereset;
	input [9:0] deflect;
	input set_side;
	output reg outA, outB, wall;

	reg sgnX;
	reg sgnY;
	parameter speedX = 4'd8;
	reg [3:0] speedY = 8;
	
	reg [3:0] lsb_x;
	reg [3:0] lsb_y;
	
	wire[3:0] woot;
	assign woot[1:0] = {(ball_x < BALLSIZE/2), (ball_x > SCREENWIDTH - BALLSIZE/2)};
	assign woot[3:2] = {(ball_y < BALLSIZE/2), (ball_y > SCREENHEIGHT - BALLSIZE/2)};
	
	always @(posedge advance or posedge reset) begin
		if (reset) begin 
			//if (outA) {ball_x,lsb_x} = {SCREENWIDTH/3, 4'b0};
			//else if (outB) {ball_x, lsb_x} = {(SCREENWIDTH-SCREENWIDTH/3), 4'b0};
			//else {ball_x, lsb_x} = {SCREENWIDTH/2, 4'b0};
			{ball_x, lsb_x} = {(set_side ? (SCREENWIDTH/3) : (SCREENWIDTH-SCREENWIDTH/3)), 4'b0};
			{ball_y, lsb_y} = SCREENHEIGHT/2 * 16;
			speedY = 1;
			outA = 0;
			outB = 0;
		end 
		else begin 
			if (collide) begin
				sgnX = !sgnX;
				sgnY = ~deflect[9];
				speedY = deflect[5:2];
				collidereset = 1;
			end 
				else collidereset = 0;
			
			if (woot[1]) begin ball_x = BALLSIZE/2; sgnX = 1; outA = 1; end
			if (woot[0]) begin ball_x = 640-BALLSIZE/2; sgnX = 0; outB = 1; end
			if (woot[3]) begin ball_y = BALLSIZE/2; sgnY = 1; end
			if (woot[2]) begin ball_y = 480-BALLSIZE/2; sgnY = 0; end
			
			{ball_x,lsb_x} = {ball_x,lsb_x} + (sgnX ? speedX : -speedX);					
			{ball_y,lsb_y} = {ball_y,lsb_y} + (sgnY ? speedY : -speedY);
			wall <= woot[2] | woot[3];
			//outA <= woot[1];
			//outB <= woot[0];
		end
	end
endmodule

// $Id: ballmover.v,v 1.10 2007/08/27 22:14:47 svo Exp $
