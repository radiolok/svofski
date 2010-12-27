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
// This is the main design unit.
// Pins description:
//     	clk		in		master clock input, assign to dedicated input 
//						when possible. Clock should be 25.178MHz
//						for proper 60Hz	VGA display
//		LED1	out		LED1 used to display game status
//		LED2	out		LED2 used to display player 2 mode
// 		VSYNC, HSYNC, RED, GREEN, BLUE outputs are VGA signals
//		MISO	in		SPI master input
//		MOSI	out		SPI master output
//		SCK		out		SPI clock
//		SCS		out		SPI slave chip select for ADC
//		SOUND	out		1-bit sound output
//		BATON1 	input	active low, START button
//		BATON2	input	active low, ROBO-HAND button

module vga(clk, LED1, LED2, VSYNC, HSYNC, RED, GREEN, BLUE, MISO, MOSI, SCK, SCS, SOUND, BATON1, BATON2);
input clk;

output LED1;
output LED2;

output VSYNC;
output HSYNC;
output RED, GREEN, BLUE;

input MISO;
output MOSI;
output SCK;
output SCS;
output SOUND;

input BATON1;
input BATON2;

wand RED;
wand GREEN;
wand BLUE;

parameter SCREENWIDTH = 10'd640;
parameter SCREENHEIGHT = 10'd480;
parameter PADDLESIZE = 10'd64;
parameter BALLSIZE = 8;

// instantiate paddles
wire paddle_scan;
wire ball_scan;

wire[9:0] paddleA_y;			// paddle A y-coordinate
wire[9:0] paddleB_y_manual;		// paddle B y-coordinate as per ADC input
wire[9:0] paddleB_y_robot;		// paddle B y-coordinate as per Robo-Hand
reg [9:0] paddleB_y;			// paddle B y-coordinate result
reg playerBswitch;				// 1 == paddle B is played by hunam

//
// clkdiv8 is an additional, slower clock
//
reg clkdiv8;					
reg [6:0] clkdiv8_cnt;
always @(posedge clk) begin
	if (clkdiv8_cnt == 0)
		clkdiv8 <= !clkdiv8;
	clkdiv8_cnt <= clkdiv8_cnt - 1'b1;
end

//
// paddle clock for robo-hand and ball clocks	
//
reg [7:0] paddle_cnt;				
reg [6:0] ball_cnt;
reg paddleAdvance;
reg ballAdvance;
always @(posedge clkdiv8) begin
	paddle_cnt <= paddle_cnt - 1'b1;
	ball_cnt <= ball_cnt - 1'b1;
	if (ball_cnt == 0) ball_cnt <= 7'b1100000;
end

always @(posedge clk) paddleAdvance = paddle_cnt == 0;
always @(posedge clk) ballAdvance = ball_cnt == 0;

//
// Reset Generator and button debouncers
//
wire resetpulse;			// initial set pulse
wire gamereset;				// start game
resetgen resetgen(clk, resetpulse, 1);	
button2 #(8192) gameresetter(clkdiv8, gamereset, BATON1);
button2 #(8192) button2(clkdiv8, playerBbutton, BATON2);

// Handle player B Hunam/Robo-Hand switch
always @(posedge playerBbutton or posedge resetpulse) begin
	if (resetpulse) 
		playerBswitch <= 1;
	else 
		playerBswitch <= !playerBswitch;
end


// LED2 simply shows if human player is active. 
// For LED1 driver see tehgame.v
assign LED2 = playerBswitch;

// VGA scan instantiated here
wire[9:0] realx;				// user area X coordinate
wire[9:0] realy;				// user area Y coordinate
wire videoActive;				// 1 == beam in user area
wire xscanstart, xscanend;		// helper signals used for border scan
vgascan vgascan(clk, HSYNC, VSYNC, realx, realy, videoActive, xscanstart, xscanend);

// Analog input for player paddles
analinput #(PADDLESIZE, SCREENHEIGHT) analinput(clkdiv8, paddleA_y, paddleB_y_manual, MISO, MOSI, SCS, SCK);

wire[9:0] ball_y;
robohand #(PADDLESIZE,SCREENHEIGHT) paddleB(resetpulse, paddleAdvance, ball_y, paddleB_y_robot);

always @(posedge clkdiv8) begin
	paddleB_y <= playerBswitch ? paddleB_y_manual : paddleB_y_robot;
end

tehgame teh(clk, clkdiv8, resetpulse, gamereset, HSYNC, realx, realy, paddleA_y, paddleB_y, ballAdvance, {ball_scan, paddle_scan, score_scan}, ball_y, SOUND, LED1);

wire border_scan;
borderscan #(640,480) borderscan(clk, xscanstart, xscanend, realy, border_scan);
	
reg [3:0] netcnt;
always @(posedge clk) begin
	if (realx == SCREENWIDTH/2 && realy[1])
		netcnt = 4;
	if (netcnt != 0) netcnt = netcnt - 1'b1;
end
wire net_scan = netcnt != 0;


//scores scoreA(clk, HSYNC, realx, realy, outB, outA, 1'b0, score_scan);

wire bgr, bgg, bgb;
bgfill bgfill(clk, HSYNC, VSYNC, bgr, bgg, bgb);

reg red, green, blue;
always @(negedge clk) begin
	red 	<= videoActive & (bgr | paddle_scan | border_scan | ball_scan | score_scan);
	green 	<= videoActive & (bgg | paddle_scan | border_scan | ball_scan | net_scan | score_scan);
	blue 	<= videoActive & (bgb | paddle_scan | border_scan | ball_scan);
end

assign RED = red;
assign GREEN = green;
assign BLUE = blue;

endmodule

module borderscan(clk, xstart, xend, realy, q);
	parameter SCREENWIDTH = 0;
	parameter SCREENHEIGHT = 0;
	input clk, xstart, xend;
	input [9:0] realy;
	output q;

	assign q = xstart | xend | realy == 0 | realy == SCREENHEIGHT - 1;
endmodule

module bgfill(clk, hsync, vsync, r, g, b);
	input clk, hsync, vsync;
	output r, g, b;
	reg ff1, ff2;
	always @(negedge clk) begin
		if (vsync)
			ff1 <= ~ff1;
		else ff1 <= 0;
	end
	always @(negedge hsync) begin
		if (vsync)
			ff2 <= ~ff2;
		else ff2 <= 0;
	end
	assign r = (1'b0 &  ~(ff1^ff2));
	assign g = (1'b0 & (ff1^ff2));
	assign b = (1'b1 & (ff1^ff2));
endmodule

module collider(cclk, bx, by, ptop, pbot, reset, collide);
	parameter px = 0;
	parameter pw = 16;

	
	input cclk;
	input [9:0]bx;
	input [9:0]by; 
	input [9:0]ptop;
	input [9:0]pbot;
	input reset;
	output reg collide;

	always @(posedge cclk or posedge reset) begin
		if (cclk) begin
			collide <= (bx >= px && bx <= (px + pw)) && (by+4 >= ptop && by-4 <= pbot); 
		end 
		else begin
			collide <= 0;
		end
	end
endmodule


// $Id: vga.v,v 1.27 2007/08/27 22:12:30 svo Exp $