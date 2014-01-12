PROG			= main
CC				= msp430-gcc
CXX				= msp430-g++
OBJDUMP		= msp430-objdump
SIZE			= msp430-size
MSPDEBUG	= mspdebug
CFLAGS		= -Os -Wall -g -mmcu=msp430x2012

OBJS=$(PROG).o

all: $(PROG).elf $(PROG).lst
	$(SIZE) $(PROG).elf

.PHONY: all

$(PROG).elf: $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG).elf $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.lst: %.elf
	$(OBJDUMP) -DS $< >$@

clean:
	rm -fr $(PROG).elf $(PROG).lst $(OBJS)

install: $(PROG).elf
	$(MSPDEBUG) rf2500 "prog $(PROG).elf"
