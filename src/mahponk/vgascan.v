//
// generate sync and provide screen coordinates + active flag
//
module vgascan(clk, hsync, vsync, realx, realy, videoActive, pre_xstart, pre_xend);

parameter SCREENWIDTH = 640;
parameter SCREENHEIGHT = 480;
input clk;
output hsync;
output vsync;
output reg[9:0] realx;  
output reg[9:0] realy;
output videoActive;
output pre_xstart, pre_xend;

reg[2:0] scanxx_state;
reg[2:0] scanyy_state;
reg[9:0] scanxx;
reg[9:0] scanyy;

reg videoActiveX;
reg videoActiveY;
wire videoActive = videoActiveX & videoActiveY;
//assign xscan = videoActiveX;
reg pre_xstart, pre_xend;

parameter state0 = 3'b000, state1 = 3'b001, state2 = 3'b010, state3 = 3'b011, state4 = 3'b100;

assign hsync = !(scanxx_state == state2);
assign vsync = !(scanyy_state == state2);

reg scanyy_minus;

always @(posedge clk) begin
		if (scanyy == 0) begin 
			case (scanyy_state)
			state0:
					begin
						scanyy <= 11;
						scanyy_state <= state1;
						videoActiveY <= 0;
					end
			state1: 
					begin
						scanyy <= 2;
						scanyy_state <= state2;
					end
			state2:
					begin
						scanyy <= 32;
						scanyy_state <= state3;
					end
			state3:
					begin
						scanyy <= SCREENHEIGHT;
						scanyy_state <= state0;
						videoActiveY <= 1;
						realy <= 0;
					end
			default:
					begin
						scanyy_state <= state0;
					end
			endcase
		end 
		else if (scanyy_minus) begin
			scanyy_minus <= 1'b0;
			scanyy <= scanyy - 1'b1;
		end

		if (scanxx == 0) begin	
			case (scanxx_state) 
			state0: 	
					begin
						scanxx <= 10'd16 /*+ 10'd74*/ - 1'b1;
						scanxx_state <= state1;
						realy <= realy + 1'b1;
						scanyy_minus <= 1'b1;
						videoActiveX <= 1'b0;
					end
			state1: 
					begin
						scanxx <= 10'd96 - 1'b1; 
						scanxx_state <= state2;
					end
			state2:	// before scan
					begin
						scanxx <= 10'd48 - 1'b1;
						scanxx_state <= state3;
					end
			state3:	
					begin
						pre_xstart <= 1'b1;
						videoActiveX <= 1'b1;
						realx <= 1'b0;
						scanxx <= SCREENWIDTH - 2;
						scanxx_state <= state4;
					end
			state4:
					begin
						pre_xend <= 1'b1;
						//scanxx <= 0;
						scanxx_state <= state0;
					end
			default: 
					begin
						scanxx_state <= state0;
					end
			endcase
		end 
		else scanxx <= scanxx - 1'b1;
		
		if (pre_xstart != 0) pre_xstart <= pre_xstart - 1'b1;
		if (pre_xend != 0) pre_xend <= pre_xend - 1'b1;

		if (videoActiveX) 
			realx <= realx + 1'b1;
		//realx <= realx + (videoActiveX & 1'b1);
		// negedge clk
end
	
endmodule
