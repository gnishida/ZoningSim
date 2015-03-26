#pragma once

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "RoadGraph.h"

using namespace std;
using namespace cv;

class Zoning {
public:
	static enum { TYPE_RESIDENTIAL = 0, TYPE_COMMERCIAL = 1, TYPE_INDUSTRIAL = 2, TYPE_PARK = 3, TYPE_UNUSED = 9 };

	float city_length;		// cityの一辺の距離 [m]
	float cell_length;	// セルの一辺の距離 [m]
	int grid_size;		// グリッドの一辺のサイズ
	RoadGraph roads;

	QMap<QString, float> weights;

	Mat_<uchar> zones;
	Mat_<float> accessibility;
	Mat_<float> activity;
	Mat_<float> pollution;
	Mat_<float> slope;
	Mat_<float> landValue;
	Mat_<float> population;
	Mat_<float> commercialJobs;
	Mat_<float> industrialJobs;

public:
	Zoning(float city_length, int grid_size);

	void setRoads(RoadGraph& roads);
	void setWeights(const QMap<QString, float>& weights);
	void computeAccessibility();
	void computeActivity();
	float computeActivity(int x, int y);
	void computePollution();
	float computePollution(int x, int y);

private:
	QVector2D gridToCity(const QVector2D& pt);
	QVector2D cityToGrid(const QVector2D& pt);
};

