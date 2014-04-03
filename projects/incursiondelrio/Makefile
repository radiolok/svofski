TARGET=incursion.rom
JS=jsshell-mac/js
OBJCOPY=/opt/local/bin/gobjcopy
PASMDIR=../prettyasm
PASM=pasm.js
CSS=listn.css
NAV=navigate.js

all:	incursion

$(PASM):	$(PASMDIR)/$(PASM)
	ln -s $< .

$(CSS):	$(PASMDIR)/$(CSS)
	ln -s $< .

$(NAV):	$(PASMDIR)/$(NAV)
	ln -s $< .

ship.inc:	makesprites.py
	python makesprites.py > ship.inc

incursion:	incursion.asm ship.inc $(PASM) $(CSS) $(NAV)
	echo inputFile=\"$<\"\;makeListing=true\; | $(JS) -f - -f $(PASM) >$@.lst.html
	echo inputFile=\"$<\"\;makeListing=false\; | $(JS) -f - -f $(PASM) >$@.hex
	$(OBJCOPY) -I ihex $@.hex -O binary $@.rom

clean:
	rm incursion.hex incursion.rom


