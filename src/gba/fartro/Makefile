.SUFFIXES:

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif
include $(DEVKITARM)/gba_rules

BUILD:=.
SOURCES=.
INCLUDES=
DEPSDIR=.
LD=$(CC)

GBAFIX=gbafix
CFLAGS=-Wall -mthumb -mthumb-interwork -ffreestanding -funroll-loops -nostartfiles -nodefaultlibs -nostdlib -mtune=arm7tdmi -O3 -Itonclib 
CFLAGSARM=-Wall -marm -mthumb-interwork -ffreestanding -funroll-loops -mtune=arm7tdmi -O3 -Itonclib 
ASFLAGS=-mthumb-interwork -mthumb

LTHUMB_IW=

MAPFILE=svogba.map

LIBS=-ltonc -lgba -lFras
LIBPATHS=-L. -Ltonclib -LFras -L$(LIBGBA)/lib
LDFLAGS=-g $(ARCH) -Wl,-Map,$(notdir $@).map

SUBDIR_MODS=mods/pkunk.o

MAIN_OBJS=main.o \
	bitmaps/hare.o \
	bitmaps/map_tiles.o \
	bitmaps/omg.o \
	wtf_map.o \
	bitmaps/lifemap_1.o	\
	bitmaps/lifemap_2.o	\
	bitmaps/lifemap_3.o	\
	bitmaps/lifemap_4.o	\
	fonts/font_8x8.o	\
	lifemaps.o			\
	text.o				\
	lamerand.o 			\
	life.o				\
	messages.o			\
    $(SUBDIR_MODS)

OFILES=$(MAIN_OBJS)

SUBDIR_LIBTONC=tonclib/libtonc.a

default:	svo-fartro.gba

svo-fartro.gba:	main.elf.moo 
	@$(OBJCOPY) -O binary main.elf svo-fartro.gba
	$(GBAFIX) svo-fartro.gba
	zip	svo-fartro.zip svo-fartro.gba

main.elf.moo:	$(SUBDIR_LIBTONC) $(SUBDIR_MODS) main.elf

main.elf:	$(SUBDIR_MODS) $(MAIN_OBJS) 

main.o:		mods/pkunk.o tonclib/libtonc.a

$(SUBDIR_MODS):
	make -C mods

$(SUBDIR_LIBTONC):
	make -C tonclib

clean:
	make -C mods clean
	make -C tonclib clean
	rm -f $(MAIN_OBJS)
	rm -f svo-fartro.gba
	rm -f svo-fartro.zip
	rm -f main.elf
	rm -f *.o
	rm -f bitmaps/*.o
	rm -f tonclib/*.o
	
