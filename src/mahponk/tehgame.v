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
// Game logic unit. What happens here:
//	-- game phases, before start, start, score keeping, endgame
//	-- sound generation
//	-- LED1 blinking
//	All game-related units are instantated here, namely:
// 		ballscan
//		paddlescan
//		deflector
//		soundnik
//		scores
//		ballmover
//
// Pins description:
//	clk			input 	master clock
//	clkdiv8		input	slow clock
//	gamereset	input	game reset pulse
//	hsync		input	vga horizontal sync (passed to scores.v)
//	realx,realy	input 	x,y screen coordinates
//	paddleA_y	input	paddle A y-coordinate
// 	paddleB_y	input	paddle B y-coordinate
// 	ballAdvance	input	ball clock
//	scans[2:0]	output	{ball_scan, paddle_scan, score_scan}
//	ball_y		output 	ball_y coordinate for Robo-Hand!
//	sound		output	now with 3 sounds!
// 	led			output 	game status led
module tehgame(clk, clkdiv8, reset, gamereset, hsync, realx, realy, paddleA_y, paddleB_y, ballAdvance, scans, ball_y, sound, led);

parameter SCREENWIDTH = 10'd640;
parameter SCREENHEIGHT = 10'd480;
parameter PADDLESIZE = 10'd64;
parameter BALLSIZE = 8;

input clk, clkdiv8;
input reset;
input gamereset;
input hsync;
input [9:0] realx;
input [9:0] realy;
input [9:0] paddleA_y;
input [9:0] paddleB_y;
input ballAdvance;
output [2:0] scans;
output [9:0] ball_y;
output sound;
output led;

reg [7:0] ledcounter;
reg 	  ledled;

assign led = ledled;

// 
// All sound is here. 
//
reg [3:0] sfx; 		// which sound to play
reg keydown;   		// initiate sound
reg [3:0] sdur; 	// sound duration in whatevers
always @(posedge clk) begin
	keydown = !reset & (collide | wallbounce | outA | outB);
	if (keydown) begin
		sdur <= 0;
		if (collide) sfx <= 0;
		else if (wallbounce) sfx <= 1;
		else if (outA | outB) begin
			sdur <= 1;
			sfx <= 2; 
		end
	end
end
soundnik canhassound(clkdiv8, sound, keydown, sfx, sdur);
// - if there was more sound related stuff, it should've been above this line


parameter state0 = 0, state1 = 1, state2 = 2, state3 = 3, state4 = 4, state5 = 5, state6 = 6, state7 = 7;

// Game state, top-level.. Not started, started.. 
reg[3:0] state;

reg ball_reset;				// reset line for ballmover
reg service_side;			// 0 = ball served from left part
reg score_reset; 			// form reset pulse for score counter
reg [11:0] pause;			// delay timer/counter
wire[9:0] ball_x;			// ball x/y
wire[9:0] ball_y;

// Main state machine. Ph33r it.
always @(posedge ballAdvance or posedge reset or posedge gamereset) begin
	if (reset) state <= state5;
	else if (gamereset) state <= state0;	// this is done by start button
	else begin
		case (state) 
				// init game
		state0: begin
				score_reset <= 1;
				service_side <= 0;
				ledled <= 1;
				//score_addA <= 0;
				//score_addB <= 0;
				state <= state1;
			end
				// set ball and wait before the service
		state1: begin 
				score_reset <= 0;
				ball_reset <= 1;
				pause <= 1024;
				state <= state2;
			end
				// pacing before state3 as defined by state1...
		state2: begin 
				if (pause == 0) begin
					ball_reset <= 0;
					state <= state3;
				end else begin
					pause <= pause - 1;
				end
			end
				// game itself, just check for out situations
		state3: begin 
				if (outA | outB) begin
					service_side <= outB;
					// if score is up, endgame; otherwise new ball
					if (scoreA == 8'h21 || scoreB == 8'h21) begin
						pause <= 4095;
						ball_reset <= 1;
						state <= state4;
					end else begin
						state <= state1;
					end
				end
			end
				// endgame, no exit state except for gamereset
		state4: begin
				ledcounter <= ledcounter - 1;
				if (ledcounter == 0) ledled = !ledled;
			end
				// power-on setup
		state5: begin				
				ball_reset <= 1;
				state <= state4;
			end
		default:
			begin 
				state <= state0;
			end
		endcase
	end
end


// Paddle data:
// 	_top 	top y
//	_bot	bottom y
// Exact coordinates are calculated to avoid complex 
// comparisons in screen scanner (see paddlescan.v)
reg[9:0] paddleA_top;
reg[9:0] paddleA_bot;
reg[9:0] paddleB_top;
reg[9:0] paddleB_bot;
always @(posedge clk) begin
	paddleA_top <= paddleA_y - PADDLESIZE/2;
	paddleA_bot <= paddleA_y + PADDLESIZE/2;
	paddleB_top <= paddleB_y - PADDLESIZE/2;
	paddleB_bot <= paddleB_y + PADDLESIZE/2;
end

scores scorekeeper(clk, hsync, realx, realy, outB, outA, score_reset, scoreA, scoreB, scans[0]);


//
// Ball movement, collision, deflection
//
wire collideA;
wire collideB;
wire collide = collideA | collideB;		// paddle collision

wire[9:0] deflectA;						// deflect angles
wire[9:0] deflectB;

wire collisionreset;					// collision reset wire

wire [7:0] scoreA;						// scores
wire [7:0] scoreB;

collider #(15, 16) 		colliderA(ballAdvance, ball_x, ball_y, paddleA_top, paddleA_bot, collisionreset, collideA);
collider #(640-32, 16) 	colliderB(ballAdvance, ball_x, ball_y, paddleB_top, paddleB_bot, collisionreset, collideB);

deflector deflectorA(collideA, ball_y, paddleA_y, deflectA);
deflector deflectorB(collideB, ball_y, paddleB_y, deflectB);

wire outA;
wire outB;
wire wallbounce;
ballmover #(SCREENWIDTH,SCREENHEIGHT,BALLSIZE) 
	ballmover(clk, ball_reset, ballAdvance, ball_x, ball_y, collide, collisionreset, ({10{collideA}} & deflectA) | ({10{collideB}} & deflectB),
			  service_side, outA, outB, wallbounce);


wire paddleA_scan, paddleB_scan;
assign scans[1] = paddleA_scan | paddleB_scan;
paddlescan #(PADDLESIZE, 15) 		paddlescanA(clk, paddleA_top, paddleA_bot, realx, realy, paddleA_scan);
paddlescan #(PADDLESIZE, 640-32) 	paddlescanB(clk, paddleB_top, paddleB_bot, realx, realy, paddleB_scan);

ballscan #(BALLSIZE) ballscan(clk, realx, realy, ball_x, ball_y, scans[2]);

endmodule

// $Id: tehgame.v,v 1.13 2007/08/27 22:14:52 svo Exp $
