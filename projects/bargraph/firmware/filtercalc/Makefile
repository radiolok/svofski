ARCH=-m32
CFLAGS=--std=c99 $(ARCH)
CPPFLAGS=-fpermissive $(ARCH) -ftrapv -DTESTBENCH $(DEFS)
FILTERFILES=biquad.o testfilters.o


all: testfilters plots calcfilters

plots:	testfilters
	./testfilters
	./mkplot.sh

instfilters.h:	calcfilters
	./calcfilters

testfilters:	$(FILTERFILES)
	g++ $(ARCH) -ftrapv testfilters.o biquad.o   -o testfilters

clean:
	rm -f *.o fr*.txt bq*.txt bq*.png bands.png bq.html testfilters calcfilters

testfilters.o:	biquad.h

calcfilters: biquad.h
