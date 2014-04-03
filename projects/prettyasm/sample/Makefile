TARGET=sample
JS=~/jsshell-mac/js
OBJCOPY=/opt/local/bin/gobjcopy
PASMDIR=..
PASM=pasm.js
CSS=listn.css
NAV=navigate.js

all:	$(TARGET)

$(PASM):	$(PASMDIR)/$(PASM)
	ln -s $< .

$(CSS):	$(PASMDIR)/$(CSS)
	ln -s $< .

$(NAV):	$(PASMDIR)/$(NAV)
	ln -s $< .

ASMFILES=$(PASM) $(CSS) $(NAV)

sample:	$(TARGET).asm messages.inc $(ASMFILES)
	echo inputFile=\"$<\"\;makeListing=true\; | $(JS) -U -f - -f $(PASM) >$@.lst.html
	echo inputFile=\"$<\"\;makeListing=false\; | $(JS) -U -f - -f $(PASM) >$@.hex
	$(OBJCOPY) -I ihex $@.hex -O binary $@.rom

clean:
	rm  $(TARGET).hex $(TARGET).rom


