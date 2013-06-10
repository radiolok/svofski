CFLAGS=-g
CPPFLAGS=-g -Wall
CC=g++
CPP=g++
LINK=g++

#all:	ncopy send
all:	msxnet spy_bootstrap.bin

clean:	
	rm msxnet *.o

msxnet: msxnet.o serial.o ncopy.o sendpacket.o commands.h diags.o basicsend.o spy.o

serial.o:	serial.h

sendpacket.o:	sendpacket.h

spy_bootstrap.bin:	spy_bootstrap.asm
	../z80asm-1.8/z80asm -I../z80asm-1.8/headers/ spy_bootstrap.asm -o spy_bootstrap.bin


