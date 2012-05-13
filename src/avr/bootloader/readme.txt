Mega bootloader "skeleton"
--------------------------

For questions/comments/(support ?) contact:

jussishow at avrfreaks   : Mastermind ... he he. AVR & linux code ...
svofski   at avrfreaks   : Windows (&BSD?) port + 
                           making the PC sourcecode "humanreadable" ...

Late 2009: by svofski: Mac OSX "port"

============
!!! NOTE !!!
============
This is a very simple&primitive bootloader!
It doesn't handle Eeprom, there's no read functionality, no CRC e.t.c.
It's simply an SPM experiment that "got out of control" ...

The AVR & the linux SW is written by me, and ported to Windows by Svofski.
Svofski also did a great job in cleaning up the code & making it look less 
like the "cowboy engineering mess" that I created ...
(the "original" SW was in one file, full of hardcoded solutions ...)

It's released under the "Nanook/Yellow snow license":

You can eat as much of the yellow snow as You like, 
but don't blame the dog if You get sick ...

**********************************************************************
**********************************************************************

Ok, how to build/use it ??

AVR
====

I've built the bootloader using:

binutils 2.16.1
gcc 4.0.2
avr-libc 1.4.3

Building the AVR code should be as simple as:

make MCU_TARGET=atmega8/atmega168
(or simply 'make' which uses the MCU_TARGET specified in the Makfile)

--> boot_atmega8.hex / boot_atmega168.hex

Upload the hexfile using Your favourite ISP programmer.
(see Programmer section in Makefile), set fusebits and enjoy !

The size of the Mega8 version is 124 words, 
The Mega168 version is slightly bigger at 145 words ...

To change the baudrate (default 115200) use the BAUDRATE constant:

ex:

F_OSC    = 11059200
B_1152   = (F_OSC/(16*115200))-1
B_19200  = (F_OSC/(16*19200))-1
BAUDRATE = B_1152

Linux SW
========

Again, a simple 'make TARGET=mega8/mega168' should do the trick ...
The executable is called mega8/mega168_bootload.

For verbose/debugging version : 'make TARGET=mega8/mega168 DEBUG=1'

! NOTE ! To build for both targets a 'make clean' is required between builds ...

The commandline parameters for the serial port/baudrate are optional. 
If omitted, the default values specified in 'mega_bootload.h' are used.


OpenBSD SW
==========
Everything said about Linux applies to OpenBSD, but you may want to do the 
following:
	- Alter default serial device in mega_bootload.h to something like
		/dev/cuaa0
	- Use gmake instead of make. BSD make utility is not compatible with
	  GNU Makefile used in this project.

This software is not tested under OpenBSD except that it compiles and runs.
You're welcome to try it yourself and report your success.

Windows SW
==============

To build under Windows you need some Windows compiler. The code is developed
with help of Bloodshed DevC++ IDE, but for compiling you will only need mingw
installation.

mingw can be obtained at http://www.mingw.org
Bloodshed Software website is at http://www.bloodshed.net

You will need to edit mk.bat so that PATH environment variable is pointing to
mingw bin directory. No additional changes should be required, but your mileage
may vary.

After your environment is set up, simply run mk.bat with target specified in the 
command line. For example:

C> mk.bat "TARGET=atmega168"

It is important to enclose target argument in double quotes. If everything is 
configured properly, you will be rewarded with mega168_bootload.exe. 

All comments related to default configuration of bootloader software from the 
Linux secion also apply to Windows port. Default serial device under Windows
is COM1.

Usage
======

To upload the file demo.bin using /dev/ttyS2 @ 57600 bps:

1: Invoke PC SW :

$ mega168_bootload demo.bin /dev/ttyS2 57600

$ Jussishow bootloader... version XXX
$ Build date: Feb  4 2006  14:07:03

$ Built for ATMega168 Pagesize 128 bytes
$ Using /dev/ttyS2 at 57600 baud.

$ Waiting for AVR...

2: Reset target AVR :

$ Ok here we go ...

[Data is being sent to the AVR ... ]

$ Transfer complete ...

$ Done ...

3: Over and out !








