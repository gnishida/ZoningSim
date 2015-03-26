#pragma once

#include "common.h"
#include <stdio.h>
#include "RoadVertex.h"
#include "RoadEdge.h"

using namespace boost;

typedef adjacency_list<vecS, vecS, undirectedS, RoadVertexPtr, RoadEdgePtr> BGLGraph;
typedef graph_traits<BGLGraph>::vertex_descriptor RoadVertexDesc;
typedef graph_traits<BGLGraph>::edge_descriptor RoadEdgeDesc;
typedef graph_traits<BGLGraph>::vertex_iterator RoadVertexIter;
typedef graph_traits<BGLGraph>::edge_iterator RoadEdgeIter;
typedef graph_traits<BGLGraph>::out_edge_iterator RoadOutEdgeIter;
typedef graph_traits<BGLGraph>::in_edge_iterator RoadInEdgeIter;


typedef std::vector<RoadEdgeDesc> RoadEdgeDescs;
typedef std::vector<RoadVertexDesc> RoadVertexDescs;

class RoadGraph {
public:
	bool modified;
	BGLGraph graph;

public:
	RoadGraph();
	~RoadGraph();

	void setModified() { modified = true; }

	void clear();
};

typedef boost::shared_ptr<RoadGraph> RoadGraphPtr;
