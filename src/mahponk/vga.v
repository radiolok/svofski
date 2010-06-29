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

//reg [26:0] cnt;
//always @(posedge clk) cnt <= cnt + 1'b1;
//assign LED1 = cnt[25] == 1;

// instantiate paddles
wire paddle_scan;
wire ball_scan;

wire[9:0] paddleA_y;
wire[9:0] paddleB_y_manual;
wire[9:0] paddleB_y_robot;
reg [9:0] paddleB_y;
reg playerBswitch;

//
// make clocks
//
reg clkdiv8;
reg [6:0] clkdiv8_cnt;
always @(posedge clk) begin
	if (clkdiv8_cnt == 0)
		clkdiv8 <= !clkdiv8;
	clkdiv8_cnt <= clkdiv8_cnt - 1'b1;
end
	
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
// instantiate reset generator
//
wire resetpulse;
wire gamereset;
resetgen resetgen(clk, resetpulse, 1);
button2 #(8192) gameresetter(clkdiv8, gamereset, BATON1);
button2 #(8192) button2(clkdiv8, playerBbutton, BATON2);

always @(posedge playerBbutton or posedge resetpulse) begin
	if (resetpulse) 
		playerBswitch <= 1;
	else 
		playerBswitch <= !playerBswitch;
end



//assign LED1 = !resetpulse;
assign LED2 = playerBswitch;

// VGA is here
wire[9:0] realx;
wire[9:0] realy;
wire videoActive;
wire xscanstart, xscanend;
vgascan vgascan(clk, HSYNC, VSYNC, realx, realy, videoActive, xscanstart, xscanend);


//assign LED1 = collide;	




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


//
// ball scan magic
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

 