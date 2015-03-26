#include <iostream>
#include "GLWidget3D.h"
#include "MainWindow.h"
#include <GL/GLU.h>
#include "GraphUtil.h"

GLWidget3D::GLWidget3D(MainWindow* mainWin) : QGLWidget(QGLFormat(QGL::SampleBuffers), (QWidget*)mainWin) {
	this->mainWin = mainWin;
	camera.dz = 1000;

	// 重みを適当にセットする
	QMap<QString, float> weights;
	weights["accessibility_highway"] = 0.01f;
	weights["accessibility_avenue"] = 0.005f;
	weights["accessibility_street"] = 0.001f;
	weights["pollution"] = 0.1f;
	weights["activity"] = 0.1f;

	zoning = new Zoning(1000, 10, weights);
	loadRoads("osm/lafayette.gsm");
}

/**
 * This event handler is called when the mouse press events occur.
 */
void GLWidget3D::mousePressEvent(QMouseEvent *e) {
	lastPos = e->pos();
}

/**
 * This event handler is called when the mouse release events occur.
 */
void GLWidget3D::mouseReleaseEvent(QMouseEvent *e) {

	updateGL();
}

/**
 * This event handler is called when the mouse move events occur.
 */
void GLWidget3D::mouseMoveEvent(QMouseEvent *e) {
	float dx = (float)(e->x() - lastPos.x());
	float dy = (float)(e->y() - lastPos.y());
	lastPos = e->pos();

	if (e->buttons() & Qt::LeftButton) {
		camera.changeXRotation(dy);
		camera.changeYRotation(dx);
	} else if (e->buttons() & Qt::RightButton) {
		camera.changeXYZTranslation(0, 0, -dy * camera.dz * 0.02f);
		if (camera.dz < -9000) camera.dz = -9000;
		if (camera.dz > 9000) camera.dz = 9000;
	} else if (e->buttons() & Qt::MidButton) {
		camera.changeXYZTranslation(-dx, dy, 0);
	}

	updateGL();
}

/**
 * This function is called once before the first call to paintGL() or resizeGL().
 */
void GLWidget3D::initializeGL() {
	glClearColor(0.443, 0.439, 0.458, 0.0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);

	static GLfloat lightPosition[4] = {0.0f, 0.0f, 100.0f, 0.0f};
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

/**
 * This function is called whenever the widget has been resized.
 */
void GLWidget3D::resizeGL(int width, int height) {
	height = height?height:1;

	glViewport( 0, 0, (GLint)width, (GLint)height );
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (GLfloat)width/(GLfloat)height, 0.1f, 10000);
	glMatrixMode(GL_MODELVIEW);
}

/**
 * This function is called whenever the widget needs to be painted.
 */
void GLWidget3D::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
   	camera.applyCamTransform();	

	drawScene();		
}

/**
 * Draw the scene.
 */
void GLWidget3D::drawScene() {
	// 道路を描画
	glNormal3f(0, 0, 1);
	glBegin(GL_LINES);
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
		if (roads.graph[*ei]->type == RoadEdge::TYPE_HIGHWAY) {
			glLineWidth(3);
			glColor4f(1.0, 0.7, 0, 1);
		} else if (roads.graph[*ei]->type == RoadEdge::TYPE_AVENUE) {
			glLineWidth(2);
			glColor4f(1.0, 1.0, 0, 1);
		} else if (roads.graph[*ei]->type == RoadEdge::TYPE_STREET) {
			glLineWidth(1);
			glColor4f(1.0, 1.0, 1.0, 1);
		}
		for (int i = 0; i < roads.graph[*ei]->polyline.size() - 1; ++i) {
			glVertex3f(roads.graph[*ei]->polyline[i].x(), roads.graph[*ei]->polyline[i].y(), 0.0f);
			glVertex3f(roads.graph[*ei]->polyline[i+1].x(), roads.graph[*ei]->polyline[i+1].y(), 0.0f);
		}
	}
	glEnd();

	// 各セルを色で塗る
	float opacity = (float)mainWin->controlWidget->ui.horizontalSliderOpacity->value() / 100.0f;
	float z = 1.0f;
	glNormal3f(0, 0, 1);
	glBegin(GL_QUADS);
	for (int r = 0; r < zoning->grid_size; ++r) {
		for (int c = 0; c < zoning->grid_size; ++c) {
			float x = c * zoning->cell_length - zoning->city_length * 0.5;
			float y = r * zoning->cell_length - zoning->city_length * 0.5;

			if (mainWin->controlWidget->ui.radioButtonZones->isChecked()) {
				if (zoning->zones(r, c) == Zoning::TYPE_RESIDENTIAL) {
					glColor4f(1.0f, 0.f, 0.0f, opacity);
				} else if (zoning->zones(r, c) == Zoning::TYPE_COMMERCIAL) {
					glColor4f(0.0f, 0.0f, 1.0f, opacity);
				} else if (zoning->zones(r, c) == Zoning::TYPE_INDUSTRIAL) {
					glColor4f(1.0f, 1.0f, 0.0f, opacity);
				} else if (zoning->zones(r, c) == Zoning::TYPE_PARK) {
					glColor4f(0.0f, 0.8f, 0.0f, opacity);
				}
			} else if (mainWin->controlWidget->ui.radioButtonAccessibility->isChecked()) {
				glColor4f(1.0f, 1 - zoning->accessibility(r, c), 1 - zoning->accessibility(r, c), opacity);
			} else if (mainWin->controlWidget->ui.radioButtonActivity->isChecked()) {
				glColor4f(1.0f, 1 - zoning->activity(r, c), 1 - zoning->activity(r, c), opacity);
			} else if (mainWin->controlWidget->ui.radioButtonPollution->isChecked()) {
				glColor4f(1.0f, 1 - zoning->pollution(r, c), 1 - zoning->pollution(r, c), opacity);
			} else if (mainWin->controlWidget->ui.radioButtonSlope->isChecked()) {
				glColor4f(1.0f, 1 - zoning->slope(r, c), 1 - zoning->slope(r, c), opacity);
			} else if (mainWin->controlWidget->ui.radioButtonLandValue->isChecked()) {
				glColor4f(1.0f, 1 - zoning->landValue(r, c), 1 - zoning->landValue(r, c), opacity);
			} else if (mainWin->controlWidget->ui.radioButtonPopulation->isChecked()) {
				glColor4f(1.0f, 1 - zoning->population(r, c) / Zoning::MAX_POPULATION, 1 - zoning->population(r, c) / Zoning::MAX_POPULATION, opacity);
			} else if (mainWin->controlWidget->ui.radioButtonCommercialJobs->isChecked()) {
				glColor4f(1.0f, 1 - zoning->commercialJobs(r, c) / Zoning::MAX_JOBS, 1 - zoning->commercialJobs(r, c) / Zoning::MAX_JOBS, opacity);
			} else if (mainWin->controlWidget->ui.radioButtonIndustrialJobs->isChecked()) {
				glColor4f(1.0f, 1 - zoning->industrialJobs(r, c) / Zoning::MAX_JOBS, 1 - zoning->industrialJobs(r, c) / Zoning::MAX_JOBS, opacity);
			}

			glVertex3f(x, y, z);
			glVertex3f(x + zoning->cell_length, y, z);
			glVertex3f(x + zoning->cell_length, y + zoning->cell_length, z);
			glVertex3f(x, y + zoning->cell_length, z);
		}
	}
	glEnd();
}


void GLWidget3D::loadRoads(const QString& filename) {
	GraphUtil::loadRoads(roads, filename);
	zoning->setRoads(roads);
}