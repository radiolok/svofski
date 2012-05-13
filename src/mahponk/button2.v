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
// Design File: button2.v
// Button input driver with debounce.
// 
// Pins description:
// 	clk		input 	master clock
//	pulse	output	button pulse
//	button	input	active low button trigger
//
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

// $Id: button2.v,v 1.6 2007/08/27 22:14:48 svo Exp $
