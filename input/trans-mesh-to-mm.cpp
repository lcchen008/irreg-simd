// We generate a column-major dense format for matlab visualization.
//
// *REQUIRED INPUT FORMAT* : first line must begin with an integer indicating
// the number of vertices. 
// Each of the remaining lines (exactly num_vertices lines) contains the list of
// edges from the vertex. If a node has no edges, then it reserves an empty
// line. 

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <stdlib.h>

#include "graph.h"

using namespace std;

void ReadGraph(string ifilename, string mfilename) {
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
  vector<set<int> > metis_data(num_vertices); // data is 0 based.

  // There are num_vertices columns. Column-major
  // dense data is used for matlab visualiation.
  // Matalab data is also 0-based.
  vector<set<int> > matlab_data(num_vertices); 

	while(getline(input, line)) {
		istringstream ss(line);
		while(ss >> edge) {
      edge--;
      
      if (line_no >= num_vertices) {
        cout << "Line no out of bound: " << line_no << endl;
      }
      if (edge >= num_vertices || edge < 0) {
        cout << "Edge out of bound: " << edge << endl;
      }

      matlab_data[edge].insert(line_no);

      // Avoid diagonal elements in metis data.
      if (line_no == edge) {
        continue;
      }

      // Two directions for metis.
      metis_data[line_no].insert(edge);
      metis_data[edge].insert(line_no);
		}
		line_no++;
	}

  int undirected_num_edges = 0;
  for (auto i = metis_data.begin(); i != metis_data.end(); ++i) {
    undirected_num_edges += i->size();
  }

  // Write the output to file.
  
  // cout << "Generating metis input: " << ofilename << endl;
  // ofstream output(ofilename.c_str());
  // output << " " << num_vertices << " " << undirected_num_edges/2 << endl;

  // for (int i = 0; i < num_vertices; ++i) {
  //   ostringstream line;
  //   for (auto j = metis_data[i].begin(); j != metis_data[i].end(); ++j) {
  //     line << " ";
  //     line << *j + 1;  // Write as 1-based edges. 
  //   }
  //   output << line.str() << endl;
  // }

  cout << "Generating matlab input: " << mfilename << endl;
  // Generate matlab visualization file.
  ofstream matlab_output(mfilename.c_str());
  matlab_output << num_vertices << " " << num_vertices << " " << num_edges << endl;
  for (int i = 0; i < matlab_data.size(); ++i) {
    ostringstream line;
    for (auto j = matlab_data[i].begin(); j != matlab_data[i].end(); ++j) {
      line << *j + 1 << " " << i + 1 << " 1" << endl;
    }
    matlab_output << line.str();
  }
  matlab_output.close();
}

int main(int argc, char** argv) {
  string input_file = string(argv[1]);
  string matlab_file = input_file + ".matlab";
  cout << "input file: " << input_file << endl;
  cout << "matlab file: " << matlab_file << endl;
  ReadGraph(input_file, matlab_file);
  cout << "Transfer to matrix-market format done." << endl;
}
