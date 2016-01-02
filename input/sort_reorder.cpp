// This util translates an undirected graph to a directed graph.
// The purpose of this util is to generate input for metis partitioning.
// The input file is 1-based.
// We also generate a column-major dense format for matlab visualization.
//
// *REQUIRED INPUT FORMAT* : first line must begin with an integer indicating
// the number of vertices. 
// Each of the remaining lines (exactly num_vertices lines) contains the list of
// edges from the vertex. If a node has no edges, then it reserves an empty
// line. 

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <stdlib.h>

#include "graph.h"

using namespace std;

struct Compare {
  bool operator()(const pair<float, int>& l, const pair<float, int>& r) const {
    return l.first < r.first;
  }
};

void ReadGraph(string ifilename, string ofilename) {
	// Construct graph.
	ifstream input(ifilename.c_str());		
	string line;
	int edge;
	int line_no = 0;

	cout << "*************************************" << endl;
	cout << "Begin reading graph input." << endl;

  getline(input, line);
  istringstream ss(line);
  int num_vertices;
  int num_edges;
  ss >> num_vertices;
  ss >> num_edges;
  cout << "Vertices: " << num_vertices << endl;
  cout << "Edges: " << num_edges << endl;

  vector<int> positions(num_vertices, 0);
  vector<int> frequency(num_vertices, 1);
  vector<pair<float, int> > to_sort(num_vertices);

  int count = 0;
	while(getline(input, line)) {
		istringstream ss(line);
		while(ss >> edge) {
      edge--;
      positions[line_no] += count;
      positions[edge] += count;
      ++frequency[line_no];
      ++frequency[edge];
      ++count;
		}
		line_no++;
	}

  for (int i = 0; i < num_vertices; ++i) {
    to_sort[i].first = (float)positions[i]/frequency[i];
    to_sort[i].second = i;
  }

  sort(to_sort.begin(), to_sort.end(), Compare());

  vector<int> position_map(num_vertices);

  for (int i = 0; i < to_sort.size(); ++i) {
    position_map[to_sort[i].second] = i;
  }

  // Write the output to file.
  cout << "Generating ordering input: " << ofilename << endl;
  ofstream output(ofilename.c_str());

  for (int i = 0; i < num_vertices; ++i) {
    output << position_map[i] << endl;
  }

  output.close();
}

int main(int argc, char** argv) {
  string input_file = string(argv[1]);
  string output_file = input_file + ".ordering";
  cout << "input file: " << input_file << endl;
  cout << "output file: " << output_file << endl;
  ReadGraph(input_file, output_file);
}
