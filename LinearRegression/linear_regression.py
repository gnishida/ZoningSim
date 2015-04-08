'''
Zoningの特徴量を表すベクトルと、そのスコアのデータから、線形回帰を行う。
つまり、score = Axという式に当てはめ、xを求める。
なお、定数項を含めるため、行列Aの一番右の列には1が並ぶ。

SciPyライブラリのLinear algebraパッケージ(linalg)の、least-squaresソルバ(lstsq)を使用する。
lstsq()の使い方：
    第一引数: 行列A
    第二引数: ベクトルb
'''

import numpy as np
from scipy import linalg as LA

from sklearn.decomposition import PCA
import matplotlib.pyplot as plt

A = []
b = []
for line in open("data.txt", 'r'):
	items = line.split(",")
	values = [ float(item) for item in items ]
	tmp = values[0:-1]
	tmp.append(1)
	A.append(tmp)
	b.append(values[-1])

x, residues, rank, s = LA.lstsq(np.array(A), np.array(b))
print x
print residues
