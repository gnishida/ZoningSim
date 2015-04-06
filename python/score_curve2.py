#
# Draw score curve
#

from numpy import *
import pylab as plt
import sys

argvs = sys.argv

score_list1 = []
for line in open("scores_step50.txt", 'r'):
	score_list1.append(float(line))

score_list2 = []
for line in open("scores_step30.txt", 'r'):
	score_list2.append(float(line))

score_list3 = []
for line in open("scores_step10.txt", 'r'):
	score_list3.append(float(line))

x = xrange(len(score_list1))
plt.plot(x, score_list1, label="50% move")
plt.plot(x, score_list2, label="30% move")
plt.plot(x, score_list3, label="10% move")
plt.legend(loc="lower right")
#plt.ylim(0, 0.5);

plt.savefig("score_curve.png")
plt.show()