#!/usr/bin/env python

from pasm import evaluateNumber

print evaluateNumber('0x55') == 0x55
print evaluateNumber('0xf5') == 0xf5
print evaluateNumber('0xF5') == 0xf5
print evaluateNumber('$ff')  == 0xff
print evaluateNumber('0ffh')  == 0xff
print evaluateNumber('10b')  == 2
print evaluateNumber('10101010b')  == 0xaa
print evaluateNumber('377q')  == 0377
