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

reg [3:0] sfx; // which sound to play
reg keydown;   // initiate sound
reg [3:0] sdur; // sound duration in whatevers
reg [7:0] ledcounter;
reg 	  ledled;

assign led = ledled;

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

parameter state0 = 0, state1 = 1, state2 = 2, state3 = 3, state4 = 4, state5 = 5, state6 = 6, state7 = 7;

reg[3:0] state;




//reg score_addA, score_addB;
reg ball_reset;
reg service_side;	// 0 = ball served from left part
reg score_reset; // form reset pulse for score counter
reg [11:0] pause;
wire[9:0] ball_x;
wire[9:0] ball_y;

always @(posedge ballAdvance or posedge reset or posedge gamereset) begin
	if (reset) state <= state5;
	else if (gamereset) state <= state0;
	else begin
		case (state) 
		state0: begin
				score_reset <= 1;
				service_side <= 0;
				ledled <= 1;
				//score_addA <= 0;
				//score_addB <= 0;
				state <= state1;
			end
		state1: begin 
				// ball set and pause
				score_reset <= 0;
				//service_side <= 0;
				ball_reset <= 1;
				pause <= 1024;
				state <= state2;
			end
		state2: begin 
				if (pause == 0) begin
					ball_reset <= 0;
					state <= state3;
				end else begin
					pause <= pause - 1;
				end
			end
		state3: begin 
				if (outA | outB) begin
					service_side <= outB;
					if (scoreA == 8'h21 || scoreB == 8'h21) begin
						pause <= 4095;
						ball_reset <= 1;
						state <= state4;
					end else begin
						state <= state1;
					end
				end
			end
		state4: begin
				// endgame
				// resets by reset button
				//if (pause == 0) begin
				//	state <= state0;
				//end else begin
				//	pause <= pause - 1;
				//end
				ledcounter <= ledcounter - 1;
				if (ledcounter == 0) ledled = !ledled;
			end
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


reg[9:0] paddleA_top;
reg[9:0] paddleA_bot;
reg[9:0] paddleB_top;
reg[9:0] paddleB_bot;
reg[9:0] paddleA_ctop;
reg[9:0] paddleA_cbot;
reg[9:0] paddleB_ctop;
reg[9:0] paddleB_cbot;
always @(posedge clk) begin
	paddleA_top <= paddleA_y - PADDLESIZE/2;
	paddleA_bot <= paddleA_y + PADDLESIZE/2;
	paddleB_top <= paddleB_y - PADDLESIZE/2;
	paddleB_bot <= paddleB_y + PADDLESIZE/2;
end

wire collideA;
wire collideB;
wire collide = collideA | collideB;
wire[9:0] deflectA;
wire[9:0] deflectB;
wire collisionreset;
wire [7:0] scoreA;
wire [7:0] scoreB;

scores scorekeeper(clk, hsync, realx, realy, outB, outA, score_reset, scoreA, scoreB, scans[0]);

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