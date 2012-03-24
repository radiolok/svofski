#!/usr/bin/python

import fileinput
import string
import re
import sys
import math

SCALE=2540

def scale(x):
    return int(SCALE*x)

def scaleses(x):
    return x

def rotate(phi, x, y):
    phi = phi*math.pi/180.0
    x2 = x * math.cos(phi) - y * math.sin(phi)
    y2 = x * math.sin(phi) + y * math.cos(phi)
    return int(x2),int(y2)

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
dsnLayers = {}

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

def parsePolyline(tokens):
    p = [scale(eval(x)) for x in tokens[2:]]
    polyline = ['polyline', p[0]] + zip(p[1::2], p[2::2])

    list = []
    if tokens[1] in dsnLayers:
        list = dsnLayers[tokens[1]]
    else:
        dsnLayers[tokens[1]] = list
    list.append(polyline)

def parsePolygon(tokens):
    return None

def parseWiring(tokens):
    for wire in tokens:
        if wire[0] == 'wire':
            if wire[1][0] == 'polyline_path':
                parsePolyline(wire[1])
            elif wire[1] == 'polygon':
                parsePolygon(wire[1])
            
        elif wire[0] == 'via':
            #print 'via', wire[1]
            None
        else:
            print 'BOO', wire

def parseNetworkWire(wire):
    if wire[0] == 'path':
        layer = wire[1]
        aperture = scaleses(eval(wire[2]))
        p = [scaleses(eval(x)) for x in wire[3:]]
        points = zip(p[::2], p[1::2])

        path = ['path', aperture] + points

        list = []
        if layer in dsnLayers:
            list = dsnLayers[layer]
        else:
            dsnLayers[layer] = list
        list.append(path)

def parseNetworkVia(via):
    None

def parseNetwork(net):
    for piece in net[2:]:
        if piece[0] == 'wire':
            parseNetworkWire(piece[1])
        elif piece[0] == 'via':
            parseNetworkVia(piece[1])
        else:
            print 'BOO', piece

def parseNetworks(nets):
    for net in nets:
        parseNetwork(net)

def strPadShape(angle, x, y, name, shape, flags='square'):
    xy = [scale(eval(q)) for q in shape[2:6]]

    # rotate the shape before offsetting
    p1 = rotate(angle, xy[0], xy[1])
    p2 = rotate(angle, xy[2], xy[3])
    if p1[0] < p2[0]:
        xy = list(p1) + list(p2)
    else:
        xy = list(p2) + list(p1)

    thick, length = abs(xy[2]-xy[0]), abs(xy[3]-xy[1])
    if thick > length:
        temp = length
        length = thick
        thick = temp
        coords = [x-length/2+thick/2,y, x+length/2-thick/2,y, thick]
    else:
        # x = const
        coords = [x,y-length/2+thick/2,x,y+length/2-thick/2, thick]

    ofs = " ".join([repr(x) for x in coords])
    return 'Pad[' + ofs + ' 1000 3500 "" "%s" "%s"]'%(name, flags) 

def strPinShape(angle, x, y, name, shape):
    thick = scale(eval(shape[2]))
    result = 'Pin[%d %d %d %d %d %d "%s" "%s" "%s"]' %\
        (x, y, thick, 1000, 3500, 500, name, '', '')
    return result
    
def strOctagonShape(angle, x, y, name, shape):
    s = [scale(eval(q)) for q in shape[3:]]
    xmin = min(s[3::2])
    xmax = max(s[3::2])
    ymin = min(s[4::2])
    ymax = max(s[4::2])

    width, height = abs(xmax-xmin), abs(ymax-ymin)

    radius = width
    result = 'Pin[%d %d %d %d %d %d "%s" "%s" "%s"]' %\
        (x, y, radius, 1000, 3500, 500, name, '', '')
    return result


def strOblongShape(angle, x, y, name, shape):
    s = [eval(q) for q in shape[3:]]
    xmin = min(s[3::2])
    xmax = max(s[3::2])
    ymin = min(s[4::2])
    ymax = max(s[4::2])
    fakeshape = [0,0,str(ymin),str(xmin),str(ymax),str(xmax)]

    s1 = strPadShape(angle, x, y, name, fakeshape, '')

    fakeshape = [0,0,str(min([abs(xmax-xmin), abs(ymax-ymin)])),0,0]

    s2 = strPinShape(angle, x, y, name, fakeshape)

    return s1 + '\n' + s2

def strShape(angle, x, y, name, shape, padstack):
    if shape[0] == 'rect':
        return strPadShape(angle, x, y, name, shape)
    elif shape[0] == 'circle':
        return strPinShape(angle, x, y, name, shape)
    elif (shape[0] == 'polygon'):
        if padstack.startswith('Octagon'):
            return strOctagonShape(angle, x, y, name, shape)
        elif padstack.startswith('Oblong'):
            return strOblongShape(angle, x, y, name, shape)

    #return '--' + shape[0] + '--'     
    return None

def printPin(angle, x, y, name, pad, padstack):
    for piece in pad:
        if piece[0] == 'shape':
            pinstr = strShape(angle, x, y, name, piece[1:][0], padstack)
            if pinstr != None:
                return "\t" + pinstr
    return None

def printElement(co, e):
    elementName = e[1].strip('"')
    MX = scale(eval(e[2]))
    MY = scale(eval(e[3]))
    angle = eval(e[5])
    SIDE = e[4]
    # rotate the shit here
    # ...
    str = 'Element ["" "%s" "%s" "unknown" %d %d %d %d %d %d ""]\n(\n'%\
        (co, elementName, MX, MY, 1000,1000,1000,1000)
    # Pin.. ElementLine.. ElementArc..
    image = dsnImages[co]
    validPins = 0
    for sections in image:
        if sections[0] == 'pin':
            padstackName = sections[1]
            pad = dsnPadstacks[padstackName]
            name = manglePadName(sections[2])
            padX = scale(eval(sections[3]))
            padY = scale(eval(sections[4]))
            padX,padY = rotate(angle, padX, padY)
            strpad = printPin(angle, padX, padY, name, pad, padstackName)
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

def strPolyline(polyline):
    str = ''
    xy = polyline[2]
    for xys in polyline[3:]:
        #if (xys[0] == 0) or (xys[1] == 0): continue
        if xys[0] == 0: xys = (xy[0],xys[1])
        if xys[1] == 0: xys = (xys[0],xy[1])
        str = str + '\tLine[%d %d %d %d ' % (xy[0],xy[1],xys[0],xys[1])
        str = str + '%d 1000 "clearline"]\n' % polyline[1]
        xy = xys
    return str

def strPath(path):
    str = ''
    xy = path[2]
    for xys in path[3:]:
        str = str + '\tLine[%d %d %d %d ' % (xy[0],xy[1],xys[0],xys[1])
        str = str + '%d 1000 "clearline"]\n' % path[1]
        xy = xys
    return str

def printLayer(layerName):
    str = 'Layer(%s "%s")\n(' % tuple(layerName.split('#'))
    for stuffie in dsnLayers[layerName]:
        if stuffie[0] == 'polyline':
            #str = str + strPolyline(stuffie) + '\n'
            None
        elif stuffie[0] == 'path':
            str = str + strPath(stuffie) + '\n'
    str = str + ')'
    print str

def printLayers():
    for la in dsnLayers:
        printLayer(la)


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
    if section[0] == 'wiring':
        parseWiring(section[1:])


text = readinput('testk.ses')
tokens = re.split(r'([\(\)\s])',text)
ses = parse(None, tokens)[1][1]

for sespart in ses:
    if sespart[0] == 'routes':
        for routepart in sespart[1:]:
            if routepart[0] =='network_out':
                parseNetworks(routepart[1:])

#print ses
exit

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
printLayers()
