// This util reorders the adjacency list based mesh data
// according to the ordering file originated from the
// metis partitioning result.
// The output is used for verifying the pure effect of reordering.

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

void convert(string mesh_file,
             string ordering_file) {
  vector<int> reorder_map;

  // Load ordering file.
  ifstream ordering_input(ordering_file.c_str());
  int order;
  while (ordering_input >> order) {
    reorder_map.push_back(order);
  } 
  ordering_input.close();

  // Load mesh data file.
  ifstream matlab_input(mesh_file.c_str()); 
  string line;

  int row;
  int col;
  int num_nodes; 
  int nnz;
  getline(matlab_input, line);
  istringstream is(line);
  is >> num_nodes;
  is >> nnz;

  string output_filename = mesh_file + ".reordered";
  ofstream output(output_filename.c_str());
  output << num_nodes << " " << nnz << endl;

  vector<vector<int> > edges(num_nodes);
  int line_no = 0;
  int total = 0;

  while (getline(matlab_input, line)) {
    istringstream ss(line);
    int edge;
    int new_line = reorder_map[line_no];
    while (ss >> edge) {
      edges[new_line].push_back(reorder_map[edge-1]+1);
    }
    sort(edges[new_line].begin(), edges[new_line].end());
    ++line_no;
  }
  matlab_input.close();

  for (int i = 0; i < edges.size(); ++i) {
    ostringstream os;
    for (int j = 0; j < edges[i].size(); ++j) {
      os << edges[i][j] << " ";
      ++total;
    }
    output << os.str() << endl;
  }

  cout << "Total edges: " << total << endl;

  output.close();
}

int main(int argc, char** argv) {
  convert(string(argv[1]), string(argv[2]));
}
