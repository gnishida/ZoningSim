﻿#include "Zoning.h"
#include "Util.h"
#include "GraphUtil.h"
#include <QElapsedTimer>

//#define DEBUG	0

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
	neighborPopulation = Mat_<float>::zeros(grid_size, grid_size);
	neighborCommercial = Mat_<float>::zeros(grid_size, grid_size);
	pollution = Mat_<float>::zeros(grid_size, grid_size);
	slope = Mat_<float>::zeros(grid_size, grid_size);
	landValue = Mat_<float>::zeros(grid_size, grid_size);
	population = Mat_<float>::zeros(grid_size, grid_size);
	commercialJobs = Mat_<float>::zeros(grid_size, grid_size);
	industrialJobs = Mat_<float>::zeros(grid_size, grid_size);
	
	init();
}

/**
 * 道路をセットする。
 */
void Zoning::setRoads(RoadGraph& roads) {
	this->roads = roads;

	computeAccessibility();
}

/**
 * 指定された乱数シードを使って、ゾーンを初期化する。
 *
 * @param rand_seed		乱数シード
 */
void Zoning::init(int rand_seed) {
	landValue = Mat_<float>::zeros(grid_size, grid_size);
	population = Mat_<float>::zeros(grid_size, grid_size);
	commercialJobs = Mat_<float>::zeros(grid_size, grid_size);
	industrialJobs = Mat_<float>::zeros(grid_size, grid_size);

	srand(rand_seed);

	// ゾーンをランダムに初期化
	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			float n = Util::genRand(0, 10);

			if (n <= 6) {
				zones(r, c) = TYPE_RESIDENTIAL;
			} else if (n <= 8) {
				zones(r, c) = TYPE_COMMERCIAL;
			} else if (n <= 9) {
				zones(r, c) = TYPE_MIXED;
			} else {
				zones(r, c) = TYPE_INDUSTRIAL;
			}
		}
	}


	// 人、仕事を初期化
	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			if (zones(r, c) == TYPE_RESIDENTIAL) {
				population(r, c) = (int)Util::genRand(50, 350);
			} else if (zones(r, c) == TYPE_COMMERCIAL) {
				commercialJobs(r, c) = (int)Util::genRand(50, 350);
			} else if (zones(r, c) == TYPE_INDUSTRIAL) {
				industrialJobs(r, c) = (int)Util::genRand(50, 350);
			} else if (zones(r, c) == TYPE_MIXED) {
				population(r, c) = (int)(Util::genRand(50, 350) * 0.5);
				commercialJobs(r, c) = (int)(Util::genRand(50, 350) * 0.5);
			}
		}
	}


	computeNeighborPopulation();
	computeNeighborCommercial();
	computePollution();

	updateLandValue();

	computeLife();
	computeShop();
	computeFactory();

	cout << "Score: " << computeScore() << endl;
	cout << "Initialized." << endl;
	cout << endl;
}

/**
 * シミュレーションをnumStepsステップ進める。
 *
 * @param numSteps			シミュレーションのステップ数
 * @param more_rate			各ステップで動かす人・仕事の割合
 * @param saveScores		各ステップのスコアを保存するか？
 * @param saveBestZoning	ベストスコアのゾーニングを保存するか？
 * @param saveZonings		各ステップのゾーニングを保存するか？
 */
void Zoning::nextSteps(int numSteps, float move_rate, bool saveScores, bool saveBestZoning, bool saveZonings) {
	elapsedTimes.clear();

	FILE* fp;
	if (saveScores) {
		fp = fopen("scores.txt", "w");
	}

	Mat_<uchar> best_zones;
	float best_score = -numeric_limits<float>::max();

	for (int iter = 0; iter < numSteps; ++iter) {
		updateLandValue();

		computeLife();
		computeShop();
		computeFactory();

		updatePeopleAndJobs(move_rate);

		updateZones();

		computeNeighborPopulation();
		computeNeighborCommercial();
		computePollution();

		float score = computeScore();

		if (saveScores) {
			fprintf(fp, "%lf\n", score);
		}

		if (score > best_score) {
			best_score = score;
			zones.copyTo(best_zones);
		}

		if (saveZonings) {
			char filename[256];
			sprintf(filename, "zone_%d.png", iter);
			saveZoneImage(zones, filename);
		}
	}

	if (saveScores) {
		fclose(fp);
	}

	if (saveBestZoning) {
		cout << "Best score: " << best_score << endl;
		cout << endl;
		saveZoneImage(best_zones, "best_zone.png");
	}


	cout << "computeNeighborPopulation(): " << elapsedTimes["computeNeighborPopulation"] << " [sec]" << endl;
	cout << "computeNeighborComercial(): " << elapsedTimes["computeNeighborComercial"] << " [sec]" << endl;
	cout << "computePollution(): " << elapsedTimes["computePollution"] << " [sec]" << endl;
	cout << "updateLandValue(): " << elapsedTimes["updateLandValue"] << " [sec]" << endl;
	cout << "updatePeopleAndJobs(): " << elapsedTimes["updatePeopleAndJobs"] << " [sec]" << endl;
	cout << "computeLife(): " << elapsedTimes["computeLife"] << " [sec]" << endl;
	cout << "computeShop(): " << elapsedTimes["computeShop"] << " [sec]" << endl;
	cout << "computeFactory()(): " << elapsedTimes["computeFactory"] << " [sec]" << endl;
	cout << "updateZones(): " << elapsedTimes["updateZones"] << " [sec]" << endl;
	cout << endl;
	cout << "Score: " << computeScore() << endl;
	cout << "... next steps done.\n" << endl;
	cout << endl;
}

void Zoning::testRandomGeneration(int num) {
	FILE* fp = fopen("features.txt", "w");

	time_t timer;
	time(&timer);
	for (int iter = 0; iter < num; ++iter) {
		init(timer + iter);

		nextSteps(10, 0.5, false, false, false);

		vector<float> feature = computeFeature(zones);
		for (int k = 0; k < feature.size(); ++k) {
			fprintf(fp, "%lf,", feature[k]);
		}
		fprintf(fp, "%lf\n", computeScore());
	}

	fclose(fp);
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
		float oneWay = roads.graph[*ei]->oneWay ? 0.5f : 1.0f;
		for (int i = 0; i < polyline.size() - 1; ++i) {
			QVector2D pt = cityToGrid(polyline[i]);
			if (pt.x() < 0 || pt.x() >= grid_size) continue;
			if (pt.y() < 0 || pt.y() >= grid_size) continue;

			// 道路セグメントの長さ（一方通行の場合は、半分にする）
			float len = (polyline[i + 1] - polyline[i]).length() * oneWay;

			// このセルを中心に、3x3セルの範囲で、道路長を追加していく
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

	float cell_length2 = cell_length * cell_length;
	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			accessibility(r, c) = std::max(weights["highway_accessibility"] * road_length[0](r, c) / cell_length2, std::max(weights["avenue_accessibility"] * road_length[1](r, c) / cell_length2, weights["street_accessibility"] * road_length[2](r, c) / cell_length2));
			accessibility(r, c) = min(accessibility(r, c), 1.0f);
			//accessibility(r, c) = min(weights["highway_accessibility"] * road_length[0](r, c) / cell_length2  + weights["avenue_accessibility"] * road_length[1](r, c) / cell_length2 + weights["streeet_accessibility"] * road_length[2](r, c) / cell_length2, 1.0f);
		}
	}
}

/**
 * 周辺の人口を計算する。
 */
void Zoning::computeNeighborPopulation() {
	QElapsedTimer timer;
	timer.start();

	neighborPopulation = Mat_<float>::zeros(grid_size, grid_size);

	// アクティビティが広がる最大距離
	const float dist_max = 1000.0f;

	int window_size = dist_max / cell_length + 0.5f;

	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			// 当該セルの周辺セルに、人口を追加する
			for (int dr = -window_size; dr <= window_size; ++dr) {
				if (r + dr < 0 || r + dr >= grid_size) continue;
				for (int dc = -window_size; dc <= window_size; ++dc) {
					if (c + dc < 0 || c + dc >= grid_size) continue;

					float dist = sqrt(SQR(dc * cell_length) + SQR(dr * cell_length));
					neighborPopulation(r + dr, c + dc) += weights["population_neighbor"] * population(r, c) / MAX_JOBS / expf(weights["distance_neighbor_population"] * dist);
				}
			}
		}
	}

	// 最大値を1にする
	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			neighborPopulation(r, c) = min(neighborPopulation(r, c), 1.0f);
		}
	}

	elapsedTimes["computeNeighborPopulation"] += timer.elapsed() * 0.001;

#ifdef DEBUG
	cout << "Neighbor population: " << endl;
	cout << neighborPopulation << endl;
#endif
}

/**
 * 周辺の商業を計算する。
 */
void Zoning::computeNeighborCommercial() {
	QElapsedTimer timer;
	timer.start();

	neighborCommercial = Mat_<float>::zeros(grid_size, grid_size);

	// アクティビティが広がる最大距離
	const float dist_max = 1000.0f;

	int window_size = dist_max / cell_length + 0.5f;

	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			// 当該セルの周辺セルに、商業を追加する
			for (int dr = -window_size; dr <= window_size; ++dr) {
				if (r + dr < 0 || r + dr >= grid_size) continue;
				for (int dc = -window_size; dc <= window_size; ++dc) {
					if (c + dc < 0 || c + dc >= grid_size) continue;

					float dist = sqrt(SQR(dc * cell_length) + SQR(dr * cell_length));
					neighborCommercial(r + dr, c + dc) += weights["commercial_neighbor"] * commercialJobs(r, c) / MAX_JOBS / expf(weights["distance_neighbor_commercial"] * dist);
				}
			}
		}
	}

	// 最大値を1にする
	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			neighborCommercial(r, c) = min(neighborCommercial(r, c), 1.0f);
		}
	}

	elapsedTimes["computeNeighborComercial"] += timer.elapsed() * 0.001;

#ifdef DEBUG
	cout << "Neighbor population: " << endl;
	cout << neighborPopulation << endl;
#endif
}

/**
 * 汚染度を計算する
 */
void Zoning::computePollution() {
	QElapsedTimer timer;
	timer.start();

	pollution = Mat_<float>::zeros(grid_size, grid_size);

	// 汚染が広がる最大距離
	const float dist_max = 1000.0f;

	int window_size = dist_max / cell_length + 0.5f;

	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			// 当該工業ゾーンの周辺セルに、汚染度を追加する
			for (int dr = -window_size; dr <= window_size; ++dr) {
				if (r + dr < 0 || r + dr >= grid_size) continue;
				for (int dc = -window_size; dc <= window_size; ++dc) {
					if (c + dc < 0 || c + dc >= grid_size) continue;

					float dist = sqrt(SQR(dc * cell_length) + SQR(dr * cell_length));
					pollution(r + dr, c + dc) += weights["industrial_pollution"] * industrialJobs(r, c) / MAX_JOBS / exp(weights["distance_pollution"] * dist);
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

	elapsedTimes["computePollution"] += timer.elapsed() * 0.001;

#ifdef DEBUG
	cout << "Pollution: " << endl;
	cout << pollution << endl;
#endif
}

/**
 * 地価を更新する。
 * 地価は、0からMAX_LANDVALUEの範囲の値をとる。
 */
void Zoning::updateLandValue() {
	QElapsedTimer timer;
	timer.start();

	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			float expected_landValue = weights["accessibility_landvalue"] * accessibility(r, c)
				+ weights["neighbor_population_landvalue"] * neighborPopulation(r, c)
				+ weights["neighbor_commercial_landvalue"] * neighborCommercial(r, c)
				+ weights["pollution_landvalue"] * pollution(r, c)
				+ weights["slope_landvalue"] * slope(r, c)
				+ weights["population_landvalue"] * population(r, c) / MAX_POPULATION
				+ weights["commercialjobs_landvalue"] * commercialJobs(r, c) / MAX_JOBS
				+ weights["industrialjobs_landvalue"] * industrialJobs(r, c) / MAX_JOBS;
			if (expected_landValue < 0) expected_landValue = 0.0f;
			if (expected_landValue > MAX_LANDVALUE) expected_landValue = MAX_LANDVALUE;

			//landValue(r, c) += (expected_landValue - landValue(r, c)) * 0.1f;
			landValue(r, c) = expected_landValue;
		}
	}

	elapsedTimes["updateLandValue"] += timer.elapsed() * 0.001;

#ifdef DEBUG
	cout << "Land value:" << endl;
	cout << landValue << endl;
#endif
}

/**
 * 人口と仕事を更新する。
 *
 * @param ratio		移動する比率
 */
void Zoning::updatePeopleAndJobs(float ratio) {
	QElapsedTimer timer;
	timer.start();

	// 全人口を計算する
	float total_population = matsum(population);

	// 全仕事量を計算する
	float total_commercialJobs = matsum(commercialJobs);
	float total_industrialJobs = matsum(industrialJobs);

	// 人口を移動する
	removePeople(total_population * ratio);
	addPeople(total_population * ratio);

	// 仕事を移動する
	removeCommercialJobs(total_commercialJobs * ratio);
	addCommercialJobs(total_commercialJobs * ratio);

	// 仕事を移動する
	removeIndustrialJobs(total_industrialJobs * ratio);
	addIndustrialJobs(total_industrialJobs * ratio);

	elapsedTimes["updatePeopleAndJobs"] += timer.elapsed() * 0.001;

#ifdef DEBUG
	cout << "People: " << matsum(population) << endl;
	cout << "Com jobs: " << matsum(commercialJobs) << endl;
	cout << "Ind jobs: " << matsum(industrialJobs) << endl;
#endif
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
			int r, c;
			while (true) {
				r = Util::genRand(0, grid_size);
				c = Util::genRand(0, grid_size);
				if (population(r, c) / MAX_POPULATION + commercialJobs(r, c) / MAX_JOBS + industrialJobs(r, c) / MAX_JOBS < 1.0f) break;
			}
			cells[i] = QVector2D(c, r);
			pdf[i] = life(c, r);
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
#ifdef DEBUG
	cout << commercialJobs << endl;
#endif

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
			int r, c;
			while (true) {
				r = Util::genRand(0, grid_size);
				c = Util::genRand(0, grid_size);
				if (population(r, c) / MAX_POPULATION + commercialJobs(r, c) / MAX_JOBS + industrialJobs(r, c) / MAX_JOBS < 1.0f) break;
			}
			cells[i] = QVector2D(c, r);
			pdf[i] = shop(c, r);
		}

		int id = Util::sampleFromPdf(pdf);
		commercialJobs(cells[id].y(), cells[id].x())++;
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
			int r, c;
			while (true) {
				r = Util::genRand(0, grid_size);
				c = Util::genRand(0, grid_size);
				if (population(r, c) / MAX_POPULATION + commercialJobs(r, c) / MAX_JOBS + industrialJobs(r, c) / MAX_JOBS < 1.0f) break;
			}
			cells[i] = QVector2D(c, r);
			pdf[i] = factory(c, r);
		}

		int id = Util::sampleFromPdf(pdf);
		industrialJobs(cells[id].y(), cells[id].x())++;
		num--;
	}
}

/**
 * 生活の快適さの指標を計算する。
 */
void Zoning::computeLife() {
	QElapsedTimer timer;
	timer.start();

	life = Mat_<float>(grid_size, grid_size);

	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			life(r, c) = lifeValue(c, r);
		}
	}

	elapsedTimes["computeLife"] += timer.elapsed() * 0.001;

#ifdef DEBUG
	cout << "Life:" << endl;
	cout << life << endl;
#endif
}

/**
 * 店をオープンする指標を計算する。
 */
void Zoning::computeShop() {
	QElapsedTimer timer;
	timer.start();

	shop = Mat_<float>(grid_size, grid_size);

	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			shop(r, c) = shopValue(c, r);
		}
	}

	elapsedTimes["computeShop"] += timer.elapsed() * 0.001;

#ifdef DEBUG
	cout << endl << "Shop:" << endl;
	cout << shop << endl;
#endif
}

/**
 * 工場をオープンする指標を計算する。
 */
void Zoning::computeFactory() {
	QElapsedTimer timer;
	timer.start();

	factory = Mat_<float>(grid_size, grid_size);

	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			factory(r, c) = factoryValue(c, r);
		}
	}

	elapsedTimes["computeFactory"] += timer.elapsed() * 0.001;

#ifdef DEBUG
	cout << endl << "Factory:" << endl;
	cout << factory << endl;
#endif
}

/**
 * 指定されたセルの生活価値を返却する。
 */
float Zoning::lifeValue(int x, int y, float max_value) {
	float v = weights["accessibility_life"] * accessibility(y, x)
		+ weights["neighbor_population_life"] * neighborPopulation(y, x)
		+ weights["neighbor_commercial_life"] * neighborCommercial(y, x)
		+ weights["pollution_life"] * pollution(y, x)
		+ weights["slope_life"] * slope(y, x)
		+ weights["landvalue_life"] * landValue(y, x) / MAX_LANDVALUE
		+ weights["population_life"] * population(y, x) / MAX_POPULATION
		+ weights["commercialjobs_life"] * commercialJobs(y, x) / MAX_JOBS
		+ weights["industrialjobs_life"] * industrialJobs(y, x) / MAX_JOBS;

	// 桁あふれを防ぐため
	return expf(v - max_value);
}

float Zoning::shopValue(int c, int r, float max_value) {
	float v = weights["accessibility_shop"] * accessibility(r, c)
		+ weights["neighbor_population_shop"] * neighborPopulation(r, c)
		+ weights["neighbor_commercial_shop"] * neighborCommercial(r, c)
		+ weights["pollution_shop"] * pollution(r, c)
		+ weights["slope_shop"] * slope(r, c)
		+ weights["landvalue_shop"] * landValue(r, c) / MAX_LANDVALUE
		+ weights["population_shop"] * population(r, c) / MAX_POPULATION
		+ weights["commercialjobs_shop"] * commercialJobs(r, c) / MAX_JOBS
		+ weights["industrialjobs_shop"] * industrialJobs(r, c) / MAX_JOBS;

	// 桁あふれを防ぐため
	return expf(v - max_value);
}

float Zoning::factoryValue(int c, int r, float max_value) {
	float v = weights["accessibility_factory"] * accessibility(r, c)
		+ weights["neighbor_population_factory"] * neighborPopulation(r, c)
		+ weights["neighbor_commercial_factory"] * neighborCommercial(r, c)
		+ weights["pollution_factory"] * pollution(r, c)
		+ weights["slope_factory"] * slope(r, c)
		+ weights["landvalue_factory"] * landValue(r, c) / MAX_LANDVALUE
		+ weights["population_factory"] * population(r, c) / MAX_POPULATION
		+ weights["commercialjobs_factory"] * commercialJobs(r, c) / MAX_JOBS
		+ weights["industrialjobs_factory"] * industrialJobs(r, c) / MAX_JOBS;

	// 桁あふれを防ぐため
	return expf(v - max_value);
}

/**
 * スコアを計算する。
 */
float Zoning::computeScore() {
	float score = 0.0f;
	float total_population = 0.0f;

	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			score += life(r, c) * population(r, c);
			score += shop(r, c) * commercialJobs(r, c);
			score += factory(r, c) * industrialJobs(r, c);
			total_population += population(r, c) + commercialJobs(r, c) + industrialJobs(r, c);
		}
	}

	return score / total_population;
}

/**
 * ゾーンを更新する。
 */
void Zoning::updateZones() {
#ifdef DEBUG
	cout << "Population:" << endl;
	cout << population << endl;
	cout << "Com jobs:" << endl;
	cout << commercialJobs << endl;
	cout << "Ind jobs:" << endl;
	cout << industrialJobs << endl;
#endif

	QElapsedTimer timer;
	timer.start();

	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			if (industrialJobs(r, c) / MAX_JOBS > population(r, c) / MAX_POPULATION && industrialJobs(r, c) > commercialJobs(r, c)) {
				zones(r, c) = TYPE_INDUSTRIAL;
			} else if (population(r, c) / MAX_POPULATION < 0.1 && commercialJobs(r, c) / MAX_JOBS < 0.1 && industrialJobs(r, c) / MAX_JOBS < 0.1) {
				zones(r, c) = TYPE_PARK;
			} else if (population(r, c) / MAX_POPULATION > commercialJobs(r, c) / MAX_JOBS * 2) {
				zones(r, c) = TYPE_RESIDENTIAL;
			} else if (commercialJobs(r, c) / MAX_JOBS > population(r, c) / MAX_POPULATION * 2) {
				zones(r, c) = TYPE_COMMERCIAL;
			} else {
				zones(r, c) = TYPE_MIXED;
			}
		}
	}

	elapsedTimes["updateZones"] += timer.elapsed() * 0.001;
}

void Zoning::saveZoneImage(const Mat_<uchar>& zones, char* filename) {
	Mat tmp(grid_size, grid_size, CV_8UC3);
	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			Vec3b p;
			if (zones(r, c) == TYPE_RESIDENTIAL) {
				p = Vec3b(0, 0, 255);
			} else if (zones(r, c) == TYPE_COMMERCIAL) {
				p = Vec3b(255, 0, 0);
			} else if (zones(r, c) == TYPE_INDUSTRIAL) {
				p = Vec3b(0, 255, 255);
			} else if (zones(r, c) == TYPE_MIXED) {
				p = Vec3b(255, 0, 255);
			} else if (zones(r, c) == TYPE_PARK) {
				p = Vec3b(0, 204, 0);
			}
			tmp.at<Vec3b>(r, c) = p;
		}
	}

	flip(tmp, tmp, 0);
	imwrite(filename, tmp);
}

QVector2D Zoning::gridToCity(const QVector2D& pt) {
	return (pt + QVector2D(0.5f, 0.5f)) / (float)grid_size * city_length - QVector2D(city_length, city_length) * 0.5f;
}

QVector2D Zoning::cityToGrid(const QVector2D& pt) {
	float x = pt.x() / (float)city_length * grid_size + (float)grid_size * 0.5f;
	float y = pt.y() / (float)city_length * grid_size + (float)grid_size * 0.5f;

	return QVector2D(floor(x), floor(y));
}

float Zoning::matsum(Mat_<float>& mat) {
	Mat_<float> temp;
	reduce(mat, temp, 0, CV_REDUCE_SUM);
	reduce(temp, temp, 1, CV_REDUCE_SUM);
	return temp(0, 0);
}

float Zoning::matmax(Mat_<float>& mat) {
	Mat_<float> temp;
	reduce(mat, temp, 0, CV_REDUCE_MAX);
	reduce(temp, temp, 1, CV_REDUCE_MAX);
	return temp(0, 0);
}

vector<float> Zoning::computeFeature(const Mat_<uchar>& zones) {
	Mat_<float> f = Mat_<float>::zeros(5, 5);
	int count = 0;

	for (int r = 0; r < grid_size; ++r) {
		for (int c = 0; c < grid_size; ++c) {
			for (int dr = -1; dr <= 1; ++dr) {
				if (r + dr < 0 || r + dr >= grid_size) continue;
				for (int dc = -1; dc <= 1; ++dc) {
					if (c + dc < 0 || c + dc >= grid_size) continue;
					if (dr == 0 && dc == 0) continue;
					if (dr != 0 && dc != 0) continue;

					int t1 = zones(r, c);
					int t2 = zones(r+dr, c+dc);
					f(t1, t2)++;
					count++;
				}
			}
		}
	}

	f /= (float)count;

	vector<float> ret;
	for (int r = 0; r < f.rows; ++r) {
		for (int c = r; c < f.cols; ++c) {
			ret.push_back(f(r, c));
		}
	}
	
	return ret;
}

