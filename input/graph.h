// A skeloton for CSR graph format.

#ifndef GRAP_H_
#define GRAP_H_

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;

template <class VertexValue, class EdgeValue>
class graph {
	public:
		size_t num_vertices;
		size_t num_edges;

		// Array vertices store the starting position
    // of the edge going from each vertex. vertices
    // should end with num_edges while generating
    // CSR graph.
		vector<int> edges;	
		vector<long long> vertices;

    // An array for locating each vertex. It tells
    // which process a vertex belongs to.
    vector<int> process_map;

    vector<int> in_degree;

		VertexValue *vertex_value;
		EdgeValue *edge_value;

		graph(size_t num_vertices);
		~graph();

};

template <class VertexValue, class EdgeValue>
graph<VertexValue, EdgeValue>::graph(size_t num_vertices) {
	this->num_vertices = num_vertices;
	this->vertices.resize(num_vertices, -1);
  this->in_degree.resize(num_vertices, 0);
  edge_value = 0;
  vertex_value = 0;
}

template <class VertexValue, class EdgeValue>
graph<VertexValue, EdgeValue>::~graph() {
  if (vertex_value) {
    delete [] vertex_value;
  }
  if (edge_value) {
    delete[] edge_value;
  }
}


#endif
