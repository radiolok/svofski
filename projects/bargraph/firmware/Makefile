#  Project Name
PROJECT = blink

DEFS=-DSAMPLERATE=28000

ARCH=-mthumb -mcpu=cortex-m3
#OPTIMIZATION = -O0 -g
OPTIMIZATION = -O2 -g -funroll-all-loops -fprefetch-loop-arrays

# Directory structure
SOURCEDIRS = src src/drivers
INCLUDEDIRS = include include/drivers
BUILDDIR = build
OUTDIR = bin

#  Compiler/Assembler/Linker/etc.
GCC = arm-none-eabi-gcc
GPP = arm-none-eabi-g++
LDSCRIPT = ldscript_rom_gnu.ld
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size
MKDIR = mkdir
RM = rm -f

#  Compiler and linker flags
CFLAGS = -Wall -fno-common $(ARCH) $(OPTIMIZATION)
CFLAGS += $(foreach DIR, $(INCLUDEDIRS), -I$(DIR))
CPPFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
LDFLAGS = $(ARCH) $(OPTIMIZATION) -nostartfiles
LDFLAGS += -Wl,--gc-sections,-Map=$(BUILDDIR)/$(PROJECT).map -T$(LDSCRIPT) -u _sbrk
ASFLAGS = $(ARCH) -DRAM_MODE=0
LDLIBS = -lstdc++

# Source and object files
CFILES = $(foreach DIR, $(SOURCEDIRS), $(wildcard $(DIR)/*.c))
CFILES += $(foreach DIR, $(SOURCEDIRS), $(wildcard $(DIR)/*.cpp))
CFILES += $(foreach DIR, $(SOURCEDIRS), $(wildcard $(DIR)/*.S))
OBJS = $(foreach CFILE, $(CFILES), $(BUILDDIR)/$(basename $(notdir $(CFILE))).o) 
HFILES = $(foreach DIR, $(INCLUDEDIRS), $(wildcard $(DIR)/*.h))

# Search path
VPATH = $(foreach DIR, $(SOURCEDIRS), $(strip $(DIR)):)

# Build rules
.PHONY: all clean 

all: include/instfilters.h $(BUILDDIR)/$(PROJECT).hex $(OUTDIR)/$(PROJECT).bin

burn: $(OUTDIR)/$(PROJECT).bin
	~/tmpsrc/lpc21isp/lpc21isp -bin $(OUTDIR)/$(PROJECT).bin /dev/tty.linvor-DevB 9600 12000

burnterm: $(OUTDIR)/$(PROJECT).bin
	~/tmpsrc/lpc21isp/lpc21isp -term -bin $(OUTDIR)/$(PROJECT).bin /dev/tty.linvor-DevB 9600 12000

clean:
	$(RM) $(OBJS)
	$(RM) $(BUILDDIR)/$(PROJECT).*
	$(RM) $(OUTDIR)/$(PROJECT).*
	CFLAGS=$(DEFS) CPPFLAGS=$(DEFS) make -C filtercalc clean

main.o: $(HFILES) include/instfilters.h

include/biquad2.h:  include/instfilters.h

include/instfilters.h:	filtercalc/instfilters.h

filtercalc/instfilters.h:	filtercalc
	make -C filtercalc instfilters.h DEFS=$(DEFS)
	cp filtercalc/instfilters.h include/instfilters.h
	
$(OUTDIR)/$(PROJECT).bin: $(BUILDDIR)/$(PROJECT).elf
	$(OBJCOPY) -O binary -j .text -j .data $(BUILDDIR)/$(PROJECT).elf $(OUTDIR)/$(PROJECT).bin
	$(SIZE) $(BUILDDIR)/$(PROJECT).elf

$(BUILDDIR)/$(PROJECT).hex: $(BUILDDIR)/$(PROJECT).elf
	$(OBJCOPY) -R .stack -O ihex $(BUILDDIR)/$(PROJECT).elf $(BUILDDIR)/$(PROJECT).hex

$(BUILDDIR)/$(PROJECT).elf: $(OBJS)
	$(GCC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(BUILDDIR)/%.o: %.c
	$(GCC) $(CFLAGS) -c -o $@ $<

$(BUILDDIR)/%.o: %.cpp
	$(GPP) $(CPPFLAGS) -c -o $@ $<

$(BUILDDIR)/%.o: %.S
	$(GCC) $(ASFLAGS) -c -o $@ $<

$(OBJS): | $(BUILDDIR)

$(OUTDIR):
	$(MKDIR) $(OUTDIR)

$(BUILDDIR): | $(OUTDIR)
	$(MKDIR) $(BUILDDIR)
