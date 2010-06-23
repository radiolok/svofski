ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

DEPSDIR=.
CFLAGS=-mthumb-interwork -ffreestanding -mthumb
MODFILES=pkunk.o

OBJFILES=pkunk.o
CFILES=pkunk.c pkunk.h

all:	Mod2Fras $(OBJFILES)

Mod2Fras:
	echo Please build Mod2Fras manually

include $(DEVKITARM)/gba_rules
pkunk.o:	pkunk.c pkunk.h

clean:
	rm -f $(OBJFILES) $(CFILES)

.mod.c:	Mod2Fras
	./Mod2Fras $^

