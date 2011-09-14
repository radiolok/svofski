module quad(clk, reset, quadA, quadB, count);
parameter SCREENHEIGHT=0;
parameter PADDLESIZE = 0;
input clk, reset, quadA, quadB;
output [9:0] count;

reg [2:0] quadA_delayed, quadB_delayed;
always @(posedge clk) quadA_delayed <= {quadA_delayed[1:0], quadA};
always @(posedge clk) quadB_delayed <= {quadB_delayed[1:0], quadB};

wire count_enable = quadA_delayed[1] ^ quadA_delayed[2] ^ quadB_delayed[1] ^ quadB_delayed[2];
wire count_direction = quadA_delayed[1] ^ quadB_delayed[2];

reg [5:0] accelcount;
reg [9:0] count;

wire [3:0] ac = (accelcount == 0) ? 1 : 5;
always @(posedge clk or posedge reset)
begin
  if (reset) begin
	count <= SCREENHEIGHT/2;
  end 
  else begin
	  if(count_enable)
	  begin
		count <= paddlelimiter(count_direction ? count + ac : count - ac);
		accelcount <= -1;
	  end
	  if (accelcount != 0) accelcount <= accelcount - 1;
  end
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