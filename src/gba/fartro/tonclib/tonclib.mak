#
#	tonclib.mak
#
# Making my gba utilities

# --- Project details ---

TONCLIB := libtonc.a
# set ASMOUT to 1 for c->asm output
ASMOUT  := 0

# --- commands ---
CROSS   := arm-elf-
AR      := $(CROSS)ar
AS      := $(CROSS)as
CC      := $(CROSS)gcc

MODEL   :=  -mthumb-interwork -mthumb
ASFLAGS := $(MODEL)
CFLAGS  := $(MODEL) -O3 -Wall

# --- Source files ---
# I want to see the asm code of the c-files too, but not of the
# luts, so filter those out first. And also filter out the generated
# s files (from .c -> .s) from the true assembly files
LUTS    := $(wildcard *lut.c)
CFILES	:= $(filter-out $(LUTS), $(wildcard *.c))
SFILES	:= $(filter-out $(CFILES:.c=.s), $(wildcard *.s))

# --- objects files ---
COBJS   := $(CFILES:.c=.o)
LUTOBJS := $(LUTS:.c=.o)
SOBJS   := $(SFILES:.s=.o)
OBJS    := $(COBJS) $(LUTOBJS) $(SOBJS)

# === main targets ===

# default: just compile the files
build: $(OBJS)

# buildlib: create a library
buildlib : $(TONCLIB)

$(TONCLIB) : $(OBJS)
	$(AR) rcs $@ $^

# === code generation ===

#standard c files (compiles to assembly too)
$(COBJS): %.o : %.c
ifeq ($(ASMOUT), 1)
	$(CC) $(CFLAGS) -S $<
endif
	$(CC) $(CFLAGS) -c $< -o $@

# luts
$(LUTOBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

# true assembly files
$(SOBJS) : %.o : %.s
	$(AS) $(ASFLAGS) $< -o $@


.PHONY : clean
clean: 
	rm -fv $(OBJS)
	rm -fv $(CFILES:.c=.s)
