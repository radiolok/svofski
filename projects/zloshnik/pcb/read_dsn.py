#!/usr/bin/python

import fileinput
import string
import re
import sys
import math

SCALE=3937
default_clearance = 1.2*SCALE
pcb_width = 600000
pcb_height = 300000

def scale(x):
    return int(SCALE*x)

def scaleses(x):
    return int(x/10000.0*SCALE)

def rotate(phi, x, y):
    phi = phi*math.pi/180.0
    x2 = x * math.cos(phi) - y * math.sin(phi)
    y2 = x * math.sin(phi) + y * math.cos(phi)
    return int(x2),int(y2)

def manglePadName(s):
    #return '['+s+']'+s.replace('+', '_plus_').replace('-', '_minus_')
    return s.strip('\"')

def translateLayer(s):
    if s == '1#Top':
        return '1#component'
    if s == '16#Bottom':
        return '2#solder'
    return s

def readinput(fileName):
    fi = open(fileName, 'r')
    text = ''
    for line in fi:
        text = text + ' ' + line.strip(string.whitespace)

    return text

def parse(scope, tokens):
    i = 0
    data = []
    selfscope = None
    while i < len(tokens):
        if tokens[i] == ')':
            # end of scope
            return i + 1, data
        elif tokens[i] == '(':
            # open scope
            if selfscope == None:
                selfscope = ''
            size, sdata = parse(selfscope, tokens[i+1:])
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
dsnViaList = []
dsnNetList = {}

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

# only find out the board boundary
def parseStructure(tokens):
    global pcb_width, pcb_height

    for boundary in tokens:
        if boundary[1][0:2] == ['rect','pcb']:
            [x1,y1,x2,y2] = [scale(eval(q)) for q in boundary[1][2:6]]
            pcb_width, pcb_height = abs(x2-x1), abs(y2-y1)

def parseNetListNet(netName, pins):
    global dsnNetList

    pinList = []
    for pin in pins[1:]:
        m = re.match(r'("[_.,\-+\w]+"|[_.,\-+\w]+)-("[_.,\-+\w]+"|[_.,\-+\w]+)', pin)
        if m != None:
            componentName = m.group(1).strip('"')
            pinName = m.group(2).strip('"')
            pinList.append((componentName, pinName))

    dsnNetList[netName] = pinList
        

def parseNetList(tokens):
    # parse dsn netlist, ignore the vias, they will reappear in the session file
    for net in tokens:
        if net[0] == 'net':
            parseNetListNet(net[1].strip('"'), net[3])
    

def parseNetworkWire(wire, netName):
    if wire[0] == 'path':
        layer = wire[1]
        aperture = scaleses(eval(wire[2]))
        p = [scaleses(eval(x)) for x in wire[3:]]
        points = zip(p[::2], p[1::2])

        path = ['path', aperture, netName] + points

        list = []
        if layer in dsnLayers:
            list = dsnLayers[layer]
        else:
            dsnLayers[layer] = list
        list.append(path)

def parseNetworkVia(via, netName):
    padstack = via[1]
    x,y = scaleses(eval(via[2])),scaleses(eval(via[3]))
    dsnViaList.append((padstack,x,y))

def parseNetwork(net):
    netName = net[1]
    for piece in net[2:]:
        if piece[0] == 'wire':
            parseNetworkWire(piece[1], netName)
        elif piece[0] == 'via':
            parseNetworkVia(piece, netName)
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

    coords[1] = -coords[1]
    coords[3] = -coords[3]

    ofs = " ".join([repr(x) for x in coords])
    return 'Pad[' + ofs + ' %d 3500 "" "%s" "%s"]'%(default_clearance, name, flags) 

def strPinShape(angle, x, y, name, shape, flags=''):
    thick = scale(eval(shape[2]))
    drill = scale(0.4)
    result = 'Pin[%d %d %d %d %d %d "%s" "%s" "%s"]' %\
        (x, -y, thick, default_clearance, 3500, drill, name, name, flags)
    return result
    
def strOctagonShape(angle, x, y, name, shape, flags=''):
    s = [scale(eval(q)) for q in shape[3:]]
    xmin = min(s[3::2])
    xmax = max(s[3::2])
    ymin = min(s[4::2])
    ymax = max(s[4::2])

    width, height = abs(xmax-xmin), abs(ymax-ymin)

    radius = width
    result = 'Pin[%d %d %d %d %d %d "%s" "%s" "%s"]' %\
        (x, -y, radius, default_clearance, 3500, 500, name, name, '')
    return result


def strOblongShape(angle, x, y, name, shape, flags=''):
    s = [eval(q) for q in shape[3:]]
    xmin = min(s[3::2])
    xmax = max(s[3::2])
    ymin = min(s[4::2])
    ymax = max(s[4::2])
    fakeshape = [0,0,str(ymin),str(xmin),str(ymax),str(xmax)]

    s0 = strPadShape(angle, x, y, name, fakeshape, '')
    s1 = strPadShape(angle, x, y, name, fakeshape, 'onsolder')


    fakeshape = [0,0,str(min([abs(xmax-xmin), abs(ymax-ymin)])),0,0]

    s2 = strPinShape(angle, x, y, name, fakeshape)

    return s0+'\n\t' + s1 + '\n\t' + s2

def strShape(angle, x, y, name, shape, padstack, flags):
    if shape[0] == 'rect':
        return strPadShape(angle, x, y, name, shape, 'square,' + flags)
    elif shape[0] == 'circle':
        return strPinShape(angle, x, y, name, shape, flags)
    elif (shape[0] == 'polygon'):
        if padstack.startswith('Octagon'):
            return strOctagonShape(angle, x, y, name, shape, flags)
        elif padstack.startswith('Oblong'):
            return strOblongShape(angle, x, y, name, shape, flags)

    return None

def printPin(angle, x, y, name, pad, padstack, flags=''):
    for piece in pad:
        if piece[0] == 'shape':
            pinstr = strShape(angle, x, y, name, piece[1:][0], padstack, flags)
            if pinstr != None:
                return "\t" + pinstr
    return None

#
# Print components
#
def printElement(co, e):
    elementName = e[1].strip('"')
    MX = scale(eval(e[2]))
    MY = scale(eval(e[3]))
    angle = eval(e[5])
    SIDE = e[4]

    textdir = int(angle/90)

    str = 'Element ["" "%s" "%s" "unknown" %d %d %d %d %d %d ""]\n(\n'%\
        (co, elementName, MX, pcb_height - MY, -3000,-2500,textdir,100)

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

def printShapedVia(via):
    x = via[1]
    y = via[2]

    str = 'Element ["" "%s" "%s" "unknown" %d %d %d %d %d %d ""]\n(\n'%\
        ('', '', x, pcb_height - y, 1000,1000,1000,1000)

    padstackName = via[0]
    pad = dsnPadstacks[padstackName]
    name = "via"
    [strpad1,strpad2] = [printPin(0, 0, 0, name, pad, padstackName), \
                    printPin(0, 0, 0, name, pad, padstackName, flags='onsolder')]
    if strpad1 != None and strpad2 != None:
        str = "\n".join([str, strpad1, strpad2])

    str = str + ')'
    print str

def printVia(via):
    [x,y] = [via[1],via[2]]
    padstackName = via[0]
    pad = dsnPadstacks[padstackName]
    rect = pad[0][1]
    xyxy = [scale(eval(q)) for q in rect[2:6]]
    dia = abs(xyxy[2]-xyxy[0])
    drill = scale(0.6)
    str = 'Via [%d %d %d %d %d %d "" "thermal"]\n' % (x,pcb_height-y,dia,3500,1000,drill)
    print str

def printComponent(co):
    for place in dsnComponents[co]:
        printElement(co, place)
    # Shaped vias are nice but they cause too much trouble with polygons in PCB
    #for via in dsnViaList:
    #    printShapedVia(via)

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
        str = str + '%d %d "clearline"]\n' % (polyline[1], default_clearance)
        xy = xys
    return str

def strPath(path):
    str = ''
    xy = path[3]
    for xys in path[4:]:
        str = str + '\tLine[%d %d %d %d ' % (xy[0],pcb_height-xy[1],xys[0],pcb_height-xys[1])
        str = str + '%d %d "clearline"]\n' % (path[1], default_clearance)
        xy = xys
    return str

def printLayer(layerName):
    str = 'Layer(%s "%s")\n(' % tuple(translateLayer(layerName).split('#'))
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


def printNetList():
    print 'NetList()\n('
    for net in dsnNetList:
        print '\tNet("%s" "(unknown)")\n\t(' % net
        for node in dsnNetList[net]:
            print '\t\tConnect("%s-%s")' % node
        print '\t)'
    print ')'

#
# -- the script starts here
#

if len(sys.argv) < 2:
    print 'Usage: ' + sys.argv[0] + ' board.ses > board.pcb'
    quit()

text = readinput(sys.argv[1])
tokens = re.split(r'([\(\)\s])',text)
ses = parse(None, tokens)[1][1]

dsnFileName = None

for tok in ses:
    if tok[0] == 'base_design':
        dsnFileName = tok[1]

if dsnFileName == None:
    print 'Could not locate base DSN file name'
    quit()

text = readinput(dsnFileName)
tokens = re.split(r'([\(\)\s])',text)
pcb = parse(None, tokens)[1][1]
if pcb[0] != 'PCB':
    print 'Not a PCB?'
    exit
name = pcb[0]

for section in pcb[2:]:
    if section[0] == 'library':
        parseLibrary(section[1:])
    if section[0] == 'placement':
        parsePlacement(section[1:])
    if section[0] == 'wiring':
        parseWiring(section[1:])
    if section[0] == 'structure':
        parseStructure(section[1:])
    if section[0] == 'network':
        parseNetList(section[1:])



for sespart in ses:
    if sespart[0] == 'routes':
        for routepart in sespart[1:]:
            if routepart[0] =='network_out':
                parseNetworks(routepart[1:])


print 'PCB["" %d %d]' %(pcb_width,pcb_height)
print 'Grid[1.000000 0 0 1]'
print 'Cursor[0 0 0.000000]'
print 'PolyArea[200000000.000000]'
print 'Thermal[0.500000]'
print 'DRC[1000 1000 1000 500 1500 1000]'
print 'Flags(0x00000000000008d0)'
print 'Groups("1,c:2,s:3:4:5:6:7:8")'
print 'Styles["Signal,1000,3600,2000,1000:Power,2500,6000,3500,1000:Fat,4000,6000,3500,1000:Skinny,600,2402,1181,600"]'

try:
    fsyminc = open('symbols.inc','r')
    print fsyminc.read()
    fsyminc.close()
except:
    None

for via in dsnViaList:
    printVia(via)

printElements()
printLayers()

print 'Layer(3 "GND")'
print '('
print ')'
print 'Layer(4 "power")'
print '('
print ')'

printNetList()
