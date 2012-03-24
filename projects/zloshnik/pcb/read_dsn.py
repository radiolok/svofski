#!/usr/bin/python

import fileinput
import string
import re
import sys

SCALE=2540

def scale(x):
    return int(SCALE*x)

def manglePadName(s):
    #return '['+s+']'+s.replace('+', '_plus_').replace('-', '_minus_')
    return s.strip('\"')

def readinput(fileName):
    fi = open(fileName, 'r')
    text = ''
    for line in fi:
        text = text + ' ' + line.strip(string.whitespace)

    return text

def parse(scope, tokens):
    #print 'SCOPE=', scope, tokens[0]
    i = 0
    data = []
    selfscope = None
    while i < len(tokens):
        #print '----', tokens[i:i+5]
        if tokens[i] == ')':
            # end of scope
            # print '['+selfscope+']', data
            return i + 1, data
        elif tokens[i] == '(':
            # open scope
            if selfscope == None:
                selfscope = ''
            size, sdata = parse(selfscope, tokens[i+1:])
            #print 'appended ', size, sdata
            i = i + size + 1
            data.append(sdata)
        elif selfscope == None:
            selfscope = tokens[i]
            data.append(selfscope)
            if scope != None:
                selfscope = scope + '.' + selfscope
            i = i + 1
        else:
            trimmed = tokens[i].strip(string.whitespace)
            if len(trimmed) > 0:
                data.append(trimmed)
            i = i + 1
    return i + 1, data

dsnImages = {}
dsnPadstacks = {}
dsnComponents = {}

def parseImage(tokens):
    dsnImages[tokens[0].strip('"')] = tokens[1:]

def parsePadstack(tokens):
    dsnPadstacks[tokens[0]] = tokens[1:]

def parseComponent(tokens):
    dsnComponents[tokens[0].strip('"')] = tokens[1:]

def parseLibrary(tokens):
    for item in tokens:
        if item[0] == 'image':
            parseImage(item[1:])
        elif item[0] == 'padstack':
            parsePadstack(item[1:])

def parsePlacement(tokens):
    for item in tokens:
        if item[0] == 'component':
            parseComponent(item[1:])

def strPadShape(x, y, shape):
    if shape[0] == 'rect':
        xy = [scale(eval(q)) for q in shape[2:6]]
        thick, length = xy[2]-xy[0], xy[3]-xy[1]
        if thick > length:
            temp = length
            length = thick
            thick = temp
            coords = [x-length/2+thick/2,y, x+length/2-thick/2,y, thick]
        else:
            # x = const
            coords = [x,y-length/2+thick/2,x,y+length/2-thick/2, thick]

        #coords = [int(SCALE*(o[0]+eval(o[1]))) for o in zip([x,y,x,y],shape[2:6])]
        ofs = " ".join([repr(x) for x in coords])
        return ofs
    #return '--' + shape[0] + '--'     
    return None

def printPad(x, y, name, pad):
    str = "Pad["
    for piece in pad:
        if piece[0] == 'shape':
            shape = strPadShape(x, y, piece[1:][0])
            if shape != None:
                str = str + shape
            else:
                return None
            break
    str = str + " 1000 3500";
    str = str + ' "" "%s" "square"]'%(name)
    return  "\t" + str

def printElement(co, e):
    MX = scale(eval(e[2]))
    MY = scale(eval(e[3]))
    ROT = e[5]
    SIDE = e[4]
    # rotate the shit here
    # ...
    str = 'Element ["" "%s" "%s" "unknown" %d %d %d %d %d %d ""]\n(\n'%\
        (co, e[1], MX, MY, 1000,1000,1000,1000)
    # Pin.. ElementLine.. ElementArc..
    image = dsnImages[co]
    validPins = 0
    for sections in image:
        if sections[0] == 'pin':
            pad = dsnPadstacks[sections[1]]
            name = manglePadName(sections[2])
            padX = scale(eval(sections[3]))
            padY = scale(eval(sections[4]))
            strpad = printPad(padX, padY, name, pad)
            if strpad != None:
                str = str + strpad + '\n'
                validPins = validPins + 1
        
    str = str + ')'
    if validPins > 0:
        print str

def printComponent(co):
    for place in dsnComponents[co]:
        printElement(co, place)

def printElements():
    for co in dsnComponents:
        printComponent(co)

text = readinput('testk.dsn')
tokens = re.split(r'([\(\)\s])',text)
pcb = parse(None, tokens)[1][1]
if pcb[0] != 'PCB':
    print 'Not a PCB?'
    exit
name = pcb[0]

#print "PCB name=", eval(pcb[1])
for section in pcb[2:]:
    if section[0] == 'library':
        parseLibrary(section[1:])
    if section[0] == 'placement':
        parsePlacement(section[1:])

#
#for q in dsnImages:
#    print q, dsnImages[q]
#for q in dsnPadstacks:
#    print q, dsnPadstacks[q]
#for q in dsnComponents:
#    print q, dsnComponents[q]

print 'PCB["" 1250000 1250000]'
print 'Grid[1.000000 0 0 1]'
print 'Cursor[0 0 0.000000]'
print 'PolyArea[200000000.000000]'
print 'Thermal[0.500000]'
print 'DRC[1000 1000 1000 500 1500 1000]'
print 'Flags("rubberband,nameonpcb,uniquename,clearnew,snappin,onlynames")'
print 'Groups("1,c:2,s:3:4:5:6:7:8")'
print 'Styles["Signal,1000,3600,2000,1000:Power,2500,6000,3500,1000:Fat,4000,6000,3500,1000:Skinny,600,2402,1181,600"]'

printElements()
