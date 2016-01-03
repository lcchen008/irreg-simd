#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/time.h>
#include <tuple>
#include <vector>
using namespace std;

#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define RESET   "\033[0m"

double comp_clock()
{
  struct timezone Tzp;
  struct timeval Tp;
  int stat;
  stat = gettimeofday (&Tp, &Tzp);
  if (stat != 0) printf("Error return from gettimeofday: %d",stat);
  return(Tp.tv_sec + Tp.tv_usec*1.0e-6);
};

void Compute(string filename) {
  cout << "Reading input..." << endl;
  ifstream input(filename.c_str());  
  string line;
  getline(input, line);
  istringstream ss(line);
  int num_nodes, num_edges;
  ss >> num_nodes;
  ss >> num_edges;

  vector<pair<int, int> > edges;
  int line_no = 0;
  while (getline(input, line)) {
    istringstream ss(line); 
    int edge;
    while (ss >> edge) {
      --edge;
      edges.emplace_back(line_no, edge);
    }
    ++line_no; 
  }

  vector<float> edge_data(num_edges, 0.99999);
  vector<tuple<float, float, float> > 
      velocity(num_nodes, make_tuple<float, float, float>(0.355, 0, 0));
  vector<tuple<float, float, float> > 
      local_buf(num_nodes, make_tuple<float, float, float>(0, 0, 0));

  cout << "Read done." << endl;

  float a0,a1,a2;
  float r0,r1,r2;
  int n0, n1;

  double before = comp_clock();

  for (int j = 0; j < 1; ++j) {
    for (int i = 0; i < num_edges; ++i) {
      n0=get<0>(edges[i]);
      n1=get<1>(edges[i]);

      a0=(edge_data[i]*get<0>(velocity[n0]) +
          edge_data[i]*get<1>(velocity[n0]) +
          edge_data[i]*get<2>(velocity[n0]))/3.0;
      a1=(edge_data[i]*get<0>(velocity[n1]) +
          edge_data[i]*get<1>(velocity[n1]) +
          edge_data[i]*get<2>(velocity[n1]))/3.0;

      r0=a0*get<0>(velocity[n0])+a1*get<0>(velocity[n1])+edge_data[i];
      r1=a0*get<1>(velocity[n0])+a1*get<1>(velocity[n1])+edge_data[i];
      r2=a0*get<2>(velocity[n0])+a1*get<2>(velocity[n1])+edge_data[i];

      get<0>(local_buf[n0]) = get<0>(local_buf[n0])+r0;
      get<1>(local_buf[n0]) = get<1>(local_buf[n0])+r1;
      get<2>(local_buf[n0]) = get<2>(local_buf[n0])+r2;

      get<0>(local_buf[n1]) = get<0>(local_buf[n1])-r0;
      get<1>(local_buf[n1]) = get<1>(local_buf[n1])-r1;
      get<2>(local_buf[n1]) = get<2>(local_buf[n1])-r2;
    }
  }

  double after = comp_clock();
  cout << RED << "[****Result****] ========> *Serial* Euler time: " << after - before << " secs." << RESET << endl;
}

int main(int argc, char** argv)
{
  cout << "Seq using input: " << argv[1] << endl;
  Compute(string(argv[1]));   
  cout << endl;
  return 0;
}
