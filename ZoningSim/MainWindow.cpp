#include "MainWindow.h"
#include <QFileDialog>
#include "ParameterSettingWidget.h"

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags) {
	ui.setupUi(this);

	// setup the docking widgets
	controlWidget = new ControlWidget(this);

	connect(ui.actionLoadRoads, SIGNAL(triggered()), this, SLOT(onLoadRoads()));
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.actionParameters, SIGNAL(triggered()), this, SLOT(onParameters()));

	// setup the GL widget
	glWidget = new GLWidget3D(this);
	setCentralWidget(glWidget);

	controlWidget->show();
	addDockWidget(Qt::LeftDockWidgetArea, controlWidget);
}

void MainWindow::onLoadRoads() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Street Map file..."), "", tr("StreetMap Files (*.gsm)"));
	if (filename.isEmpty()) return;

	glWidget->loadRoads(filename);
	glWidget->updateGL();
}

void MainWindow::onParameters() {
	ParameterSettingWidget dlg(this, glWidget->weights);
	if (dlg.exec() != QDialog::Accepted) {
		return;
	}
}
