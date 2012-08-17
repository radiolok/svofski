import math

class Sprite:
    def makeAll(self):
        self.makeTable('ltr')
        self.makeGroup('ltr')
        self.makeTable('rtl')
        self.makeGroup('rtl')

    def isWhite(self):
        return False

    def getSpriteName(self, orientation, shift):
        shiftstr = ['_inf'] + range(0,8) + ['_sup']
        return '%s_%s%s' % (self.getName(), orientation, shiftstr[shift+1])

    def spriteRange(self, orientation):
        return xrange([-1,0][orientation == 'rtl'],8) # [8,9][orientation == 'rtl'])

    def includeShift(self, shift):
        return True

    def makeTable(self, orientation):
        print '%s_%s_dispatch:' % (self.getName(), orientation)
        print '\n'.join(['\tdw %s' % self.getSpriteName(orientation, x) for x in self.spriteRange(orientation)])


    def makeGroup(self, orientation):
        print '%s_%s_spritegroup:' % (self.getName(), orientation)

        for shift in self.spriteRange(orientation):
            print '%s:' % self.getSpriteName(orientation, shift)
            if (self.includeShift(shift)):
                self.makeAsm(shift, orientation)
            else:
                print '  ; (shift %d skipped)' % shift
        print ';; end of sprite group %s' % self.getName()

    def makeAsm(self, shift, orientation):
        print '\tlxi h, 0'
        print '\tdad sp'
        print '\tshld sprites_scratch'    

        if not self.isWhite():
            layer = self.makeLayer('13578', shift, orientation)     # layer 0
            if (len(layer) > 0):
                print ';; layer 0 (8000)'
                print '\tmov h, d'        
                print '\tmov l, e'        
                print '\tsphl'
                print '\tlxi b, 0'
                print layer

            layer = self.makeLayer('2367', shift, orientation)      # layer 1
            if (len(layer) > 0):
                print ';; layer 1 (a000)'
                print '\tlxi h, $2000'
                print '\tdad d'
                print '\tsphl'
                print layer

            layer = self.makeLayer('4567', shift, orientation)        # layer 2
            if (len(layer) > 0):
                print ';; layer 2 (c000)'
                print '\tlxi h, $4000'
                print '\tdad d'
                print '\tsphl'
                print layer

            layer = self.makeLayer('8', shift, orientation)        # layer 3
            if (len(layer) > 0):
                print ';; layer 3 (e000)'
                print '\tlxi h, $6000'
                print '\tdad d'
                print '\tsphl'
                print layer
        else:
            # only white layer
            print '\txchg'        
            print '\tlxi d, $4000'
            print '\tdad d'
            print '\tsphl'
            print '\tlxi b, 0'
            print ';; white'
            print self.makeLayer('4', shift, orientation)    


        print '\tlhld sprites_scratch'
        print '\tsphl'
        print '\tret'

    def makeLayer(self, layerchar, shift, orientation):
        result = ''
        comment = ''

        pic = self.getPic(shift, 
                mirror = orientation == 'rtl', 
                prepend = shift == -1, 
                append = (shift == 0) and (orientation == 'rtl'))

        # pre-filter pic to find out its top and bottom boundaries

        boundsmap = map(lambda x: self.filter(x, layerchar), pic)
        for sss in zip(pic,boundsmap): 
            comment = comment + (';[%s] %010x\n' % sss)

        leading = self.countLeading(boundsmap, 0)/2
        trailing = self.countTrailing(boundsmap, 0)/2
        comment = comment + '; stripped pairs: leading: %d trailing: %d\n' % (leading,trailing)


        height = len(pic)
        width = len(pic[0])
        columns = width / 8

        lastb = -1

        # skip initial leading count
        if leading * 2 == height:
            return result
        if leading > 1:
            result = result + '\tlxi h, $%x\n' % (0xffff - (leading*2 - 1))
            result = result + '\tdad sp\n'
            result = result + '\tsphl\n'
        else:
            for i in xrange(0, leading):
                result = result + '\tpush b\n'


        for column in xrange(0,columns):
            for y in xrange(leading * 2, height - trailing * 2, 2):   
                popor = pic[y][column*8:column*8+8] + pic[y+1][column*8:column*8+8]
                b = self.filter(popor, layerchar)
                if b == 0:
                    result = result + '\tpush b\n'
                else:
                    if b != lastb:
                        result = result +('\tlxi h, $%04x\n' % b)
                        lastb = b
                    result = result + '\tpush h\n'
            if column != columns - 1:
                result = result + ('\tlxi h, 256+%d\n\tdad sp\n\tsphl\n' % (height - leading*2 - trailing * 2))
                lastb = -1

        return comment + result

    def getPic(self, shift, prepend = False, append = False, mirror = False):
        if (shift == -1): shift = 0
        unshifted = self.getPicRaw()
        if (self.isDoubleWidth()):
            unshifted = map(self.doublify, unshifted)
        if (mirror):  unshifted = map(lambda x: x[::-1], unshifted)
        if (prepend): unshifted = map(lambda x: ' '*8 + x, unshifted)
        if (append):  unshifted = map(lambda x: x + ' '*8, unshifted)
        if shift == 0:
            return unshifted
        else:
            return map(lambda x: ' '*shift + x + ' '*(8-shift), unshifted)
                
    def doublify(self, chars):
        return ''.join([x + x for x in chars])

    def isDoubleWidth(self):
        return False

    def filter(self, chars, charset):
        return reduce(lambda x,y: (x<<1)|y, [[0,1][x in charset] for x in chars])

    def countLeading(self, lst, match):
        lst = lst + [1]
        for i in xrange(len(lst)):
            if (lst[i] != match): 
                break
        return i

    def countTrailing(self, lst, match):
        for i in xrange(len(lst)):
            if (lst[len(lst) - 1 - i] != match): break
        return i
    

class Ship(Sprite):
    pic = ['      66        ',
           '      66        ',
           '    6666        ',
           '  66666666      ',
           '2222222222222222',
           '22222222222222  ',
           '55555555555     ',
           '  55555555      ']

    def getPicRaw(self):
        return self.pic

    def isDoubleWidth(self):
        return True

    def getName(self):
        return "ship"

class Copter(Sprite):
    pic = ['       7777777  ',
           '77   77777777777',
           '77   77777777777',
           '6666666666666666', 
           '77     7777777  ', 
           '77     7777777  ',
           '         777    ',
           '       7777777  ']

    def getPicRaw(self):
        return self.pic

    def isDoubleWidth(false):
        return False

    def getName(self):
        return "copter"

class RedCopter(Sprite):
    pic = ['       2222222  ',
           '22   22222222222',
           '22   22222222222',
           '3333333333333333', 
           '22     2222222  ', 
           '22     2222222  ',
           '         222    ',
           '       2222222  ']

    def getPicRaw(self):
        return self.pic

    def isDoubleWidth(false):
        return False

    def getName(self):
        return "redcopter"


class PropellerA(Sprite):
    pic = ['     444444     ',
           '         4444444',
           '         4444444',
           '         444    ']

    def getPicRaw(self):
        return self.pic

    def isDoubleWidth(false):
        return False

    def getName(self):
        return "propellerA"

    def isWhite(self):
        return True

class PropellerB(Sprite):
    pic = ['         4444444',
           '     4444444    ',
           '     4444444    ',
           '         444    ']

    def getPicRaw(self):
        return self.pic

    def isDoubleWidth(false):
        return False

    def getName(self):
        return "propellerB"

    def isWhite(self):
        return True

class Jet(Sprite):
    pic = ['44              ',
           '4444      4444  ',
           '4444444444444444', 
           '  4444   4444444', 
           '      44444     ',
           '    4444        ']

    def getPicRaw(self):
        return self.pic

    def isDoubleWidth(false):
        return False

    def isWhite(self):
        return True

    def getName(self):
        return "jet"

    def includeShift(self, shift):
        return shift in [-1, 0, 4]



        
print ';; Automatically generated file'
print ';; see makesprites.py'

a = Ship()
a.makeAll()

a = Copter()
a.makeAll()

RedCopter().makeAll()

PropellerA().makeAll()
PropellerB().makeAll()
Jet().makeAll()

