#pragma once

#include <QWidget>
#include "ui_ParameterSettingWidget.h"
#include <QDialog>

class ParameterSettingWidget : public QDialog {
Q_OBJECT

public:
	Ui::ParameterSettingWidget ui;
	QMap<QString, float>& parameters;	// パラメータの参照を保持（つまり、更新した内容は、オリジナルにも反映される）

public:
	float highway_accessibility;
	float avenue_accessibility;
	float street_accessibility;
	float population_neighbor;
	float distance_neighbor_population;
	float commercial_neighbor;
	float distance_neighbor_commercial;
	float industrial_pollution;
	float distance_pollution;
	float accessibility_landvalue;
	float neighbor_population_landvalue;
	float neighbor_commercial_landvalue;
	float pollution_landvalue;
	float slope_landvalue;
	float population_landvalue;
	float commercialjobs_landvalue;
	float industrialjobs_landvalue;

	float accessibility_life;
	float neighbor_population_life;
	float neighbor_commercial_life;
	float pollution_life;
	float slope_life;
	float landvalue_life;
	float population_life;
	float commercialjobs_life;
	float industrialjobs_life;

	float accessibility_shop;
	float neighbor_population_shop;
	float neighbor_commercial_shop;
	float pollution_shop;
	float slope_shop;
	float landvalue_shop;
	float population_shop;
	float commercialjobs_shop;
	float industrialjobs_shop;

	float accessibility_factory;
	float neighbor_population_factory;
	float neighbor_commercial_factory;
	float pollution_factory;
	float slope_factory;
	float landvalue_factory;
	float population_factory;
	float commercialjobs_factory;
	float industrialjobs_factory;


public:
	ParameterSettingWidget(QWidget* parent, QMap<QString, float>& parameters);

public slots:
	void onOK();
};

