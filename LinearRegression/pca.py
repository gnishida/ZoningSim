'''
Zoningの特徴量を表すベクトルを、PCAで分析する。
ただし、スコアが0.58以上をgood design、0.41以上をnormal design、0.41未満をbad designとして色分けする。

sklearnライブラリのPCAを使用する。
コンストラクタの引数には、n_components=で射影後の次元数を指定する。
fit()をPCAを計算し、さらに、transformで射影後のベクトルを計算する。
'''

import numpy as np
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

## PCA (2次元に射影する)
pca = PCA(n_components=2)
A_r = pca.fit(A).transform(A)			# transformで、射影を実施する
                              			# A_rは、データ数 x 2 の行列

nb = np.array(b)

## plot
plt.figure()
plt.scatter(A_r[nb < 0.41, 0], A_r[nb < 0.41, 1], c='red', label = "Bad design")
plt.scatter(A_r[np.logical_and(nb >= 0.41, nb <= 0.58), 0], A_r[np.logical_and(nb >= 0.41, nb <= 0.58), 1], c='yellow', label = "Normal design")
plt.scatter(A_r[nb > 0.58, 0], A_r[nb > 0.58, 1], c='blue', label = "Good design")
plt.legend(loc="upper left")
plt.title('PCA')
plt.savefig("pca.png")
plt.show()
