CC=gcc
CFLAGS=-mthumb-interwork -ffreestanding -mthumb
MODFILES=pkunk.o

OBJFILES=pkunk.o
CFILES=pkunk.c pkunk.h

all: $(OBJFILES)

pkunk.o:	pkunk.c pkunk.h

clean:
	rm -f $(OBJFILES) $(CFILES)

.mod.c:
	Mod2Fras $^

