// This util reorders the coordinates of the nodes
// according to the ordering result from Metis.

#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple>
#include <vector>
using namespace std;

void convert(string coordinate_file,
             string ordering_file) {
  // Load ordering file.
  ifstream order_input(ordering_file.c_str());
  vector<int> orders;
  int pos;
  while (order_input >> pos) {
    orders.push_back(pos);
  }
  order_input.close();

  cout << "Order file load done." << endl;

  // Load coordinates.
  ifstream coo_input(coordinate_file.c_str());
  cout << "Size of orders: " << orders.size() << endl;
  vector<tuple<float, float, float> > coos(orders.size());
  string line;
  int count = 0;
  while (getline(coo_input, line)) {
    istringstream ss(line);
    ss >> get<0>(coos[orders[count]]);
    ss >> get<1>(coos[orders[count]]);
    ss >> get<2>(coos[orders[count]]);
    ++count; 
  }
  coo_input.close();

  cout << "Coordinate file load done." << endl;

  // Write the reordered coordinates.
  string output_file = coordinate_file + ".reordered";
  ofstream output(output_file.c_str());

  for (int i = 0; i < coos.size(); ++i) {
    output << get<0>(coos[i])
        << " " << get<1>(coos[i]) 
        << " " << get<2>(coos[i]) << endl;
  }

  output.close();
}

int main(int argc, char** argv) {
  convert(string(argv[1]), string(argv[2]));
}
