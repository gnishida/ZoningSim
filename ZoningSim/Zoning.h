#pragma once

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "RoadGraph.h"

using namespace std;
using namespace cv;

class Zoning {
public:
	static enum { TYPE_RESIDENTIAL = 0, TYPE_COMMERCIAL = 1, TYPE_INDUSTRIAL = 2, TYPE_PARK = 3, TYPE_UNUSED = 9 };

	static const float MAX_LANDVALUE;
	static const int MAX_POPULATION;
	static const int MAX_JOBS;

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
	Zoning(float city_length, int grid_size, const QMap<QString, float>& weights);

	void setRoads(RoadGraph& roads);
	void nextStep();

private:
	void computeAccessibility();
	void computeActivity();
	void computePollution();
	void updateLandValue();
	void updatePeopleAndJobs();
	void removePeople(int num);
	void addPeople(int num);
	void removeCommercialJobs(int num);
	void addCommercialJobs(int num);
	void removeIndustrialJobs(int num);
	void addIndustrialJobs(int num);
	void updateZones();

	QVector2D gridToCity(const QVector2D& pt);
	QVector2D cityToGrid(const QVector2D& pt);
};

