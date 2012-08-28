TARGET=incursion.rom
JS=c:\bin\spidermonkey\js.exe
OBJCOPY=/opt/local/bin/gobjcopy
PYTHON=c:\python27\python.exe

all:	incursion

ship.inc:	makesprites.py
	$(PYTHON) makesprites.py > ship.inc

incursion:	incursion.asm ship.inc
	echo inputFile="$<.asm";makeListing=true; | $(JS) -f - -f pasm.js >$@.lst.html
	echo inputFile="$<.asm";makeListing=false; | $(JS) -f - -f pasm.js >$@.hex
	$(OBJCOPY) -I ihex $@.hex -O binary $@.rom

clean:
	rm incursion.hex incursion.rom


