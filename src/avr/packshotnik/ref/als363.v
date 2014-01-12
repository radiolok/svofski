// This is in fact HCMS display test

`default_nettype none

module als363(clk50mhz, SW[9:0], KEY[3:0], HEX0, HEX1, HEX2, HEX3, GPIO_1);
input			clk50mhz;
input [3:0] 	KEY;
output [6:0] 	HEX0;
output [6:0] 	HEX1;
output [6:0] 	HEX2;
output [6:0] 	HEX3;
output [13:0] 	GPIO_1;
input  [9:0]	SW;

wire	[15:0]	mSEG7_DIG;

wire 	clk = clk50mhz;

wire iDATA_OUT, oOSC, oDATA_IN, oBLANK, oSEL, oRESET_N;
reg oRS;
reg oCE_N = 1;
reg oCLK = 0;

assign GPIO_1[0] = 1'bZ;//iDATA_OUT;
assign GPIO_1[1] = oOSC;
assign GPIO_1[3] = oDATA_IN;
assign GPIO_1[4] = oRS;
assign GPIO_1[5] = oCLK;
assign GPIO_1[8] = oCE_N;
assign GPIO_1[9] = oBLANK;
assign GPIO_1[11]= oSEL;
assign GPIO_1[13]= oRESET_N;


//SEG7_LUT_4	u0(HEX0,HEX1,HEX2,HEX3,mSEG7_DIG);
SEG7_LUT_4	u0(.oSEG0(HEX0),.iDIG(mSEG7_DIG));

assign {HEX1,HEX2,HEX3} = 21'o7777777;

reg [10:0] divctr;
wire	  slow_ce = divctr == 500;
always @(posedge clk) begin
	if (~KEY[0]) divctr <= 0;
	divctr <= divctr + 1;
	if (slow_ce) divctr <= 0;
end

reg [31:0] secctr;
wire 	   second_ce = slow_ce & secctr == 5000;
always @(posedge clk) begin
	if (~KEY[0]) secctr <= 0;
	if (slow_ce) secctr <= secctr + 1;
	if (second_ce) secctr <= 0;
end

reg [15:0] seconds;
always @(posedge clk) begin
	if (~KEY[0]) seconds <= 0;
	if (second_ce) seconds <= seconds + 1;
end

assign mSEG7_DIG = brightness;

reg rOSC;
always @(posedge clk) begin
	if (divctr[7:0] == 0) rOSC <= ~rOSC;
end

assign oOSC = rOSC;
assign oRESET_N = KEY[0];
assign oBLANK = 0;
assign oSEL = 0;

reg [5:0] seqstate = 0;
reg [5:0] nextstate = 0;

reg [7:0] cword;
reg [7:0] shiftcount;
reg [2:0] bitcount;
reg [2:0] lettercount;
wire[2:0] bitcount_p1 = bitcount + 1;

reg [3:0] brightness;
reg 	  brightnesssgn = 1;
wire[3:0] brightness_p1 = brightnesssgn ? brightness + 1 : brightness - 1;

assign 	oDATA_IN = cword[7];

always @(posedge clk) begin
	if (~KEY[0]) seqstate <= 0;
	else
	if (slow_ce) begin	
	case (seqstate) 
	0:	begin
		brightness <= 1;
		brightnesssgn <= 1;
		// load control word
		oCE_N <= 1;
		oRS <= 1;
		oCLK <= 0;
		seqstate <= 16;
		shiftcount <= 128;
		end
	16: begin
		shiftcount <= shiftcount - 1;
		if (shiftcount - 1 == 0) seqstate <= 1;
		end
		
	1:	begin	
		oCE_N <= 0;		// RS latches on CE transition
		seqstate <= 2;
		nextstate <= 3;
		// load control word 0
		cword <= {2'b01,SW[5:0]};//8'b01111111;//8'b01101111;
		shiftcount <= 8;
		end
		
	2:	begin
		oCLK <= ~oCLK;
		if (oCLK) begin 
			shiftcount <= shiftcount - 1;
			if (shiftcount - 1 != 0) begin
				cword <= {cword[6:0],1'b0};
				//oCLK <= ~oCLK;
			end else begin
				seqstate <= nextstate;
				oCE_N <= 1; // data is copied when oCE_N is high and oCLK is low
			end
		end //else oCLK <= ~oCLK;
		end
		
	3:  begin 
		// load cw 1
		oCE_N <= 0;
		oCLK <= 0;
		seqstate <= 2;
		nextstate <= 7;
		cword <= 8'b10000001;
		shiftcount <= 8;
		end
		
	7:  begin
		oCE_N <= 1;
		oRS <= 0;
		oCLK <= 0;
		seqstate <= 8;
		charofs <= 0;
		end
		
	8: 	begin
		oCE_N <= 0;
		seqstate <= 9;
		cword <= charbits;//8'h55;
		shiftcount <= 160;
		bitcount <= 0;
		lettercount <= 0;
		end
		
	9: 	begin
		oCLK <= ~oCLK;
		if (oCLK) begin
			bitcount <= bitcount + 1;
			
			cword <= {cword[6:0], 1'b0};
			
			if (bitcount_p1 == 7) begin 
				charofs <= ((charofs + 1) == 5) ? 0 : (charofs + 1);
				if ((charofs + 1) == 5) lettercount <= lettercount + 1;
			end
			
			if (bitcount_p1 == 0) cword <= {1'b0,charbits};//shiftcount[3] ? 8'h55 : 8'hAA;

			shiftcount <= shiftcount - 1;
			if (shiftcount - 1 == 0) begin
				seqstate <= 10;
				oCE_N <= 1;
			end
		end
		end
		
	10:	begin 
		oCE_N <= 1;
		if (SW[7] & second_ce) begin
			brightness <= brightness_p1;
			if (brightness_p1 == 1) brightnesssgn <= ~brightnesssgn;
			if (SW[6] && brightness_p1 == 15) brightnesssgn <= ~brightnesssgn;
			cword <= {4'b0111,brightness};
			shiftcount <= 8;
			oCE_N <= 1;		// RS latches on CE transition
			seqstate <= 11;
			nextstate <= 10;			
			oRS <= 1;
		end
		end
	11: begin
		oCE_N <= 0;
		seqstate <= 2;
		end
	endcase
	end
end

wire [6:0] charbitsT;
wire [6:0] charbitsI;
wire [6:0] charbitsS;

reg[ 6:0] charbits;
reg [3:0] charofs;
charTx letterT(.adr(charofs),.bits(charbitsT)); 
charIx letterI(.adr(charofs), .bits(charbitsI)); 
charSx letterS(.adr(charofs), .bits(charbitsS)); 

always @*
	case (lettercount) 
	0:	charbits <= charbitsT;
	1: 	charbits <= charbitsI;
	2:	charbits <= charbitsT;
	3:	charbits <= charbitsS;
	endcase

endmodule 

module charQ(input [3:0] adr, output reg [5:0] bits);
always 
	case (adr) 
	0:	bits <= 6'b 001110;
	1:	bits <= 6'b 010001;
	2:	bits <= 6'b 010001;
	3:	bits <= 6'b 010001;
	4:	bits <= 6'b 010101;
	5:	bits <= 6'b 010010;
	6:	bits <= 6'b 001101;
	endcase

endmodule

module charTx(input [3:0] adr, output reg [6:0] bits);
always 
	case (adr) 
	0:	bits <= 7'b 0000001;
	1:	bits <= 7'b 0000001;
	2:	bits <= 7'b 1111111;
	3:	bits <= 7'b 0000001;
	4:	bits <= 7'b 0000001;
	5:	bits <= 7'b 0000000;
	endcase
endmodule
module charIx(input [3:0] adr, output reg [6:0] bits);
always 
	case (adr) 
	0:	bits <= 7'b 0000000;
	1:	bits <= 7'b 1000001;
	2:	bits <= 7'b 1111111;
	3:	bits <= 7'b 1000001;
	4:	bits <= 7'b 0000000;
	5:	bits <= 7'b 0000000;
	endcase
endmodule
module charSx(input [3:0] adr, output reg [6:0] bits);
always 
	case (adr) 
	0:	bits <= 7'b 0100110;
	1:	bits <= 7'b 1001001;
	2:	bits <= 7'b 1001001;
	3:	bits <= 7'b 1001001;
	4:	bits <= 7'b 0110010;
	5:	bits <= 7'b 0000000;
	endcase
endmodule

module charQx(input [3:0] adr, output reg [6:0] bits);
always 
	case (adr) 
	0:	bits <= 7'b 0111110;
	1:	bits <= 7'b 1000001;
	2:	bits <= 7'b 1010001;
	3:	bits <= 7'b 0100001;
	4:	bits <= 7'b 1011110;
	5:	bits <= 7'b 0000000;
	endcase

endmodule

module charZHx(input [3:0] adr, output reg [6:0] bits);
always 
	case (adr) 
	0:	bits <= 7'b 1110111;
	1:	bits <= 7'b 0001000;
	2:	bits <= 7'b 1111111;
	3:	bits <= 7'b 0001000;
	4:	bits <= 7'b 1110111;
	5:	bits <= 7'b 0000000;
	endcase
endmodule

module charOx(input [3:0] adr, output reg [6:0] bits);
always 
	case (adr) 
	0:	bits <= 7'b 0111110;
	1:	bits <= 7'b 1000001;
	2:	bits <= 7'b 1000001;
	3:	bits <= 7'b 1000001;
	4:	bits <= 7'b 0111110;
	5:	bits <= 7'b 0000000;
	endcase
endmodule

module charPx(input [3:0] adr, output reg [6:0] bits);
always 
	case (adr) 
	0:	bits <= 7'b 1111111;
	1:	bits <= 7'b 0000001;
	2:	bits <= 7'b 0000001;
	3:	bits <= 7'b 0000001;
	4:	bits <= 7'b 1111111;
	5:	bits <= 7'b 0000000;
	endcase
endmodule

module charAx(input [3:0] adr, output reg [6:0] bits);
always 
	case (adr) 
	0:	bits <= 7'b 1111110;
	1:	bits <= 7'b 0001001;
	2:	bits <= 7'b 0001001;
	3:	bits <= 7'b 0001001;
	4:	bits <= 7'b 1111110;
	5:	bits <= 7'b 0000000;
	endcase
endmodule

