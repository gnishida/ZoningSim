#include "Zoning.h"
#include "Util.h"
#include "GraphUtil.h"

const float Zoning::MAX_LANDVALUE = 1000.0f;
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
				population(r, c) = Util::genRand(50, 150);
			} else if (zones(r, c) == TYPE_COMMERCIAL) {
				commercialJobs(r, c) = Util::genRand(50, 150);
			} else if (zones(r, c) == TYPE_INDUSTRIAL) {
				industrialJobs(r, c) = Util::genRand(50, 150);
			}
		}
	}

	computeActivity();
	computePollution();
}

/**
 * 道路をセットする。
 */
void Zoning::setRoads(RoadGraph& roads) {
	this->roads = roads;

	computeAccessibility();
}

/**
 * シミュレーションを１ステップ進める。
 */
void Zoning::nextStep() {
	updateLandValue();

	updatePeopleAndJobs();

	updateZones();
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
			accessibility(r, c) = min(weights["highway_accessibility"] * road_length[0](r, c) + weights["avenue_accessibility"] * road_length[1](r, c) + weights["streeet_accessibility"] * road_length[2](r, c), 1.0f);
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
			// 当該セルの周辺セルに、アクティビティを追加する
			for (int dr = -window_size; dr <= window_size; ++dr) {
				if (r + dr < 0 || r + dr >= grid_size) continue;
				for (int dc = -window_size; dc <= window_size; ++dc) {
					if (c + dc < 0 || c + dc >= grid_size) continue;

					float dist = sqrt(SQR(dc * cell_length) + SQR(dr * cell_length));
					activity(r + dr, c + dc) += commercialJobs(r, c) / MAX_JOBS / (1.0f + weights["distance_activity"] * dist);
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
					pollution(r + dr, c + dc) += 1.0f / (1.0f + weights["distance_pollution"] * dist);
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

/**
 * 地価を更新する。
 */
void Zoning::updateLandValue() {
	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			float expected_landValue = weights["accessibility_landvalue"] * accessibility(r, c)
				+ weights["activity_landvalue"] * activity(r, c)
				+ weights["pollution_landvalue"] * pollution(r, c)
				+ weights["slope_landvalue"] * slope(r, c)
				+ weights["population_landvalue"] * population(r, c) / MAX_POPULATION
				+ weights["commercialjobs_landvalue"] * commercialJobs(r, c) / MAX_JOBS
				+ weights["industrialjobs_landvalue"] * industrialJobs(r, c) / MAX_JOBS;
			if (expected_landValue < 0) expected_landValue = 0.0f;
			if (expected_landValue > MAX_LANDVALUE) expected_landValue = MAX_LANDVALUE;

			landValue(r, c) += (expected_landValue - landValue(r, c)) * 0.1f;
		}
	}

	cout << landValue << endl;
}

/**
 * 人口と仕事を更新する。
 */
void Zoning::updatePeopleAndJobs() {
	// 全人口を計算する
	Mat_<float> sum_population;
	reduce(population, sum_population, 0, CV_REDUCE_SUM);
	reduce(sum_population, sum_population, 1, CV_REDUCE_SUM);
	float total_population = sum_population(0, 0);

	// 全仕事量を計算する
	Mat_<float> sum_commercialJobs;
	reduce(commercialJobs, sum_commercialJobs, 0, CV_REDUCE_SUM);
	reduce(sum_commercialJobs, sum_commercialJobs, 1, CV_REDUCE_SUM);
	float total_commercialJobs = sum_commercialJobs(0, 0);

	// 全仕事量を計算する
	Mat_<float> sum_industrialJobs;
	reduce(industrialJobs, sum_industrialJobs, 0, CV_REDUCE_SUM);
	reduce(sum_industrialJobs, sum_industrialJobs, 1, CV_REDUCE_SUM);
	float total_industrialJobs = sum_industrialJobs(0, 0);

	// 人口増加数を計算
	float d_population = (total_commercialJobs + total_industrialJobs - total_population) * 0.1;

	// 仕事増加数を計算
	float d_commercialJobs = -total_commercialJobs / (total_commercialJobs + total_industrialJobs) * d_population;
	float d_industrialJobs = -total_industrialJobs / (total_commercialJobs + total_industrialJobs) * d_population;

	// 人口を移動する
	if (d_population >= 0) {
		removePeople(total_population * 0.1);
		addPeople(total_population * 0.1 + d_population);
	} else {
		removePeople(total_population * 0.1 - d_population);
		addPeople(total_population * 0.1);
	}

	// 仕事を移動する
	if (d_commercialJobs >= 0) {
		removeCommercialJobs(total_commercialJobs * 0.1);
		addCommercialJobs(total_commercialJobs * 0.1 + d_commercialJobs);
	} else {
		removeCommercialJobs(total_commercialJobs * 0.1 - d_commercialJobs);
		addCommercialJobs(total_commercialJobs * 0.1);
	}

	// 仕事を移動する
	if (d_industrialJobs >= 0) {
		removeCommercialJobs(total_industrialJobs * 0.1);
		addCommercialJobs(total_industrialJobs * 0.1 + d_industrialJobs);
	} else {
		removeCommercialJobs(total_industrialJobs * 0.1 - d_industrialJobs);
		addCommercialJobs(total_industrialJobs * 0.1);
	}
}

/** 
 * 指定された人数を減らす。ランダムにセルを選択し、一人減らす。これを人数分繰り返す。
 */
void Zoning::removePeople(int num) {
	while (num > 0) {
		int r = Util::genRand(0, grid_size);
		int c = Util::genRand(0, grid_size);

		if (population(r, c) > 0) {
			population(r, c)--;
			num--;
		}
	}
}

/** 
 * 指定された人数を増やす。ランダムにセルを選択し、一人増やす。これを人数分繰り返す。
 */
void Zoning::addPeople(int num) {
	const int T = 10;

	while (num > 0) {
		vector<QVector2D> cells(T);
		vector<float> pdf(T);

		for (int i = 0; i < T; ++i) {
			int r = Util::genRand(0, grid_size);
			int c = Util::genRand(0, grid_size);
			cells.push_back(QVector2D(c, r));

			float quality = weights["accessibility_life"] * accessibility(r, c)
				+ weights["activity_life"] * activity(r, c)
				+ weights["pollution_life"] * pollution(r, c)
				+ weights["slope_life"] * slope(r, c)
				+ weights["landvalue_life"] * landValue(r, c)
				+ weights["population_life"] * population(r, c)
				+ weights["commercialjobs_life"] * commercialJobs(r, c)
				+ weights["industrialjobs_life"] * industrialJobs(r, c);
			pdf.push_back(exp(quality));
		}

		int id = Util::sampleFromPdf(pdf);
		population(cells[id].y(), cells[id].x())++;
		num--;
	}
}

/** 
 * 指定された商業仕事を減らす。ランダムにセルを選択し、一人減らす。これを指定された数だけ繰り返す。
 */
void Zoning::removeCommercialJobs(int num) {
	while (num > 0) {
		int r = Util::genRand(0, grid_size);
		int c = Util::genRand(0, grid_size);

		if (commercialJobs(r, c) > 0) {
			commercialJobs(r, c)--;
			num--;
		}
	}
}

/** 
 * 指定された商業仕事を増やす。ランダムにセルを選択し、一人増やす。これを指定された数だけ繰り返す。
 */
void Zoning::addCommercialJobs(int num) {
	const int T = 10;

	while (num > 0) {
		vector<QVector2D> cells(T);
		vector<float> pdf(T);

		for (int i = 0; i < T; ++i) {
			int r = Util::genRand(0, grid_size);
			int c = Util::genRand(0, grid_size);
			cells.push_back(QVector2D(c, r));

			float quality = weights["accessibility_shop"] * accessibility(r, c)
				+ weights["activity_shop"] * activity(r, c)
				+ weights["pollution_shop"] * pollution(r, c)
				+ weights["slope_shop"] * slope(r, c)
				+ weights["landvalue_shop"] * landValue(r, c)
				+ weights["population_shop"] * population(r, c)
				+ weights["commercialjobs_shop"] * commercialJobs(r, c)
				+ weights["industrialjobs_shop"] * industrialJobs(r, c);
			pdf.push_back(exp(quality));
		}

		int id = Util::sampleFromPdf(pdf);
		population(cells[id].y(), cells[id].x())++;
		num--;
	}
}

/** 
 * 指定された工業仕事を減らす。ランダムにセルを選択し、一人減らす。これを指定された数だけ繰り返す。
 */
void Zoning::removeIndustrialJobs(int num) {
	while (num > 0) {
		int r = Util::genRand(0, grid_size);
		int c = Util::genRand(0, grid_size);

		if (industrialJobs(r, c) > 0) {
			industrialJobs(r, c)--;
			num--;
		}
	}
}

/** 
 * 指定された工業仕事を増やす。ランダムにセルを選択し、一人増やす。これを指定された数だけ繰り返す。
 */
void Zoning::addIndustrialJobs(int num) {
	const int T = 10;

	while (num > 0) {
		vector<QVector2D> cells(T);
		vector<float> pdf(T);

		for (int i = 0; i < T; ++i) {
			int r = Util::genRand(0, grid_size);
			int c = Util::genRand(0, grid_size);
			cells.push_back(QVector2D(c, r));

			float quality = weights["accessibility_factory"] * accessibility(r, c)
				+ weights["activity_factory"] * activity(r, c)
				+ weights["pollution_factory"] * pollution(r, c)
				+ weights["slope_factory"] * slope(r, c)
				+ weights["landvalue_factory"] * landValue(r, c)
				+ weights["population_factory"] * population(r, c)
				+ weights["commercialjobs_factory"] * commercialJobs(r, c)
				+ weights["industrialjobs_factory"] * industrialJobs(r, c);
			pdf.push_back(exp(quality));
		}

		int id = Util::sampleFromPdf(pdf);
		population(cells[id].y(), cells[id].x())++;
		num--;
	}
}

/**
 * ゾーンを更新する。
 */
void Zoning::updateZones() {
	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			if (population(r, c) / MAX_POPULATION > commercialJobs(r, c) / MAX_JOBS && population(r, c) / MAX_POPULATION > industrialJobs(r, c) / MAX_JOBS) {
				zones(r, c) = TYPE_RESIDENTIAL;
			} else if (commercialJobs(r, c) / MAX_JOBS > industrialJobs(r, c) / MAX_JOBS) {
				zones(r, c) = TYPE_COMMERCIAL;
			} else {
				zones(r, c) = TYPE_INDUSTRIAL;
			}
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