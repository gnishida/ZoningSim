#
# Draw score curve
#

from numpy import *
import pylab as plt
import sys

argvs = sys.argv

score_list = []
for line in open(argvs[1], 'r'):
	score_list.append(float(line))

plt.plot(xrange(len(score_list)), score_list)
#plt.ylim(0, 0.5);

plt.savefig("score_curve.png")
plt.show()