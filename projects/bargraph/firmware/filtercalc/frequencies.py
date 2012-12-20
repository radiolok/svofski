#!/usr/bin/python

from math import sqrt

freq = 44

for n in xrange(20):
	print freq
	freq = freq * 6**(1.0/6)
