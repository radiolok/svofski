#!/usr/bin/env python

import re
import json
import sys
import getopt
from os.path import splitext

NOLIST = ".nolist"
LIST   = ".list"

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

class MemoryDump:
    @staticmethod
    def span(bytes, conv):
        return (''.join([conv(i, x) for i, x in enumerate(bytes)]) 
                    if reduce(lambda x,y: x or y != None, bytes, False) else False)
    @classmethod
    def spanHex(cls, bytes):
        return cls.span(bytes, lambda i, x: '%s%s' % ([' ', '-'][(i > 0) and (i % 8 == 0)], 
                    ('  ' if x == None else errorSpan(hex8(x)) if x < 0 else hex8(x))))

    @classmethod
    def spanChar(cls, bytes):
        return cls.span(bytes, lambda i, x: char8(x) if x != None else ' ')

    @classmethod
    def dump(cls, mem):
        return (['<pre>Memory dump:</pre><div class="hordiv"></div>'] +
                ['<pre class="d%d">%s: %s  %s</pre><br/>' % (line % 2, hex16(addr), cls.spanHex(memr), cls.spanChar(memr))
                for line, addr, memr in slicing_enumerate2(mem, 0, 16, lambda x: x != None)])

class IntelHex:
    @staticmethod
    def intelHex(mem):
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

class Memory:
    mem = [None] * 65536
    def __getitem__(self, key):
        return self.mem[key]
    def __setitem__(self, key, value):
        self.mem[key] = value
    def __len__(self):
        return len(self.mem)
    def set16(self, addr, immediate):
        if immediate >= 0:
            self[addr] = immediate & 0xff
            self[addr+1] = immediate >> 8
        else:
            self[addr] = immediate
            self[addr+1] = immediate
    def set8(self, addr, immediate):
        self[addr] = [immediate, immediate & 0xff][immediate >= 0]

class LineData:
    LENGTH, ADDR, REFERENCE, TEXTLABEL, FLAGS = range(5)
    data = []
    def __init__(self):
        self.data = [None] * 5    
    def __getitem__(self, key):
        return self.data[key]
    def __setitem__(self, key, value):
        self.data[key] = value
    def __repr__(self):
        return "[len=%s; addr=%s; ref=%s; textlabel=%s; flags=%s]" % tuple((str(x) for x in self.data))

def substituteBitwiseOps(x):
    if x.group(0) == 'and': return '&'
    elif x.group(0) == 'or': return '|'
    elif x.group(0) == 'xor': return '^'
    elif x.group(0) == 'shl': return '<<'
    elif x.group(0) == 'shr': return '>>'
    else: return x.group(0)

class Resolver:
    labels = {}
    resolveTable = {} # label negative id, resolved address
    errors = {}
    labelsCount = 0

    def __init__(self, linedata):
        self.linedata = linedata

    def setError(self, line, text):
        self.errors[line] = text

    def getError(self, key):
        return self.errors.get(key)

    def getLabels(self):
        return self.labels

    def markLabel(self, identifier, address, linenumber = None, override = False):
        identifier = identifier.strip()
        identifier = re.sub(r'\$([0-9a-fA-F]+)', r'0x\1', identifier)
        identifier = re.sub(r"(^|[^'])(\$|\.)", ' ' + str(address) + ' ', identifier)
        number = evaluateNumber(identifier)
        if number != -1: 
            return number
        if linenumber == None:
            self.labelsCount += 1
            address = -1 - self.labelsCount
        identifier = identifier.lower()
        found = self.labels.get(identifier)
        if found != None:
            if address >= 0:
                self.resolveTable[-found] = address
            else:
                address = found
        if (found == None) or override:
            self.labels[identifier] = address
        if linenumber != None:
            self.linedata[linenumber][LineData.TEXTLABEL] = identifier

        return address
    
    def referencesLabel(self, identifier, linenumber):
        #if self.linedata[linenumber][LineData.REFERENCE] == None:
            self.linedata[linenumber][LineData.REFERENCE] = identifier.lower()

    def evaluateLabels(self):
        for i in self.labels.keys():
            label = self.labels[i]
            if label < 0 and self.resolveTable.get(-label) == None:
                result = self.evaluateExpression(i, -1)
                if result >= 0:
                    self.resolveTable[-label] = result
                    self.labels[i] = None

    def resolveLabelsInMem(self, mem):
        i = 0
        while i < len(mem):
            negativeId = mem[i]
            if negativeId != None and negativeId < 0:
                newvalue = self.resolveTable.get(-negativeId)
                if newvalue != None:
                    mem[i] = newvalue & 0xff
                i += 1
                if mem[i] == negativeId:
                    if newvalue != None:
                        mem[i] = 0xff & (newvalue >> 8)
                    i += 1
            else:
                i += 1 

    def resolveLabelsTable(self):
        for i in self.labels.keys():
            label = self.labels[i]
            if label < 0:
                addr = self.resolveTable.get(-label)
                if addr != None:
                    self.labels[i] = addr

    def evaluateExpression(self, input, addr, linenumber = None):
        try:
            input = re.sub(r'\$([0-9a-fA-F]+)', r'0x\1', input)
            input = re.sub(r"(^|[^'])\$|\.", ' ' + str(addr) + ' ', input, re.I)
            input = re.sub(r'([\d\w]+)\s(shr|shl|and|or|xor)\s([\d\w]+)', r'(\1 \2 \3)', input, re.I)
            input = re.sub(r'\b(shl|shr|xor|or|and|[+\-*\/()])\b', substituteBitwiseOps, input)
            q = re.split(r'<<|>>|[+\-*\/()\^\&\|]', input)
        except Exception, e:
            print '%s: error: evaluateExpression [%s]' % (repr(linenumber), input)
            return -1
        vars = []
        for qident in q:
            qident = qident.strip()
            if evaluateNumber(qident) != -1:
                continue
            addr = self.labels.get(qident)
            if linenumber != None and len(qident) > 0:
                self.referencesLabel(qident, linenumber)
                #print linenumber, 'referencesLabel was', self.linedata[linenumber][LineData.REFERENCE], '->', repr(qident), ' because ', input, ' but', self.linedata[linenumber][LineData.REFERENCE]
            if addr != None:
                if addr >= 0:
                    vars += [('_'+qident, addr)]
                    input = re.sub(r'\b' + qident + r'\b', '_' + qident, input)
                else:
                    break
        expr = re.sub(r"0x[0-9a-fA-F]+|[0-9][0-9a-fA-F]*[hbqdHBQD]|'.'", lambda x: str(evaluateNumber(x.group(0))), input)
        try:
            for variable, value in vars:
                locals()[variable] = value
            return eval(expr.lower())
        except Exception, e:
            pass
        return -1

    @staticmethod
    def getExpr(arr):
        ex = ' '.join(arr).strip()
        if ex[0] == '"' or ex[0] == "'":
            return ex
        return ex.split(';')[0]

    def useExpr(self, s, addr, linenumber):
        expr = self.getExpr(s)
        if expr == None or len(expr.strip()) == 0: 
            return False
        immediate = self.markLabel(expr, addr)
        #print 'EVAL useExpr ', repr(expr), repr(linenumber)
        #self.referencesLabel(expr, linenumber)
        self.evaluateExpression(expr, addr, linenumber) ## BOB
        return immediate

    def resolve(self, mem):
        self.resolveLabelsTable()
        self.evaluateLabels()
        self.resolveLabelsInMem(mem)

class RegUsage:
    usageMap = {}

    def __getitem__(self, key): return self.usageMap[key]
    def __setitem__(self, key, value): self.usageMap[key] = value

    def process(self, instr, linenumber):
        usage = self.usageMap.get(linenumber)
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

ops0 = {
    "nop": "00",
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
    s = s.strip().lower()
    if len(s) > 1: 
        return -1
    return "bcdehlma".find(s[0])

def evaluateNumber(identifier):
    if identifier == None or len(identifier) == 0: 
        return -1
    else:
        identifier = identifier.strip().lower()
    if (identifier[0] == "'" or identifier[0] == '"') and (len(identifier) == 3):
        return 0xff & ord(identifier[1])
    if identifier.startswith('0x'):
        identifier = '0' + identifier[2:] + 'h'
    if identifier[0] == '$':
        identifier = '0' + identifier[1:] + 'h'
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

def tokenDBDW(s, addr, longueur, linenumber):
    global resolver

    if len(s) == 0: 
        return 0

    n = resolver.markLabel(s, addr)
    resolver.referencesLabel(s, linenumber)

    if longueur == None: 
        longueur = 1
    size = -1
    if longueur == 1 and n < 256:
        mem.set8(addr, n)
        size = 1
    elif longueur == 2 and n < 65536:
        mem.set16(addr, n)
        size = 2
    return size

def tokenString(s, addr, linenumber):
    return len([mem.set8(addr+i, ord(s[i])) for i in xrange(len(s))])

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
                break
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

def parseInstruction(text, addr, linenumber, regUsage):
    global resolver 

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
            elif mnemonic in {"ral", "rar", "rla", "rra", "cma"}:
                regUsage[linenumber] = ['#', 'a']
            return 1
        
        # immediate word
        opcs = opsIm16.get(mnemonic)
        if opcs != None:
            mem[addr] = int(opcs, 16)
            immediate = resolver.useExpr(parts[1:], addr, linenumber)
            mem.set16(addr+1, immediate)
            if mnemonic in {"lhld", "shld"}:
                regUsage[linenumber] = ['#', 'h', 'l']
            elif mnemonic in {"lda", "sta"}:
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
            immediate = resolver.useExpr(subparts[1:], addr, linenumber)
            mem.set16(addr+1, immediate)
            regUsage[linenumber] = ['@'+subparts[0].strip()]
            if subparts[0].strip() in {"h","d"}:
                rpmap = {"h":"l","d":"e"}
                regUsage[linenumber] += ['#', rpmap[subparts[0].strip()]]
            return 3

        # immediate byte       
        opcs = opsIm8.get(mnemonic)
        if opcs != None:
            mem[addr] = int(opcs, 16)
            immediate = resolver.useExpr(parts[1:], addr, linenumber)
            mem.set8(addr+1, immediate)
            if mnemonic in {"sui", "sbi", "xri", "ori", "ani", "adi", "aci", "cpi"}:
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
            immediate = resolver.useExpr(subparts[1:], addr, linenumber)
            mem.set8(addr+1, immediate)
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
            if mnemonic in opsRegDst:
                reg = reg << 3
            mem[addr] = int(opcs, 16) | reg
            regUsage[linenumber] = [parts[1].strip()]
            if mnemonic in {"ora", "ana", "xra", "add", "adc", "sub", "sbc", "cmp"}:
                regUsage[linenumber] += ['#', 'a']
            return 1
        
        # single register pair
        opcs = opsRp.get(mnemonic)
        if opcs != None:
            rp = parseRegisterPair(parts[1])
            if rp == -1:
                return -1
            mem[addr] = int(opcs, 16) | (rp << 4)
            regUsage[linenumber] = ['@'+parts[1].strip()]
            if mnemonic == "dad":
                regUsage[linenumber] += ['#', 'h', 'l']
            elif mnemonic in {"inx", "dcx"}:
                if parts[1].strip() in {"h","d"}:
                    rpmap = {"h":"l","d":"e"}
                    regUsage[linenumber] += ['#', rpmap[parts[1].strip()]]
            return 1
        
        # rst
        if mnemonic == "rst":
            n = evaluateNumber(parts[1])
            if n >= 0 and n < 8:
                mem[addr] = 0xC7 | (n << 3)
                return 1
            return -1;
        
        if mnemonic == ".org" or mnemonic == "org":
            n = resolver.evaluateExpression(' '.join(parts[1:]), addr)
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
            return mnemonic

        # assign immediate value to label
        if mnemonic == ".equ" or mnemonic == "equ":
            if labelTag == None: 
                return -1
            value = resolver.evaluateExpression(' '.join(parts[1:]), addr)
            resolver.markLabel(labelTag, value, linenumber, True)
            return 0

        if mnemonic == ".encoding":
            encoding = ' '.join(parts[1:])
            try:
                encoded = toTargetEncoding('test', encoding)
                targetEncoding = encoding
            except Exception, e:
                return -1
            return -100000

        if mnemonic in {'cpu', 'aseg', '.aseg'}:
            return 0;

        if mnemonic in {'db', '.db', 'str'}:
            #print 'text=%s parts=%s' % (repr(text), repr(parts))
            return parseDeclDB(parts, addr, linenumber, 1)
        
        if mnemonic == 'dw' or mnemonic == '.dw':
            return parseDeclDB(parts, addr, linenumber, 2)

        if mnemonic == 'ds' or mnemonic == '.ds':
            size = resolver.evaluateExpression(' '.join(parts[1:]), addr)
            if size >= 0:
                for i in xrange(size):
                    mem.set8(addr+i, 0)
                return size
            return -1
        
        if parts[0].startswith(';'):
            return 0

        # nothing else works, it must be a label
        if labelTag == None:
            splat = mnemonic.split(':')
            labelTag = splat[0]
            resolver.markLabel(labelTag, addr, linenumber)
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
        ['<pre>Labels:</pre><div class="hordiv"></div><pre class="labeltable">'] +
        ["<span class='%s%s' onclick=\"return gotoLabel('%s');\">%-24s%4s</span>%s" % 
            (['t1','t2'][(col + 1) % 4 == 0], ' errorline' if label_val < 0 else '', 
                label_id, label_id, hex16(label_val),
                '<br/>' if (col + 1) % 4 == 0 else '')
            for col, (label_id, label_val) in 
                enumerate(sorted(labels.items(), key=lambda x:x[1])) if label_id != None and len(label_id) > 0] +
        ['</pre>'])

def hexorize(bytes, prefix=''):
    return reduce(lambda x, y: x + hex8(y) + ' ', bytes, prefix)

def commentSpan(comment, pre=False):
    pretag = ['<pre>', '</pre>'] if pre else [''] * 2
    return '%s<span class="cmt">%s</span>%s' % (pretag[0], comment, pretag[1])

def errorSpan(comment, pre=False):
    pretag = ['<pre>', '</pre>'] if pre else [''] * 2
    return '%s<span class="errorline">%s</span>%s' % (pretag[0], comment, pretag[1])

def listingLineUncond(i, remainder, linedata, regUsage):
    global resolver

    addr = linedata[LineData.ADDR]
    textlabel = linedata[LineData.TEXTLABEL]

    if textlabel != None:
        labeltext = remainder[:remainder.lower().index(textlabel) + len(textlabel)]
        remainder = remainder[len(labeltext):]
    else:
        labeltext = ''
    comment = ''
    semicolon = remainder.find(';')
    if semicolon != -1:
        comment = remainder[semicolon:]
        remainder = remainder[:semicolon]
    remainder = regUsage.process(remainder, i)

    id = "l" + str(i)
    labelid = "label" + str(i)
    remid = "code" + str(i)

    length = linedata[LineData.LENGTH]
    hexlen = min(length, 4) 
    unresolved = len([x for x in mem[addr:addr + length] if x < 0]) > 0

    #if resolver.getError(i) != None:
    #    print i, 'error=', resolver.getError(i), 'labeltext=', labeltext, 'remainder=', remainder, 'unresolved=', unresolved

    result = (['<pre id="%s"%s>' % (id, ' class="errorline" ' if unresolved or resolver.getError(i) != None else '')] +
        [hexorize(mem[addr : addr + hexlen], prefix='<span class="adr">%s</span>\t' % [' ', hex16(addr)][length>0])] +
        [' ' * (16 - hexlen * 3)])
    if len(labeltext) > 0:
        result += [('<span class="l" id="%s" onmouseover="return mouseovel(%d);"'
            ' onmouseout="return mouseout(%d);">%s</span>') % (labelid, i, i, labeltext)]

    tmp = len(remainder)
    remainder = remainder.lstrip()
    result += [' ' * (tmp - len(remainder))]
    if len(remainder) > 0:
        result += [('<span id="%s" onmouseover="return mouseover(%d);"' 
            ' onmouseout="return mouseout(%d);">%s</span>') % (remid, i, i, remainder)]
    if len(comment) > 0:
        result += [commentSpan(comment, pre=False)]

    # display only first and last lines of a long db section
    if hexlen < length:
        result += ['<br/>']
        if length / 4 > 1: 
            result += ['\t.&nbsp;.&nbsp;.&nbsp;<br/>']
        endofs = (length / 4) * 4
        if length % 4 == 0:
            endofs -= 4
        result += [hexorize(mem[addr + endofs : addr + length], prefix = hex16(addr + endofs) + '\t') + '<br/>']
    
    result += ['</pre>']
    return result

def listingLine(i, line, linedata, regUsage):
    if linedata[LineData.FLAGS] == -1:
        return [commentSpan("                        ; list generation turned off", pre=True)]
    elif linedata[LineData.FLAGS] >= 0:
        return [commentSpan("                        ; skipped %d lines" % linedata[LineData.FLAGS], pre=True)]
    elif linedata[LineData.FLAGS] != None:
        return []
    else:
        return listingLineUncond(i, line, linedata, regUsage)

def listing(text, linedata, regUsage, doHexDump = False):
    global resolver

    result = reduce(lambda x,y: x+y, 
        (listingLine(i, line, linedata, regUsage) for i,(line,linedata) in enumerate(zip(text,linedata))))
    result += labelList(resolver.getLabels())
    result += ["<div>&nbsp;</div>"]

    if True or not makeListing:
        if doHexDump:
            result += MemoryDump.dump(mem)
        result += ["<div>&nbsp;</div>"]
        result += ['<br/>'.join(IntelHex.intelHex(mem))]
        result += ["<div>&nbsp;</div>"]
    return result

# assembler main entry point
def assemble(filename):
    global linedata, mem, resolver

    inputlines = readInput(filename)
    linedata = [LineData() for x in xrange(len(inputlines))]
    resolver = Resolver(linedata)

    addr = 0
    regUsage = RegUsage()
    mem = Memory()
    doHexDump = True
    listOff = False
    listOffCount = 0
    
    for line in xrange(len(inputlines)):
        encodedLine = toTargetEncoding(inputlines[line].strip(), targetEncoding)
        size = parseInstruction(encodedLine, addr, line, regUsage)
        if size == NOLIST:
            listOff = True
            size = 0
        elif size == LIST:
            listOff = False
            linedata[line][LineData.FLAGS] = listOffCount
            listOffCount = 0
            size = 0
        elif size <= -100000:
            addr = -size-100000
            size = 0
        elif size < 0:
            resolver.setError(line, "syntax error")
            size = -size
        linedata[line][LineData.LENGTH] = size
        linedata[line][LineData.ADDR] = addr
        if listOff:
            listOffCount += 1
            linedata[line][LineData.FLAGS] = -listOffCount
        addr += size

    resolver.resolve(mem)
    return listing(inputlines, linedata, regUsage, doHexDump)

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

def preamble():
    return ['<?xml version="1.0" encoding="UTF-8"?><!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">',
        '<html lang="en" xmlns="http://www.w3.org/1999/xhtml" xml:lang="ru"> <head> <title>Pretty 8080 Assembler</title>',
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
        '<div id="list">']

def tail():
    return ['</div></body></html>']

def references(data):
    return [x[LineData.REFERENCE] for x in data]

def textlabels(data):
    return [x[LineData.TEXTLABEL] for x in data]

def flags(data):
    return [x[LineData.FLAGS] for x in data]

def jsons():
    global linedata
    return ['<div style="display:none" id="json_references">\n' + json.dumps(references(linedata)) + '</div>',
        '<div style="display:none" id="json_textlabels">\n' + json.dumps(textlabels(linedata)) + '</div>\n']

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
                lst.write(''.join(preamble() +
                                  assemble(inputFileName) +
                                  jsons() +
                                  tail()))
        #except Exception, e:
            #print str(e)
            #print e
            #sys.exit(1)

    if hexFileName != None:
        try:
            with open(hexFileName, "w") as hf:
                hf.write('\n'.join(IntelHex.intelHex(mem)))
        except Exception, e:
            print str(e)
            sys.exit(1)


if __name__ == "__main__":
   main(sys.argv[1:])