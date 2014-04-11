#!/usr/bin/env python

import re
import json
import sys
import getopt
from os.path import splitext

def enumerate2(sequence, start=0, granularity=1):
    n = 0
    v = start
    for elem in sequence:
        yield v, elem
        n += 1
        if n == granularity:
            v += granularity
            n = 0

def slicing_enumerate2(sequence, start=0, granularity=1, validator = lambda x: True):
    i = 0
    for base in xrange(start, len(sequence), granularity):
        if len([x for x in sequence[base : base + granularity] if validator(x)]) > 0:
            yield i, base, sequence[base : base + granularity]
            i += 1

def char8(byteval):
    return [lambda x:'.', chr][byteval > 32 and byteval < 127](byteval)

def hex8(byteval):
    return [lambda x:'%02X' % x, lambda x:'??'][byteval == None or byteval < 0 or byteval > 255](byteval)

def hex16(wordval):
    return [lambda x:'%04X' % x, lambda x:'????'][wordval == None or wordval < 0 or wordval > 65535](wordval)

def isValidIm16(s):
    return s != None and len(s) > 0

def isValidIm8(s):
    return s != None and len(s) > 0

def toTargetEncoding(str, encoding):
    return str.decode('utf-8').encode(encoding)

ops0 = {"nop": "00",
        "hlt":  "76",
        "ei":   "fb",
        "di":   "f3",
        "sphl": "f9",
        "xchg": "eb",
        "xthl": "e3",
        "daa":  "27",
        "cma":  "2f",
        "stc":  "37",
        "cmc":  "3f",
        "rlc":  "07",
        "rrc":  "0f",
        "ral":  "17",
        "rar":  "1f",
        "pchl": "e9",
        "ret":  "c9",
        "rnz":  "c0",
        "rz":   "c8",
        "rnc":  "d0",
        "rc":   "d8",
        "rpo":  "e0",
        "rpe":  "e8",
        "rp":   "f0",
        "rm":   "f8"}

opsIm16 = {
    "lda":  "3a",
    "sta":  "32",
    "lhld": "2a",
    "shld": "22",
    "jmp":  "c3",
    "jnz":  "c2",
    "jz":   "ca",
    "jnc":  "d2",
    "jc":   "da",
    "jpo":  "e2",
    "jpe":  "ea",
    "jp":   "f2",
    "jm":   "fa",
    "call": "cd",
    "cnz":  "c4",
    "cz":   "cc",
    "cnc":  "d4",
    "cc":   "dc",
    "cpo":  "e4",
    "cpe":  "ec",
    "cp":   "f4",
    "cm":   "fc"}

# lxi rp, im16
opsRpIm16 = {
    "lxi":  "01"    # 00rp0001, bc=00, de=01,hl=10, sp=11
    }


# adi 33, out 10
opsIm8 = {
    "adi":  "c6",
    "aci":  "ce",
    "sui":  "d6",
    "sbi":  "de",
    "ani":  "e6",
    "xri":  "ee",
    "ori":  "f6",
    "cpi":  "fe",
    "in":   "0db",
    "out":  "d3"}


opsRegIm8 = {
    "mvi":  "06"}

opsRegReg = {
    "mov":  "40"}

opsReg = {
    "add": "80", # regsrc
    "adc": "88",
    "sub": "90",
    "sbb": "98",
    "ana": "a0",
    "xra": "a8",
    "ora": "b0",
    "cmp": "b8",

    "inr": "04", # regdst (<<3)
    "dcr": "05"}

# these are the direct register ops, regdst
opsRegDst = {"inr", "dcr"}

opsRp = {
    "ldax": "0A", # rp << 4 (only B, D)
    "stax": "02", # rp << 4 (only B, D)
    "dad":  "09", # rp << 4
    "inx":  "03", # rp << 4
    "dcx":  "0b", # rp << 4
    "push": "c5", # rp << 4
    "pop":  "c1"  # rp << 4
    }

mem = [None] * 65536
labelsCount = 0
labels = {}
resolveTable = {} # label negative id, resolved address
textlabels = []
references = []
errors = {}
regUsage = {}
lineCount = 0
doHexDump = True
hexFileName = None
binFileName = None

def setmem16(addr, immediate):
    if immediate >= 0:
        mem[addr] = immediate & 0xff
        mem[addr+1] = immediate >> 8
    else:
        mem[addr] = immediate
        mem[addr+1] = immediate

def setmem8(addr, immediate):
    mem[addr] = [immediate, immediate & 0xff][immediate >= 0]

def parseRegisterPair(s):
    if (s != None):
        s = s.strip().split(';')[0].lower()
        if s == 'b' or s == 'bc': return 0
        if s == 'd' or s == 'de': return 1
        if s == 'h' or s == 'hl': return 2
        if s == 'sp' or s == 'psw' or s == 'a': return 3
    return -1

# b=000, c=001, d=010, e=011, h=100, l=101, m=110, a=111
def parseRegister(s):
    if s == None:
        return -1
    s = s.strip()
    if len(s) > 1: 
        return -1
    return "bcdehlma".find(s[0])

def resolveNumber(identifier):
    if identifier == None or len(identifier) == 0: 
        return -1
    if (identifier[0] == "'" or identifier[0] == '"') and (len(identifier) == 3):
        return 0xff & ord(identifier[1])
    if identifier.startswith('0x'):
        identifier = identifier[2:] + 'h'
    if identifier[0] == '$':
        identifier = identifier[1:] + 'h'
    if '0123456789'.find(identifier[0]) != -1:
        try:
            return int(identifier)
        except Exception, e:
            pass
        try:
            return int(identifier[:-1], {'d':10,'h':16,'b':2,'q':8}[identifier[-1].lower()])
        except Exception, e:
            pass
    return -1

def referencesLabel(identifier, linenumber):
    global references
    if references[linenumber] == None:
        references[linenumber] = identifier.lower()

def markLabel(identifier, address, linenumber = None, override = False):
    global labelsCount, labels, resolveTable, textlabels

    identifier = re.sub(r'\$([0-9a-fA-F]+)', r'0x\1', identifier)
    identifier = re.sub(r"(^|[^'])(\$|\.)", ' ' + str(address) + ' ', identifier)
    number = resolveNumber(identifier.strip())
    if number != -1: 
        return number
    if linenumber == None:
        labelsCount = labelsCount + 1
        address = -1 - labelsCount
    identifier = identifier.lower()

    found = labels.get(identifier)
    if found != None:
        if address >= 0:
            resolveTable[-found] = address
        else:
            address = found

    if (found == None) or override:
        labels[identifier] = address

    if linenumber != None:
        textlabels[linenumber] = identifier

    return address

def tokenDBDW(s, addr, longueur, linenumber):
    if len(s) == 0: 
        return 0

    n = markLabel(s, addr)
    referencesLabel(s, linenumber)

    if longueur == None: 
        longueur = 1
    size = -1
    if longueur == 1 and n < 256:
        setmem8(addr, n)
        size = 1
    elif longueur == 2 and n < 65536:
        setmem16(addr, n)
        size = 2
    return size

def tokenString(s, addr, linenumber):
    return len([setmem8(addr+i, ord(s[i])) for i in xrange(len(s))])

def parseDeclDB(args, addr, linenumber, dw):
    text = ' '.join(args[1:])
    arg = ""
    mode, nbytes = 0, 0
    cork = False

    for i in xrange(len(text)):
        if mode == 0:
            if text[i] == '"' or text[i] == "'":
                mode = 1
                cork = text[i]
            elif text[i] == ',':
                longueur = tokenDBDW(arg, addr + nbytes, dw, linenumber)
                if longueur < 0:
                    return -1
                nbytes = nbytes + longueur
                arg = ''
            elif text[i] == ';':
                i = longueur(text)
            else:
                arg = arg + text[i]
        elif mode == 1:
            if text[i] != cork:
                arg = arg + text[i]
            else:
                cork = False
                mode = 0
                longueur = tokenString(arg, addr + nbytes, linenumber)
                if longueur < 0:
                    return -1
                nbytes = nbytes + longueur
                arg = ''

    if mode == 1: return -1    # unterminated string
    longueur = tokenDBDW(arg, addr+nbytes, dw, linenumber)
    if longueur < 0: return -1
    return nbytes + longueur

def getExpr(arr):
    ex = ' '.join(arr).strip()
    if ex[0] == '"' or ex[0] == "'":
        return ex
    return ex.split(';')[0]

def useExpr(s, addr, linenumber):
    expr = getExpr(s)
    if expr == None or len(expr.strip()) == 0: 
        return False
    immediate = markLabel(expr, addr)
    referencesLabel(expr, linenumber)
    return immediate

def inSet(set, val):
    return set.intersection({val}) != ({0}-{0})

def parseInstruction(text, addr, linenumber):
    parts = ((lambda s: 
                s[:next((i for i,q in enumerate(s) if q.startswith(';')), len(s))])
                ([x for x in re.split(r'\s+', text) if len(x) > 0]))
    
    labelTag, immediate = None, None
    while len(parts) > 0:
        opcs = ''
        mnemonic = parts[0].lower()

        # no operands
        opcs = ops0.get(mnemonic)
        if opcs != None:
            mem[addr] = int(opcs, 16)
            if mnemonic == "xchg":
                regUsage[linenumber] = ['#', 'h', 'l', 'd', 'e']
            elif mnemonic == "sphl" or mnemonic == "xthl":
                regUsage[linenumber] = ['#', 'sp', 'h']
            elif inSet({"ral", "rar", "rla", "rra", "cma"}, mnemonic):
                regUsage[linenumber] = ['#', 'a']
            return 1
        
        # immediate word
        opcs = opsIm16.get(mnemonic)
        if opcs != None:
            mem[addr] = int(opcs, 16)
            immediate = useExpr(parts[1:], addr, linenumber)
            setmem16(addr+1, immediate)
            if inSet({"lhld", "shld"}, mnemonic):
                regUsage[linenumber] = ['#', 'h', 'l']
            elif inSet({"lda", "sta"}, mnemonic):
                regUsage[linenumber] = ['#', 'a']
            return 3
        
        # register pair <- immediate
        opcs = opsRpIm16.get(mnemonic)
        if opcs != None:
            subparts = ' '.join(parts[1:]).split(',')
            if len(subparts) < 2: 
                return -3
            rp = parseRegisterPair(subparts[0])
            if rp == -1:
                return -3
            mem[addr] = int(opcs, 16) | (rp << 4)
            immediate = useExpr(subparts[1:], addr, linenumber)
            setmem16(addr+1, immediate)
            regUsage[linenumber] = ['@'+subparts[0].strip()]
            if inSet({"h","d"}, subparts[0].strip()):
                rpmap = {"h":"l","d":"e"}
                regUsage[linenumber] += ['#', rpmap[subparts[0].strip()]]
            return 3

        # immediate byte       
        opcs = opsIm8.get(mnemonic)
        if opcs != None:
            mem[addr] = int(opcs, 16)
            immediate = useExpr(parts[1:], addr, linenumber)
            setmem8(addr+1, immediate)
            if inSet({"sui", "sbi", "xri", "ori", "ani", "adi", "aci", "cpi"}, mnemonic):
                regUsage[linenumber] = ['#', 'a']
            return 2

        # single register, im8
        opcs = opsRegIm8.get(mnemonic)
        if opcs != None:
            subparts = ' '.join(parts[1:]).split(',')
            if len(subparts) < 2: 
                return -2
            reg = parseRegister(subparts[0])
            if reg == -1:
                return -2
            mem[addr] = int(opcs, 16) | (reg << 3)
            immediate = useExpr(subparts[1:], addr, linenumber)
            setmem8(addr+1, immediate)
            regUsage[linenumber] = [subparts[0].strip()]
            return 2
                
        # dual register (mov)
        opcs = opsRegReg.get(mnemonic)
        if opcs != None:
            subparts = ' '.join(parts[1:]).split(',')
            if len(subparts) < 2:
                return -1
            reg1 = parseRegister(subparts[0])
            reg2 = parseRegister(subparts[1])
            if reg1 == -1 or reg2 == -1: 
                return -1;
            mem[addr] = int(opcs, 16) | (reg1 << 3) | reg2
            regUsage[linenumber] = [subparts[0].strip(), subparts[1].strip()]
            return 1

        # single register
        opcs = opsReg.get(mnemonic)
        if opcs != None:
            reg = parseRegister(parts[1])
            if reg == -1:
                return -1
            if inSet(opsRegDst, mnemonic):
                reg = reg << 3
            mem[addr] = int(opcs, 16) | reg
            regUsage[linenumber] = [parts[1].strip()]
            if inSet({"ora", "ana", "xra", "add", "adc", "sub", "sbc", "cmp"}, mnemonic):
                regUsage[linenumber] += ['#', 'a']
            return 1
        
        # single register pair
        opcs = opsReg.get(mnemonic)
        if opcs != None:
            rp = parseRegisterPair(parts[1])
            if rp == -1:
                return -1
            mem[addr] = int(opcs, 16) | (rp << 4)
            regUsage[linenumber] = ['@'+parts[1].strip()]
            if mnemonic == "dad":
                regUsage[linenumber] += ['#', 'h', 'l']
            elif inSet({"inx", "dcx"}, mnemonic):
                if inSet({"h","d"}, parts[1].strip()):
                    rpmap = {"h":"l","d":"e"}
                    regUsage[linenumber] += ['#', rpmap[parts[1].strip()]]
            return 1
        
        # rst
        if mnemonic == "rst":
            n = resolveNumber(parts[1])
            if n >= 0 and n < 8:
                mem[addr] = 0xC7 | (n << 3)
                return 1
            return -1;
        
        if mnemonic == ".org" or mnemonic == "org":
            n = evaluateExpression(' '.join(parts[1:]), addr)
            if n >= 0:
                return -100000-n
            return -1

        if mnemonic == ".binfile":
            if len(parts) > 1 and len(parts[1].strip()) > 0:
                binFileName = parts[1]
            return -100000

        if mnemonic == ".hexfile":
            if len(parts) > 1 and len(parts[1].strip()) > 0:
                hexFileName = parts[1]
            return -100000

        if mnemonic == ".download":
            if len(parts) > 1 and len(parts[1].strip()) > 0:
                downloadFormat = parts[1].strip()
            return -100000

        if mnemonic == ".objcopy":
            return -100000

        if mnemonic == ".postbuild":
            return -100000

        if mnemonic == ".nodump":
            doHexDump = False
            return -100000

        if mnemonic == ".nolist" or mnemonic == ".list":
            return 0

        # assign immediate value to label
        if mnemonic == ".equ" or mnemonic == "equ":
            if labelTag == None: 
                return -1
            value = evaluateExpression(' '.join(parts[1:]), addr)
            markLabel(labelTag, value, linenumber, true)
            return 0

        if mnemonic == ".encoding":
            encoding = ' '.join(parts[1:])
            try:
                encoded = toTargetEncoding('test', encoding)
                targetEncoding = encoding
            except Exception, e:
                return -1
            return -100000

        if inSet({'cpu', 'aseg', '.aseg'}, mnemonic):
            return 0;

        if inSet({'db', '.db', 'str'}, mnemonic):
            return parseDeclDB(parts, addr, linenumber, 1)
        
        if mnemonic == 'dw' or mnemonic == '.dw':
            return parseDeclDB(parts, addr, linenumber, 2)

        if mnemonic == 'ds' or mnemonic == '.ds':
            size = evaluateExpression(' '.join(parts[1:]), addr)
            if size >= 0:
                for i in xrange(size):
                    setmem8(addr+i, 0)
                return size
            return -1
        
        if parts[0].startswith(';'):
            return 0

        # nothing else works, it must be a label
        if labelTag == None:
            splat = mnemonic.split(':')
            labelTag = splat[0]
            markLabel(labelTag, addr, linenumber)
            parts = parts[1:]
            if len(splat) > 1:
                parts = filter(lambda x: len(x)>0, [':'.join(splat[1:])]) + parts
            continue
        
        mem[addr] = -2
        return -1 # error
    return 0 # empty

# -- output --

def labelList(labels):
    return (
        ['<pre>Labels:</pre><div class="hordiv"/><pre class="labeltable">'] +
        ["<span class='%s%s' onclick=\"return gotoLabel('%s');\">%-24s%4s</span>%s" % 
            (['t1','t2'][(col + 1) % 4 == 0], ' errorline' if label_val < 0 else '', 
                label_id, label_id, hex16(label_val),
                '<br/>' if (col + 1) % 4 == 0 else '')
            for col, (label_id, label_val) in 
                enumerate(sorted(labels.items(), key=lambda x:x[1])) if label_id != None and len(label_id) > 0] +
        ['</pre>'])

def dumpspan(bytes, mode):
    conv = [lambda i, x: '%s%s' % ([' ', '-'][(i > 0) and (i % 8 == 0)], 
                ('  ' if x == None else errorSpan(hex8(x)) if x < 0 else hex8(x))), # hexes with separators
            lambda i, x: char8(x)][mode]                                            # characters
    return (''.join([conv(i, x) for i, x in enumerate(bytes)]) 
                if reduce(lambda x,y: x or y != None, bytes, False) else False)

def dump(mem):
    return (['<pre>Memory dump:</pre><div class="hordiv"></div>'] +
            ['<pre class="d%d">%s: %s  %s</pre><br/>' % (line % 2, hex16(addr), dumpspan(memr, 0), dumpspan(memr, 1))
            for line, addr, memr in slicing_enumerate2(mem, 0, 16, lambda x: x != None)])

def intelHex():
    pureHex = []
    i = 0
    while i < len(mem):
        j = i
        while j < len(mem) and mem[j] == None:
            j += 1
        i = j
        if i >= len(mem):
            break
        line, rec = ":", ""
        cs, j = 0, 0
        while j < 32 and mem[i+j] != None:
            if mem[i+j] < 0:
                mem[i+j] = 0
            rec += hex8(mem[i+j])
            cs += mem[i+j]
            j += 1

        cs += j
        line += hex8(j)    # byte count
        cs += (i >> 8) & 255
        cs += i & 255
        line += hex16(i)   # record address
        cs += 0
        line += "00"       # record type 0, data
        line += rec

        cs = 0xff&(-(cs&255))
        line += hex8(cs)
        pureHex += [line]
        i += j
    return pureHex + [':00000001FF']


def getLabel(l):
    return labels.get(l.lower())

def processRegUsage(instr, linenumber):
    usage = regUsage.get(linenumber)
    if usage != None:
        # check indirects
        indirectsidx = -1
        if '#' in usage:
            indirectsidx = usage.index('#')
        indirects = None
        directs = ""
        if indirectsidx != -1:
            indirects = usage[indirectsidx + 1:]
            directs = usage[0:indirectsidx]
        else:
            directs = usage

        if indirects != None:
            regs = "','rg".join([''] + indirects)[2:] + "'"
            rep1 = '<span onmouseover="return rgmouseover([%s]);" onmouseout="return rgmouseout([%s]);">\\1</span>'
            rep1 = rep1 % (regs,regs)
            instr = re.sub(r'(\w+)', rep1, instr, count = 1)

        if len(directs) == 2:
            # reg, reg
            s1 = "rg" + directs[0]
            s2 = "rg" + directs[1]
            rep1 = '<span class="%s" ' % s1
            rep1 += 'onmouseover="return rgmouseover(\'%s\');" ' % s1
            rep1 += 'onmouseout="return rgmouseout(\'%s\');">\\2</span>' % s1
            rep2 = '<span class="%s" ' % s2
            rep2 += 'onmouseover="return rgmouseover(\'%s\');" ' % s2
            rep2 += 'onmouseout="return rgmouseout(\'%s\');">\\3</span>' % s2
            replace = '\\1%s, %s' % (rep1, rep2)
            instr = re.sub(r'(.+\s)([abcdehlm])\s*,\s*([abcdehlm])', replace, instr, count = 1)
        elif len(directs) == 1:
            rpname = directs[0]
            if rpname[0] == '@':
                rpname = rpname[1:]
                # register pair
                s1 = "rg" + rpname
                rep1 = '<span class="%s" ' % s1
                rep1 += 'onmouseover="return rgmouseover(\'%s\');" ' % s1
                rep1 += 'onmouseout="return rgmouseout(\'%s\');">\\2</span>' % s1
                replace = '\\1'+rep1
                instr = re.sub(r'([^\s]+[\s]+)([bdh]|sp)', replace, instr, count = 1)
            else:
                # normal register
                s1 = "rg" + rpname
                rep1 = '<span class="%s" ' % s1
                rep1 += 'onmouseover="return rgmouseover(\'%s\');" ' % s1
                rep1 += 'onmouseout="return rgmouseout(\'%s\');">\\2</span>' % s1
                replace = '\\1'+rep1
                instr = re.sub(r'([^\s]+[\s]+)([abcdehlm])', replace, instr, count = 1)
    return instr

def hexorize(bytes, prefix=''):
    return reduce(lambda x, y: x + hex8(y) + ' ', bytes, prefix)

def commentSpan(comment, pre=False):
    pretag = ['<pre>', '</pre>'] if pre else [''] * 2
    return '%s<span class="cmt">%s</span>%s' % (pretag[0], comment, pretag[1])

def errorSpan(comment, pre=False):
    pretag = ['<pre>', '</pre>'] if pre else [''] * 2
    return '%s<span class="errorline">%s</span>%s' % (pretag[0], comment, pretag[1])

def listingLineUncond(i, labeltext, remainder, length, addr):
    lineResult = ''
    comment = ''
    semicolon = remainder.find(';')
    if semicolon != -1:
        comment = remainder[semicolon:]
        remainder = remainder[:semicolon]
    remainder = processRegUsage(remainder, i)

    id = "l" + str(i)
    labelid = "label" + str(i)
    remid = "code" + str(i)

    hexlen = [length, 4][length > 4]
    unresolved = len([x for x in mem[addr:addr + hexlen] if x < 0]) > 0

    lineResult += '<pre id="%s"%s>' % (id, ' class="errorline" ' if unresolved or errors.get(i) != None else '')
    lineResult += '<span class="adr">%s</span>\t' % [' ', hex16(addr)][length > 0] # address
    lineResult += hexorize(mem[addr : addr + hexlen])                              # hexes
    lineResult += ' ' * (16 - hexlen * 3)
    if len(labeltext) > 0:
        lineResult += ('<span class="l" id="%s" onmouseover="return mouseovel(%d);"'
            ' onmouseout="return mouseout(%d);">%s</span>') % (labelid, i, i, labeltext)

    tmp = len(remainder)
    remainder = remainder.lstrip()
    lineResult += ' ' * (tmp - len(remainder))
    if len(remainder) > 0:
        lineResult += ('<span id="%s" onmouseover="return mouseover(%d);"' 
            ' onmouseout="return mouseout(%d);">%s</span>') % (remid, i, i, remainder)
    if len(comment) > 0:
        lineResult += commentSpan(comment, pre=False)

    # display only first and last lines of a long db section
    if hexlen < length:
        lineResult += '<br/>\t.&nbsp;.&nbsp;.&nbsp;<br/>'
        endofs = (length / 4) * 4
        if length % 4 == 0:
            endofs -= 4
        lineResult += hexorize(mem[addr + endofs : addr + length], 
            prefix = hex16(addr + endofs) + '\t') + '<br/>'
    
    lineResult += '</pre>'
    return lineResult

def listingLine(i, line, length, addr, listOn, skipLineCount):
    labeltext = ''
    remainder = line
    parts = re.split(r'[\:\s]', line, 1)
    if len(parts) > 1:
        if getLabel(parts[0]) != -1 and not parts[0].strip().startswith(';'):
            labeltext = parts[0]
            remainder = line[len(labeltext):]

    if remainder.strip().startswith(".nolist"):
        return commentSpan("                        ; list generation turned off", pre=True), False
    elif remainder.strip().startswith(".list"):
        return commentSpan("                        ; skipped %d lines" % skipLineCount, pre=True), True
    elif not listOn:
        return '', False
    else:
        return listingLineUncond(i, labeltext, remainder, length, addr), True

def listing(text, lengths, addresses, doHexDump = False):
    result = ['']
    listOn = True
    skipLineCount = 0
    for i, (line, length, addr) in enumerate(zip(text, lengths, addresses)):
        lineResult, listOn = listingLine(i, line, length, addr, listOn, skipLineCount)
        result += [lineResult]
        skipLineCount = [(skipLineCount + 1), 0][listOn]

    result += labelList(labels)

    result += ["<div>&nbsp;</div>"]

    if True or not makeListing:
        if doHexDump:
            result += dump(mem)
        result += ["<div>&nbsp;</div>"]
        result += ['<br/>'.join(intelHex())]
        result += ["<div>&nbsp;</div>"]
    return ''.join(result)

def error(line, text):
    errors[line] = text

def readInput(filename):
    result = []
    with open(filename) as lefile:
        for text in lefile:
            text = text.rstrip()
            parts = filter(lambda x: len(x) > 0, re.split(r'\s+', text.split(';')[0]))
            result += [";;; include %s begin" % parts[1]] + readInput(parts[1]) + [";;; include %s end" % parts[1]] \
                if len(parts) > 1 and parts[0].lower() == '.include' \
                else [text]
    return result

# assembler main entry point
def assemble(filename):
    global references, textlabels, regUsage

    inputlines = readInput(filename)
    lengths = [None] * len(inputlines)
    addresses = [None] * len(inputlines)
    addr = 0
    labelsCount = 0
    labels = {}
    resolveTable = {}
    mem = [None] * 65536
    backrefWindow = False
    references = [None] * len(inputlines)
    textlabels = [None] * len(inputlines)
    errors = {}
    regUsage = {}
    doHexDump = True
    
    for line in xrange(len(inputlines)):
        encodedLine = toTargetEncoding(inputlines[line].strip(), targetEncoding)
        size = parseInstruction(encodedLine, addr, line)
        if size <= -100000:
            addr = -size-100000
            size = 0
        elif size < 0:
            error(line, "syntax error")
            size = -size
        lengths[line] = size;
        addresses[line] = addr;
        addr += size;
    
    resolveLabelsTable()
    evaluateLabels()
    resolveLabelsInMem()

    return listing(inputlines, lengths, addresses, doHexDump)


def substituteBitwiseOps(x):
    if x.group(0) == 'and': return '&'
    elif x.group(0) == 'or': return '|'
    elif x.group(0) == 'xor': return '^'
    elif x.group(0) == 'shl': return '<<'
    elif x.group(0) == 'shr': return '>>'
    else: return m

def evaluateExpression(input, addr):
    #print "EE0:", input
    try:
        input = re.sub(r'\$([0-9a-fA-F]+)', r'0x\1', input)
        input = re.sub(r"(^|[^'])\$|\.", ' ' + str(addr) + ' ', input, re.I)
        input = re.sub(r'([\d\w]+)\s(shr|shl|and|or|xor)\s([\d\w]+)', r'(\1 \2 \3)', input, re.I)
        input = re.sub(r'\b(shl|shr|xor|or|and|[+\-*\/()])\b', substituteBitwiseOps, input)
        q = re.split(r'<<|>>|[+\-*\/()\^\&]', input)
    except Exception, e:
        return -1
    expr = ''
    for qident in q:
        qident = qident.strip()
        if resolveNumber(qident) != -1:
            continue
        addr = labels.get(qident)
        if addr != None:
            if addr >= 0:
                expr += '_%s=%s;\n' % (qident, str(addr))
                input = re.sub(r'\b' + qident + r'\b', '_' + qident, input, re.M)
            else:
                expr = ''
                break
    expr += re.sub(r"0x[0-9a-fA-F]+|[0-9][0-9a-fA-F]*[hbqdHBQD]|'.'", lambda x: str(resolveNumber(x.group(0))), input)
    try:
        return eval(expr.lower())
    except Exception, e:
        pass
    return -1;


def evaluateLabels():
    for i in labels.keys():
        label = labels[i]
        if label < 0 and resolveTable.get(-label) == None:
            result = evaluateExpression(i, -1)
            if result >= 0:
                resolveTable[-label] = result
                labels[i] = None

def resolveLabelsInMem():
    i = 0
    memsize = len(mem)
    while i < memsize:
        negativeId = mem[i]
        if negativeId != None and negativeId < 0:
            #print "resolveLabelsInMem negativeId=", negativeId
            newvalue = resolveTable.get(-negativeId)
            if newvalue != None:
                mem[i] = newvalue & 0xff
            i += 1
            if mem[i] == negativeId:
                if newvalue != None:
                    mem[i] = 0xff & (newvalue >> 8)
                i += 1
        else:
            i += 1
 

def resolveLabelsTable():
    for i in labels.keys():
        label = labels[i]
        if label < 0:
            addr = resolveTable.get(-label)
            if addr != None:
                labels[i] = addr


def preamble():
    return '\n'.join([
        '<?xml version="1.0" encoding="UTF-8"?><!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"> <html lang="en" xmlns="http://www.w3.org/1999/xhtml" xml:lang="ru"> <head> <title>Pretty 8080 Assembler</title>',
        '<script type="text/javascript"><!--  --></script>',
        '<script type="text/javascript" src="navigate.js"></script>',
        '<link href="listn.css" rel="stylesheet" type="text/css" media="screen"/>',
        '<style type="text/css">',
        '.rga { color: lightgray; }',
        '.rgb { color: lightgray; }',
        '.rgc { color: lightgray; }',
        '.rgd { color: lightgray; }',
        '.rge { color: lightgray; }',
        '.rgh { color: lightgray; }',
        '.rgl { color: lightgray; }',
        '.rgm { color: lightgray; }',
        '.rgsp { color: lightgray; }',
        '.rpb { color: lightgray; }',
        '.rpd { color: lightgray; }',
        '.rph { color: lightgray; }',
        '.rpsp { color: lightgray; }',
        '</style>',
        '<body id="main" onload="loaded(); return false;" onresize="updateSizes(); return false;">',
        '<div id="list">'])
    return r

def tail():
    return '</div></body></html>'

def jsons():
    return '\n'.join(['<div style="display:none" id="json_references">\n' + json.dumps(references) + '</div>',
        '<div style="display:none" id="json_textlabels">\n' + json.dumps(textlabels) + '</div>\n'])

def printusage():
    print "Usage: pasm.py inputfilename"

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
                lst.write(preamble())
                lst.write(assemble(inputFileName))
                lst.write(jsons())
                lst.write(tail())
        #except Exception, e:
            #print str(e)
            #print e
            #sys.exit(1)

    if hexFileName != None:
        try:
            with open(hexFileName, "w") as hf:
                hf.write('\n'.join(intelHex()))
        except Exception, e:
            print str(e)
            sys.exit(1)


if __name__ == "__main__":
   main(sys.argv[1:])