TARGET=incursion.rom
JS=jsshell-mac/js
OBJCOPY=/opt/local/bin/gobjcopy

all:	incursion

ship.inc:	makesprites.py
	python makesprites.py > ship.inc

incursion:	incursion.asm ship.inc
	echo inputFile=\"$<\"\;makeListing=true\; | $(JS) -f - -f pasm.js >$@.lst.html
	echo inputFile=\"$<\"\;makeListing=false\; | $(JS) -f - -f pasm.js >$@.hex
	$(OBJCOPY) -I ihex $@.hex -O binary $@.rom

clean:
	rm incursion.hex incursion.rom


