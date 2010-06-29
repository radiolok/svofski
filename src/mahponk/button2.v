module button2(clk, pulse, button);
	parameter TIME = 32767;
	input clk;
	output pulse;
	input button;

	reg [15:0] delay;
	assign pulse = (delay != 0) | (!button);
	always @(posedge clk) begin
		if (delay != 0) 
			delay <= delay - 1;
		else
			if (!button) delay <= TIME;
	end
endmodule
