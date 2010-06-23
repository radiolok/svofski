#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif
include $(DEVKITARM)/gba_rules

DEPSDIR=.

CFLAGS=-Wall -mthumb -mthumb-interwork -ffreestanding -funroll-loops -mtune=arm7tdmi -O3 -Itonclib -Iboyscout -Isongs
ASFLAGS=-mthumb-interwork -mthumb

OFILES=affine.o \
	bg.o \
	color.o \
	core.o \
	geom.o\
	keypad.o \
	oam.o \
	sinlut.o \
	vid.o \
	interrupt.o \
	single_ints.o \
	divlut.o 

LIBTONCA=libtonc.a

libtonc.a:	$(OFILES)
	$(AR) rcs $@ $^

clean:
	rm -f $(OFILES)
	rm -f $(LIBTONCA)
