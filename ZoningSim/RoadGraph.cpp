#include "RoadGraph.h"
#include <QGLWidget>
#include "Util.h"

RoadGraph::RoadGraph() {
	modified = false;
}

RoadGraph::~RoadGraph() {
}

void RoadGraph::clear() {
	graph.clear();
	modified = true;
}

