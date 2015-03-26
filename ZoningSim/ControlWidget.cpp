#include "ControlWidget.h"
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
	
	connect(ui.radioButtonZones, SIGNAL(clicked()), this, SLOT(onViewChanged()));
	connect(ui.radioButtonAccessibility, SIGNAL(clicked()), this, SLOT(onViewChanged()));
	connect(ui.radioButtonActivity, SIGNAL(clicked()), this, SLOT(onViewChanged()));
	connect(ui.radioButtonPollution, SIGNAL(clicked()), this, SLOT(onViewChanged()));
	connect(ui.radioButtonSlope, SIGNAL(clicked()), this, SLOT(onViewChanged()));
	connect(ui.radioButtonLandValue, SIGNAL(clicked()), this, SLOT(onViewChanged()));
	connect(ui.radioButtonPopulation, SIGNAL(clicked()), this, SLOT(onViewChanged()));
	connect(ui.radioButtonCommercialJobs, SIGNAL(clicked()), this, SLOT(onViewChanged()));
	connect(ui.radioButtonIndustrialJobs, SIGNAL(clicked()), this, SLOT(onViewChanged()));

	connect(ui.horizontalSliderOpacity, SIGNAL(valueChanged(int)), this, SLOT(onOpacityChanged(int)));

	ui.radioButtonZones->setChecked(true);

	hide();	
}

void ControlWidget::onViewChanged() {
	mainWin->glWidget->updateGL();
}

void ControlWidget::onOpacityChanged(int value) {
	mainWin->glWidget->updateGL();
}
