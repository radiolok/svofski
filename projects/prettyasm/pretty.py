#!/usr/bin/env python

import re
import json
import sys
import getopt
from os.path import splitext

from pi8 import Pi8

parser = Pi8()

def readInput(filename):
    result = []
    with open(filename) as lefile:
        for text in lefile:
        	print parser.Parse(text.rstrip())
            #text = text.rstrip()
            #parts = filter(lambda x: len(x) > 0, re.split(r'\s+', text.split(';')[0]))
            #result += [";;; include %s begin" % parts[1]] + readInput(parts[1]) + [";;; include %s end" % parts[1]] \
            #    if len(parts) > 1 and parts[0].lower() == '.include' \
            #    else [text]
    return result


def main(argv):
    global targetEncoding
    inputFileName = None
    lstFileName = None
    hexFileName = None
    targetEncoding = 'koi8-r'
    makeListing = True

    try:
        opts, args = getopt.getopt(argv, "h", ["ihex=", "lst="])
    except getopt.GetoptError as merde:
        print str(merde)
        printusage()
        sys.exit(2)

    for opt,arg in opts:
        if opt == "--ihex":
            hexFileName = arg
        elif opt == "--lst":
            lstFileName = arg

    if len(args) > 0:
        inputFileName = args[0]
        if lstFileName == None:
            root,ext = splitext(inputFileName)
            lstFileName = root + ".lst.html"
        if hexFileName == None:
            hexFileName = root + ".hex"
    else:
        print "no input file"
        printusage()
        sys.exit(2)

    if lstFileName != None:
        #try:
            with open(lstFileName, "w") as lst:
                #lst.write(''.join(preamble() +
                #                  assemble(inputFileName) +
                #                  jsons() +
                #                  tail()))
				#lst.write(''.join(assemble(inputFileName)))
				readInput(inputFileName)

if __name__ == "__main__":
   main(sys.argv[1:])                




