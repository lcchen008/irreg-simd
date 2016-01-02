// 0-based.
#include <fstream>
#include <iostream>
#include <vector>
using namespace std;

void Convert(int num_parts, string input_file_name, string output_file_name) {

  // Read the partitioning result.
  ifstream input(input_file_name); 
  vector<vector<int> > parts = vector<vector<int> >(num_parts);
  int part;
  int count = 0;
  while (input >> part) {
    parts[part].push_back(count);
    ++count;
  }
  input.close();

  // Output the permuation result.
  ofstream output(output_file_name);
  // The permutated position for every id.
  vector<int> permutation(count);
  int index = 0;
  for (int i = 0; i < parts.size(); ++i) {
    for (int j = 0; j < parts[i].size(); ++j) {
      permutation[parts[i][j]] = index;
      ++index;
    }
  }
  for (int i = 0; i < permutation.size(); ++i) {
    output << permutation[i] << endl;
  }
  output.close();
}

int main(int argc, char** argv) {
  Convert(atoi(argv[1]), string(argv[2]), string(argv[2]) + ".ordering"); 
}
