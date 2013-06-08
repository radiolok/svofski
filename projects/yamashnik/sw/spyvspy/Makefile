CFLAGS=-g
CPPFLAGS=-g -Wall
CC=g++
CPP=g++
LINK=g++

#all:	ncopy send
all:	msxnet

clean:	
	rm msxnet *.o

msxnet: msxnet.o serial.o ncopy.o sendpacket.o commands.h diags.o basicsend.o

serial.o:	serial.h

sendpacket.o:	sendpacket.h
#send: send.c commands.h

