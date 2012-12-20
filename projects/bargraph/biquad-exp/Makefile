CFLAGS=--std=c99 
CPPFLAGS=
FILTERFILES=biquad.o testfilters.o


all: testfilters plots

plots:	testfilters
	./testfilters
	./mkplot.sh

testfilters:	$(FILTERFILES)
	g++   testfilters.o biquad.o   -o testfilters

clean:
	rm -f *.o fr*.txt bq*.txt bq*.png bands.png bq.html testfilters

testfilters.o:	biquad.h
