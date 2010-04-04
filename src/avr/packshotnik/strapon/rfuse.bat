avrdude -p m8 -c pony-stk200 -P lpt1 -U hfuse:r:high.txt:s -U lfuse:r:low.txt:s

