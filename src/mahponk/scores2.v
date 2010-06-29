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
// Design File: scores2.v
// Score keeper and character generator.
//
// Pins description:
//	clk			input	master clock
//	neglineclk	input	line clock (hsync)
//	realx		input	x-coordinate
// 	realy		input	y-coordinate
//	score_addA	input	left score increment
//	score_addB	input	right score increment
//	score_reset	input	score reset
//	scoreA		output	left score bcd value
// 	scoreB		output	right score bcd value
// 	score_scan	output 	score scan
//
// Contains additional modules:
// module bcdcounterx(reset, c, cout, q);
// module unicounter(c, r, en, q1, q2, q3);
// module mux2x4(sel, d0, d1, d2, d3, q);    -- can be used instead of scorecopymux macro
// module textramx(clk, d, a, q);
// module charrom(clk, char, line, q);
// module digit(scanline, pixels);
// module counter_mod4(c, r, q, cout);
// module counter_mod8(c, r, q, cout);



module scores(clk, neglineclk, realx, realy, score_addA, score_addB, score_reset, scoreA, scoreB, score_scan);
input clk;
input neglineclk;
input [9:0] realx, realy;
input score_addA, score_addB, score_reset;
output [7:0]scoreA, scoreB;
output score_scan;

// score counters
wire scout1, scout2, scout3, scout4;
wire [3:0] score0;
wire [3:0] score1;
wire [3:0] score2;
wire [3:0] score3;
bcdcounterx bcdcounter1(score_reset,	score_addA, scout1, score1);
bcdcounterx bcdcounter2(score_reset,	scout1, scout2, score0);

bcdcounterx bcdcounter3(score_reset,	score_addB, scout3, score3);
bcdcounterx bcdcounter4(score_reset,	scout3, scout4, score2);

reg [7:0] scoreA, scoreB;
always @(score0 or score1 or score2 or score3) begin
	scoreA <= {score0,score1};
	scoreB <= {score2,score3};
end

parameter XSTART = 640/2-3*8*4;
parameter YSTART = 6*8;

reg rscxr;
wire scxr = rscxr; //= realx == XSTART;		// reset of x counters
always @(negedge clk) begin
	if (realx == XSTART) rscxr <= 1;
	else rscxr <= 0;
end

wire newline = !neglineclk;//realx == 0;
wire scyr = realy == YSTART;	// reset of y counters

reg [7:0] xwide;
always @(posedge clk) begin
	if (scxr) 
		xwide <= 6*8*4;
	else
		if (xwide > 0) xwide <= xwide - 1;
end

wire display_ax = xwide != 0;
wire display_ay = realy >= YSTART && realy < YSTART+5*8;

wire displayactive = display_ax & display_ay & skippy;


// leap skippy 
reg skippy;
always @(posedge clk) begin
	if (realx == XSTART + 2*8*4) begin
		skippy <= 0;
	end else if (realx == XSTART + 4*8*4) begin
		skippy <= 1;
	end
end

wire [3:0] dummy;
wire [1:0] bitnumber;	// this selects the bit from charrom out
wire [1:0] textadr;
unicounter unicounter(clk, scxr, skippy & display_ax, dummy[2:0], bitnumber, textadr);

wire [2:0] y;
unicounter ycounter(newline, scyr, display_ay, dummy[2:0], y[1:0], {dummy[0],y[2]});

// mux score counters
wire [3:0] textd; // this goes into text ram
//mux2x4 muxscorbles(textadr, score[0], score[1], score[2], score[3], textd);
scorecopymux scorecopymux(score0, score1, score2, score3, textadr, textd);

// the character rom and pixels
wire [3:0] pixels;
charrom charrom(clk, charcode, y, pixels);

// where the actual digits are!
wire [3:0] charcode;
textramx displaymem(clk, textd, textadr, charcode);

reg sscan;
always @(posedge clk) begin
	sscan <= displayactive & pixels[bitnumber];
end
assign score_scan = sscan;
//assign score_scan = displayactive & pixels[bitnumber];

endmodule

module bcdcounterx(reset, c, cout, q);
input reset, c;
output reg cout;
output reg [3:0]q;

always @(posedge reset or posedge c) begin
	if (reset) {cout, q} <= 0;
	else begin
		if (q == 9)
			{cout, q} <= {5'b10000};
		else 
			{cout, q} <= q + 1'b1;
	end
end
endmodule

module unicounter(c, r, en, q1, q2, q3);
	input c, r, en;
	output [2:0] q1;
	output [1:0] q2;
	output [1:0] q3;

	reg [6:0]q;
	assign q1 = q[2:0];
	assign q2 = q[4:3];
	assign q3 = q[6:5];
	
	always @(posedge c)
		if (r)
			q <= 0;
		else if (en)
			q <= q + 1'b1;
endmodule

module mux2x4(sel, d0, d1, d2, d3, q);
	input [1:0] sel;
	input [3:0] d0, d1, d2, d3;
	output [3:0] q;
	
	assign q = sel == 2'b00 ? d0 :
					  2'b01 ? d1 :
					  2'b10 ? d2 : d3;
endmodule

module textramx(clk, d, a, q);
	input clk;
	input [3:0] d;
	input [1:0] a;
	output [3:0] q;
	
	reg [3:0] ram[3:0];	// 4 locations 4-bit wide each
	reg [3:0] q;
	//assign q = ram[a];

//	always @(posedge clk) begin
//		q <= ram[a];
//		ram[a] <= d;
//	end
	always @(d or a) begin
		q <= ram[a];
		ram[a] <= d;
	end
endmodule

module charrom(clk, char, line, q);
	input clk;
	input [3:0] char;
	input [2:0] line;

	output reg[3:0] q;
	always @(char or pixels0 or pixels1 or pixels2 or pixels3 or pixels4 or pixels5 or pixels6 or pixels7 or pixels8 or pixels9)
		case (char)
			4'd0 : q <= pixels0;
			4'd1 : q <= pixels1;
			4'd2 : q <= pixels2;
			4'd3 : q <= pixels3;
			4'd4 : q <= pixels4;
			4'd5 : q <= pixels5;
			4'd6 : q <= pixels6;
			4'd7 : q <= pixels7;
			4'd8 : q <= pixels8;
			4'd9 : q <= pixels9;
			default:
				q <= 0;
		endcase

	wire [3:0] pixels0;
	digit #(
		4'b1110,
		4'b1010,
		4'b1010,
		4'b1010,
		4'b1110
				) dzigit0(line, pixels0);
	wire [3:0] pixels1;
	digit #(
		4'b1100,
		4'b0100,
		4'b0100,
		4'b0100,
		4'b1110
				) dzigit1(line, pixels1);
	wire [3:0] pixels2;
	digit #(
		4'b1110,
		4'b0010,
		4'b1110,
		4'b1000,
		4'b1110
				) dzigit2(line, pixels2);
	wire [3:0] pixels3;
	digit #(
		4'b1110,
		4'b0010,
		4'b0110,
		4'b0010,
		4'b1110
				) dzigit3(line, pixels3);
	wire [3:0] pixels4;
	digit #(
		4'b1010,
		4'b1010,
		4'b1110,
		4'b0010,
		4'b0010
				) dzigit4(line, pixels4);
	wire [3:0] pixels5;
	digit #(
		4'b1110,
		4'b1000,
		4'b1110,
		4'b0010,
		4'b1110
				) dzigit5(line, pixels5);
	wire [3:0] pixels6;
	digit #(
		4'b1110,
		4'b1000,
		4'b1110,
		4'b1010,
		4'b1110
				) dzigit6(line, pixels6);
	wire [3:0] pixels7;
	digit #(
		4'b1110,
		4'b0010,
		4'b0010,
		4'b0010,
		4'b0010
				) dzigit7(line, pixels7);
	wire [3:0] pixels8;
	digit #(
		4'b1110,
		4'b1010,
		4'b1110,
		4'b1010,
		4'b1110
				) dzigit8(line, pixels8);
	wire [3:0] pixels9;
	digit #(
		4'b1110,
		4'b1010,
		4'b1110,
		4'b0010,
		4'b1110
				) dzigit9(line, pixels9);
endmodule

module digit(scanline, pixels);
	input [2:0] scanline;
	output [3:0] pixels;
	parameter a=0,b=0,c=0,d=0,e=0;
	assign pixels = scanline == 3'd0 ? 		   {a[0],a[1],a[2],a[3]} :
        				scanline == 3'd1 ? {b[0],b[1],b[2],b[3]} :
        				scanline == 3'd2 ? {c[0],c[1],c[2],c[3]} :
        				scanline == 3'd3 ? {d[0],d[1],d[2],d[3]} :
        				scanline == 3'd4 ? {e[0],e[1],e[2],e[3]} : 4'b0000;
endmodule

module counter_mod8(c, r, q, cout);
	input c, r;
	output [2:0] q;
	output reg cout;
	
	reg[2:0] ctval;
	assign q = ctval;
	always @(posedge c or posedge r) 
		if (r) 
			{cout, ctval} <= 0;
		else 
			{cout,ctval} <= ctval + 1'b1;
endmodule

module counter_mod4(c, r, q, cout);
	input c, r;
	output reg[1:0] q;
	output reg cout;
	always @(posedge c)
		if (r)
			{cout, q} <= 0;
		else
			{cout, q} <= q + 1'b1;
endmodule

// $Id: scores2.v,v 1.9 2007/08/27 22:14:51 svo Exp $
