CFLAGS=-g
CPPFLAGS=-g -Wall
CC=g++
CPP=g++
LINK=g++

#all:	ncopy send
all:	ncopy

clean:	
	rm ncopy send *.o

ncopy: serial.o ncopy.o sendpacket.o commands.h diags.o

serial.o:	serial.h

sendpacket.o:	sendpacket.h
#send: send.c commands.h

