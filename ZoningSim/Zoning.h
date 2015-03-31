#pragma once

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "RoadGraph.h"

using namespace std;
using namespace cv;

class Zoning {
public:
	static enum { TYPE_RESIDENTIAL = 0, TYPE_COMMERCIAL = 1, TYPE_INDUSTRIAL = 2, TYPE_MIXED = 3, TYPE_PARK = 4, TYPE_UNUSED = 9 };

	static const float MAX_LANDVALUE;
	static const int MAX_POPULATION;
	static const int MAX_JOBS;

	float city_length;	// cityの一辺の距離 [m]
	float cell_length;	// セルの一辺の距離 [m]
	int grid_size;		// グリッドの一辺のサイズ
	RoadGraph roads;

	QMap<QString, float> weights;

	Mat_<uchar> zones;


	Mat_<float> accessibility;
	Mat_<float> neighborPopulation;		// 周辺の人口
	Mat_<float> neighborCommercial;		// 周辺の商業の量
	Mat_<float> pollution;
	Mat_<float> slope;
	Mat_<float> landValue;
	Mat_<float> population;
	Mat_<float> commercialJobs;
	Mat_<float> industrialJobs;

	Mat_<float> life;		// 生活の快適さ
	Mat_<float> shop;		// 店をオープンするための指標
	Mat_<float> factory;	// 工場をオープンするための指標

public:
	Zoning(float city_length, int grid_size, const QMap<QString, float>& weights);

	void setRoads(RoadGraph& roads);
	void nextSteps(int numSteps);

private:
	void computeAccessibility();
	//void computeActivity();
	void computeNeighborPopulation();
	void computeNeighborCommercial();


	void computePollution();
	void updateLandValue();
	void updatePeopleAndJobs();
	void removePeople(int num);
	void addPeople(int num);
	void removeCommercialJobs(int num);
	void addCommercialJobs(int num);
	void removeIndustrialJobs(int num);
	void addIndustrialJobs(int num);
	void computeLife();
	void computeShop();
	void computeFactory();
	float lifeValue(int x, int y);
	float shopValue(int x, int y);
	float factoryValue(int x, int y);
	void updateZones();

	QVector2D gridToCity(const QVector2D& pt);
	QVector2D cityToGrid(const QVector2D& pt);
	float matsum(Mat_<float>& mat);
	float matmax(Mat_<float>& mat);
};

