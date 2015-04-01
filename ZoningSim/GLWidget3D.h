#pragma once

#include <QGLWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include "Camera.h"
#include <QVector3D>
#include <vector>
#include "Zoning.h"
#include "RoadGraph.h"

class MainWindow;

class GLWidget3D : public QGLWidget {
public:
	MainWindow* mainWin;
	Camera camera;
	QPoint lastPos;
	Zoning* zoning;
	RoadGraph roads;
	QMap<QString, float> weights;

public:
	GLWidget3D(MainWindow *parent);
	void drawScene();
	void loadRoads(const QString& filename);

protected:
	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();    
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);

};

