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
// Design File: resetgen.v
// POR generator.
// 
// Pins description:
// 	clk		input 	master clock
//	pulse	output	reset pulse
//	button	input	active low button trigger
//
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

// $Id: resetgen.v,v 1.6 2007/08/27 22:14:50 svo Exp $
