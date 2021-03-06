﻿#include "ControlWidget.h"
#include <QFileDialog>
#include "MainWindow.h"
#include "GLWidget3D.h"

ControlWidget::ControlWidget(MainWindow* mainWin) : QDockWidget("Control Widget", (QWidget*)mainWin) {
	this->mainWin = mainWin;

	// set up the UI
	ui.setupUi(this);
	ui.horizontalSliderOpacity->setMinimum(0);
	ui.horizontalSliderOpacity->setMaximum(100);
	ui.horizontalSliderOpacity->setValue(100);
	ui.lineEditRandomSeed->setText("0");
	ui.lineEditNumSteps->setText("100");
	ui.lineEditMoveRate->setText("0.5");
	ui.checkBoxSaveScores->setChecked(true);
	ui.checkBoxSaveBestZoning->setChecked(true);
	ui.lineEditNumRandomGeneration->setText("10");
	
	connect(ui.radioButtonZones, SIGNAL(clicked()), this, SLOT(onViewChanged()));
	connect(ui.radioButtonAccessibility, SIGNAL(clicked()), this, SLOT(onViewChanged()));
	connect(ui.radioButtonNeighborPopulation, SIGNAL(clicked()), this, SLOT(onViewChanged()));
	connect(ui.radioButtonNeighborCommercial, SIGNAL(clicked()), this, SLOT(onViewChanged()));
	connect(ui.radioButtonPollution, SIGNAL(clicked()), this, SLOT(onViewChanged()));
	connect(ui.radioButtonSlope, SIGNAL(clicked()), this, SLOT(onViewChanged()));
	connect(ui.radioButtonLandValue, SIGNAL(clicked()), this, SLOT(onViewChanged()));
	connect(ui.radioButtonPopulation, SIGNAL(clicked()), this, SLOT(onViewChanged()));
	connect(ui.radioButtonCommercialJobs, SIGNAL(clicked()), this, SLOT(onViewChanged()));
	connect(ui.radioButtonIndustrialJobs, SIGNAL(clicked()), this, SLOT(onViewChanged()));
	connect(ui.radioButtonLife, SIGNAL(clicked()), this, SLOT(onViewChanged()));
	connect(ui.radioButtonShop, SIGNAL(clicked()), this, SLOT(onViewChanged()));
	connect(ui.radioButtonFactory, SIGNAL(clicked()), this, SLOT(onViewChanged()));
	connect(ui.radioButtonAll, SIGNAL(clicked()), this, SLOT(onViewChanged()));

	connect(ui.horizontalSliderOpacity, SIGNAL(valueChanged(int)), this, SLOT(onOpacityChanged(int)));
	
	connect(ui.pushButtonInit, SIGNAL(clicked()), this, SLOT(onInit()));
	connect(ui.pushButtonNextStep, SIGNAL(clicked()), this, SLOT(onNextStep()));
	connect(ui.pushButtonRandomGeneration, SIGNAL(clicked()), this, SLOT(onRandomGeneration()));

	ui.radioButtonZones->setChecked(true);

	hide();	
}

void ControlWidget::onViewChanged() {
	mainWin->glWidget->updateGL();
}

void ControlWidget::onOpacityChanged(int value) {
	mainWin->glWidget->updateGL();
}

void ControlWidget::onInit() {
	mainWin->glWidget->zoning->init(ui.lineEditRandomSeed->text().toInt());
	mainWin->glWidget->updateGL();
}

void ControlWidget::onNextStep() {
	mainWin->glWidget->zoning->nextSteps(ui.lineEditNumSteps->text().toInt(), ui.lineEditMoveRate->text().toFloat(), ui.checkBoxSaveScores->isChecked(), ui.checkBoxSaveBestZoning->isChecked(), ui.checkBoxSaveZonings->isChecked());
	mainWin->glWidget->updateGL();
}

void ControlWidget::onRandomGeneration() {
	mainWin->glWidget->zoning->testRandomGeneration(ui.lineEditNumRandomGeneration->text().toInt());
}
