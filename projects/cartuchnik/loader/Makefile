PROJECT = test
OUTDIR = build
SOURCEDIRS = src

CROSS=/opt/local/gccvectrex/bin
CC=$(CROSS)/m6809-unknown-none-gcc
LD=$(CC)
AS=$(CC)

CRT0 = crt0

SRCFILES = $(foreach DIR, $(SOURCEDIRS), $(wildcard $(DIR)/*.c))
SRCFILES += $(foreach DIR, $(SOURCEDIRS), $(wildcard $(DIR)/*.S))
OBJS = $(foreach SRCFILE, $(SRCFILES), $(OUTDIR)/$(basename $(notdir $(SRCFILE))).o) 

OFILES = $(CRT0).o $(OBJS)

VPATH = $(foreach DIR, $(SOURCEDIRS), $(strip $(DIR)):)

all:	$(OUTDIR) $(OUTDIR)/$(PROJECT).bin

clean:	
	rm -f $(OUTDIR)/*

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OUTDIR)/$(PROJECT).bin:	$(OUTDIR)/$(PROJECT).s19
	srec_cat $? -Output $@ -Binary

$(OUTDIR)/$(PROJECT).s19:	$(OUTDIR)/$(OFILES)
	echo "SRCFILES are: $(SRCFILES)"
	echo "OFILES are: $(OFILES)"
	$(LD) $(LDFLAGS) $? -o $@ -Wl,--map

$(OUTDIR)/%.o:	%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUTDIR)/%.o:	%.S
	$(AS) $(CFLAGS) -c -o $@ $<

