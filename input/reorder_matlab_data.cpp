// This util reorders the column-major matlab data
// according to the ordering file originated from the
// metis partitioning result.

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

void convert(string matlab_file,
             string ordering_file) {
  vector<int> reorder_map;

  // Load ordering file.
  ifstream ordering_input(ordering_file.c_str());
  int order;
  while (ordering_input >> order) {
    reorder_map.push_back(order);
  } 
  ordering_input.close();

  // Load matlab data file.
  ifstream matlab_input(matlab_file.c_str()); 
  string line;

  int row;
  int col;
  int num_nodes; 
  int nnz;
  getline(matlab_input, line);
  istringstream is(line);
  is >> num_nodes;
  is >> num_nodes; 
  is >> nnz;
  vector<vector<int> > reordered(num_nodes);
  while (getline(matlab_input, line)) {
    istringstream ss(line);
    ss >> row;
    ss >> col;
    // Store undirected graph.
    // if (col < row) { continue; }
    reordered[reorder_map[col-1]].push_back(reorder_map[row-1]);
  }
  matlab_input.close();

  // Sort the reordered vector, so that the row numbers are in increasing order.
  for (int i = 0; i < reordered.size(); ++i) {
    sort(reordered[i].begin(), reordered[i].end());
  }

  // Write the result.
  string output_filename = matlab_file + ".reordered";
  ofstream output(output_filename.c_str());
  int count = 0;
  output << num_nodes << " " << num_nodes << " " << nnz << endl;
  for (int i = 0; i < reordered.size(); ++i) {
    ostringstream line;
    for (int j = 0; j < reordered[i].size(); ++j) {
      //if (reordered[i][j] > i) { continue; }
      line << reordered[i][j] + 1 << " " << i + 1 << " 1" << endl; 
      ++count;
    }
    output << line.str();
  }

  cout << "Total written: " << count << endl;

  output.close();
}

int main(int argc, char** argv) {
  convert(string(argv[1]), string(argv[2]));
}
