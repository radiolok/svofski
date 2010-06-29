module resetgen(clk, pulse, button);
	input clk;
	output pulse;
	input button;

	reg [3:0] reset;
	assign pulse = (reset != 0) && (reset != 4'b1111);
	always @(posedge clk) begin
		if (reset != 4'b1111) 
			reset <= reset + 1;
		else
			if (!button) reset <= 0;
	end
endmodule
