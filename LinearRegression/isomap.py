from sklearn.datasets import load_digits
from sklearn.manifold import Isomap
import matplotlib.pyplot as plt

## データの読み込み
digits = load_digits()
X = digits.data
y = digits.target
target_names = digits.target_names

## Isomap
n_neighbors=30
isomap = Isomap(n_neighbors=30, n_components=2)
X_iso = isomap.fit(X).transform(X)

## colors
colors = [plt.cm.nipy_spectral(i/10., 1) for i in range(10)]

## plot
plt.figure()
for c, target_name  in zip(colors, target_names):
    plt.scatter(X_iso[y == target_name, 0], X_iso[y == target_name, 1], c=c, label = target_name)
plt.legend()
plt.title('Isomap')
plt.show()