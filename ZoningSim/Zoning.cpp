#include "Zoning.h"
#include "Util.h"
#include "GraphUtil.h"

const int Zoning::MAX_POPULATION = 500;
const int Zoning::MAX_JOBS = 500;

Zoning::Zoning(float city_length, int grid_size, const QMap<QString, float>& weights) {
	this->city_length = city_length;
	this->grid_size = grid_size;
	this->cell_length = city_length / grid_size;
	this->weights = weights;

	zones = Mat_<uchar>(grid_size, grid_size);
	accessibility = Mat_<float>::zeros(grid_size, grid_size);
	activity = Mat_<float>::zeros(grid_size, grid_size);
	pollution = Mat_<float>::zeros(grid_size, grid_size);
	slope = Mat_<float>::zeros(grid_size, grid_size);
	landValue = Mat_<float>::zeros(grid_size, grid_size);
	population = Mat_<float>::zeros(grid_size, grid_size);
	commercialJobs = Mat_<float>::zeros(grid_size, grid_size);
	industrialJobs = Mat_<float>::zeros(grid_size, grid_size);

	// ゾーンをランダムに初期化
	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			zones(r, c) = Util::genRand(0, 4);
		}
	}

	// 人、仕事を初期化
	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			if (zones(r, c) == TYPE_RESIDENTIAL) {
				population(r, c) = Util::genRand(10, 100);
			} else if (zones(r, c) == TYPE_COMMERCIAL) {
				commercialJobs(r, c) = Util::genRand(10, 100);
			} else if (zones(r, c) == TYPE_INDUSTRIAL) {
				industrialJobs(r, c) = Util::genRand(10, 100);
			}
		}
	}

	computeActivity();
	computePollution();
}

void Zoning::setRoads(RoadGraph& roads) {
	this->roads = roads;

	computeAccessibility();
}

/**
 * アクセシビリティを計算する
 * 道路データが変更された時のみ、この関数を呼び出してアクセシビリティを更新すれば良い。
 */
void Zoning::computeAccessibility() {
	// 各セルにおける道路長を計算する
	Mat_<float> road_length[3];
	for (int i = 0; i < 3; ++i) {
		road_length[i] = Mat_<float>::zeros(grid_size, grid_size);
	}
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
		Polyline2D polyline = GraphUtil::finerEdge(roads, *ei, 10.0f);
		for (int i = 0; i < polyline.size() - 1; ++i) {
			QVector2D pt = cityToGrid(polyline[i]);
			if (pt.x() < 0 || pt.x() >= grid_size) continue;
			if (pt.y() < 0 || pt.y() >= grid_size) continue;

			float len = (polyline[i + 1] - polyline[i]).length();

			// このセルを中心に、5x5セルの範囲で、道路長を追加していく
			for (int dx = -2; dx <= 2; ++dx) {
				if (pt.x() + dx < 0 || pt.x() + dx >= grid_size) continue;
				for (int dy = -2; dy <= 2; ++dy) {
					if (pt.y() + dy < 0 || pt.y() + dy >= grid_size) continue;

					float w = 1.0f / (1.0f + sqrtf(SQR(dx) + SQR(dy)));

					if (roads.graph[*ei]->type == RoadEdge::TYPE_HIGHWAY) {
						road_length[0](pt.y() + dy, pt.x() + dx) += len * w;
					} else if (roads.graph[*ei]->type == RoadEdge::TYPE_AVENUE) {
						road_length[1](pt.y() + dy, pt.x() + dx) += len * w;
					} else if (roads.graph[*ei]->type == RoadEdge::TYPE_STREET) {
						road_length[2](pt.y() + dy, pt.x() + dx) += len * w;
					}
				}
			}
		}
	}

	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			accessibility(r, c) = weights["accessibility_highway"] * road_length[0](r, c) + weights["accessibility_avenue"] * road_length[1](r, c) + weights["accessibility_street"] * road_length[2](r, c);
		}
	}
}

/**
 * アクティビティを計算する
 */
void Zoning::computeActivity() {
	activity = Mat_<float>::zeros(grid_size, grid_size);

	// アクティビティが広がる最大距離
	const float dist_max = 1000.0f;

	int window_size = dist_max / cell_length + 0.5f;

	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			if (zones(r, c) != TYPE_RESIDENTIAL && zones(r, c) != TYPE_COMMERCIAL) continue;

			// 当該セルの周辺セルに、アクティビティを追加する
			for (int dr = -window_size; dr <= window_size; ++dr) {
				if (r + dr < 0 || r + dr >= grid_size) continue;
				for (int dc = -window_size; dc <= window_size; ++dc) {
					if (c + dc < 0 || c + dc >= grid_size) continue;

					float dist = sqrt(SQR(dc * cell_length) + SQR(dr * cell_length));
					activity(r + dr, c + dc) += (population(r, c) / MAX_POPULATION + commercialJobs(r, c) / MAX_JOBS) / (1.0f + weights["activity"] * dist);
				}
			}
		}
	}
}

/**
 * 汚染度を計算する
 */
void Zoning::computePollution() {
	pollution = Mat_<float>::zeros(grid_size, grid_size);

	// 汚染が広がる最大距離
	const float dist_max = 1000.0f;

	int window_size = dist_max / cell_length + 0.5f;

	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			if (zones(r, c) != TYPE_INDUSTRIAL) continue;

			// 当該工業ゾーンの周辺セルに、汚染度を追加する
			for (int dr = -window_size; dr <= window_size; ++dr) {
				if (r + dr < 0 || r + dr >= grid_size) continue;
				for (int dc = -window_size; dc <= window_size; ++dc) {
					if (c + dc < 0 || c + dc >= grid_size) continue;

					float dist = sqrt(SQR(dc * cell_length) + SQR(dr * cell_length));
					pollution(r + dr, c + dc) += 1.0f / (1.0f + weights["pollution"] * dist);
				}
			}
		}
	}

	// 最大値を1にする
	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			pollution(r, c) = min(pollution(r, c), 1.0f);
		}
	}
}

QVector2D Zoning::gridToCity(const QVector2D& pt) {
	return (pt + QVector2D(0.5f, 0.5f)) / (float)grid_size * city_length - QVector2D(city_length, city_length) * 0.5f;
}

QVector2D Zoning::cityToGrid(const QVector2D& pt) {
	float x = pt.x() / (float)city_length * grid_size + (float)grid_size * 0.5f;
	float y = pt.y() / (float)city_length * grid_size + (float)grid_size * 0.5f;

	return QVector2D(floor(x), floor(y));
}