import png
import sys

r=png.Reader(sys.argv[1])
ipng=r.read()

(width,height)=(ipng[0],ipng[1])

y = 0
msum = 0.0
xsum = 0.0
ysum = 0.0

for row in ipng[2]:
    if y % 20 == 0:
        print "".join(['# .'[row[x*8]] for x in xrange(width/8)])

    msum = msum + sum([[1.0,0,0][x] for x in row])
    xsum = xsum + sum([[1.0,0,0][row[x]]*x for x in xrange(width)])
    ysum = ysum + sum([[1.0,0,0][row[x]]*y for x in xrange(width)])

    y = y + 1

xmidpoint = 1.0*xsum/msum
ymidpoint = 1.0*ysum/msum

print "The image is %dx%d" % (width,height)
print "The mass centre is @%fx%f" % (xmidpoint,ymidpoint)

print "The far vector is %f,%f" % (10*(xmidpoint-width/2.0),10*(ymidpoint-height/2.0))
