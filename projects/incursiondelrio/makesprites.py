
class Sprite:
    def makeAsm(self):
        print '\tmov h, d'        
        print '\tmov l, e'        
        print '\tsphl'
        print '\tlxi b, 0'
        print ';; green'
        self.makeLayer('v')     # base layer, only green

        print ';;;; black/magenta layer 1'
        print '\tlxi h, $2000'
        print '\tdad d'
        print '\tsphl'
        self.makeLayer('nm')     # layer 1: magenta + black

        print ';;;; yellow/magenta layer 2'
        print '\tlxi h, $4000'
        print '\tdad d'
        print '\tsphl'
        self.makeLayer('ma')    # layer 2: magena + yellow

    def makeLayer(self, layerchar):
        pic = self.getPic()
        height = len(pic)
        width = len(pic[0])
        columns = width / 8

        lastb = -1

        for column in xrange(0,columns):
            for y in xrange(0,height,2):
                popor = pic[y][column*8:column*8+8] + pic[y+1][column*8:column*8+8]
                b = self.filter(popor, layerchar)
                if b == 0:
                    print '\tpush b'
                else:
                    if b != lastb:
                        print '\tlxi h, $%04x' % b
                        lastb = b
                    print '\tpush h'
            if column != columns - 1:
                print '\tlxi h, 256+%d\n\tdad sp\n\tsphl' % height
                lastb = -1

    def getPic(self):
        raw = self.getPicRaw()
        if (self.isDoubleWidth()):
            return map(self.doublify, raw)
                
    def doublify(self, chars):
        return ''.join([x + x for x in chars])

    def isDoubleWidth(self):
        return False

    def filter(self, chars, charset):
        return reduce(lambda x,y: (x<<1)|y, [[0,1][x in charset] for x in chars])
    

class Ship(Sprite):
    pic = ['      nn        ',
           '      nn        ',
           '    nnnn        ',
           '  nnnnnnnn      ',
           'mmmmmmmmmmmmmmmm',
           'mmmmmmmmmmmmmm  ',
           'vvvvvvvvvvv     ',
           '  vvvvvvvvv     ']

    def getPicRaw(self):
        return self.pic

    def isDoubleWidth(self):
        return True

a = Ship()
a.makeAsm()

    
