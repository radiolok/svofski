#!/usr/bin/python

from math import sqrt

freq = 55

print "float Freq[]={"
for n in xrange(20):
	print "    ", freq, ","
	freq = freq * 6**(1.0/6.3)
print "};"
