MAH PONK
~~~~~~~~
http://sensi.org/~svo/mahponk
                              

This is the Verilog HDL core part of Mah Ponk, the open and free implementation 
of a classic table tennis tele/video game for VGA monitors.

I tried to add some comments to the code, and even nicely format some of it 
but don't expect it to be readable and easy to understand. I guess general
system design has a lot of room for improvement, but this is how it is now
and it works in my particular FPGA, which is EPF6016TC144-3: the slowest grade 
of a generally slow Flex 6000 series.

I will gladly accept your comments, especially if they're of constructive kind.
I suspect that this project, at least parts of it, only run thanks to very
convenient glitches. OTOH, perfected on 34MHz it works rock solid at 25MHz, so
it may be not the case.

The project is kept in my private repository, at least for now so if you are
willing to contribute to the main branch you would have to contact me first.
Of course the license permits you to do just about anything with this code,
no asking required.

The project is done in Altera Quartus II Web Edition, some of the files are
Quartus-specific. This project inherits my pin assignments that are specific
for my particular PCB design. If you load it into your own development board
or a starter kit, make sure all pin assignments match those of your board.

scorecopymux.v is an Altera-specific macro for a multiplexer. If you don't have it,
or use Xilinx or something else, it can be replaced with pure Verilog mux2x4 
module, which is located in scores2.v

scores2.v may appear a little bit strange, but Flex 6000 doesn't support ROM
macros or register initialization with data, so I had to do it like this. 
It is universal and it works but I would hate to specify an entire alphabet
like that.

I'm interested to hear how this code works on different hardware. If you have
a development board with a different FPGA, maybe a Xilinx, please let me 
know if you had success with this project. 

To run this project on your own hardware, you need to make sure about two things:
pin assignments and timing. As for the pins, your board probably already
has a VGA connector and something suitable for sound. You will need to re-check
and possibly rewrite analinput.v if you have an ADC different from AD7921.
Most SPI ADC's have similar protocol though. As for the timing, it's all about
length of the porches. Suppose you have a clock faster than necessary, then
you need to calculate how many pixel clocks to add to the front or rear porch
to compensate. If your clock is some 33-35MHz you can go for 75Hz screen refresh,
for example. See comments in vgascan.v for more details.

Viacheslav Slavinsky
http://sensi.org/~svo