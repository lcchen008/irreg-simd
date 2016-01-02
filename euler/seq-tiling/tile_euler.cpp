#include "conf.h"
// #include "../../tools/csr.h"
#include "../../tools/csr_loader.h"
#include "../../tools/ds.h"
#include "../../tools/load_tile_from_file.h"
#include "../../tools/tiling.h"
//#include "tiled_csr.h"
#include "util.h"

#include <atomic>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <math.h>
#include <omp.h>
#include <sstream>
#include <thread>
#include <tuple>
#include <vector>

#include <x86intrin.h>
// #include "sse_lib/vtypes.h"
#include "SSE_API_Package/SSE_Template/sse_api.h"

using namespace std;

#define DENSITY 0.83134
#define cutoffRadius 3.0
#define POW(a,b) pow(a,b)
#define TOLERANCE 1.2

ThreeDSoa<float>* LoadCoo(string coo_file) {
  ifstream input(coo_file.c_str());
  vector<tuple<float, float, float> > coos;
  string line;
  float x, y, z;
  while (getline(input, line)) {
    istringstream iss(line); 
    iss >> x;
    iss >> y;
    iss >> z;
    coos.push_back(make_tuple(x, y, z)); 
  }
  input.close();
  ThreeDSoa<float>* soa = new ThreeDSoa<float>(coos.size());
  soa->SetXyz(coos);
  return soa;
}

// Do in multi-thread fashion.
template <class ValueType>
void DoEuler(const PaddedNnz<ValueType>& nnzs,
             const ThreeDSoa<float>* velocities,
             ThreeDSoa<float>* forces) {
  double before = rtclock();

  float a0,a1,a2;
  float r0,r1,r2;
  int n0, n1;

  for (int ii = 0; ii < nnzs.nnz; ++ii) {
    n0 = nnzs.rows[ii];
    n1 = nnzs.cols[ii];

    a0 = (nnzs.vals[ii] * velocities->x[n0] +
         nnzs.vals[ii] * velocities->y[n0] +
         nnzs.vals[ii] * velocities->z[n0])/3.0;
    a1 = (nnzs.vals[ii] * velocities->x[n1] +
         nnzs.vals[ii] * velocities->y[n1] +
         nnzs.vals[ii] * velocities->z[n1])/3.0;

    r0 = a0*velocities->x[n0] + a1*velocities->x[n1] + nnzs.vals[ii];
    r1 = a0*velocities->y[n0] + a1*velocities->y[n1] + nnzs.vals[ii];
    r2 = a0*velocities->z[n0] + a1*velocities->z[n1] + nnzs.vals[ii];

    forces->x[n0] += r0;
    forces->y[n0] += r1;
    forces->z[n0] += r2;

    forces->x[n1] -= r0;
    forces->y[n1] -= r1;
    forces->z[n1] -= r2;
  }

  double after = rtclock();
  cout << RED << "[****Result****] ========> *Serial Tiling* Euler time: " << after - before << " secs." << RESET << endl;
}

int main(int argc, char** argv) {
  double begin = rtclock();
	cout << "NNZ file: " << string(argv[1]) << endl;
  cout << "XYZ file: " << string(argv[2]) << endl;

  // Load NNZs. 
  PaddedNnz<int>* nnzs;
  LoadTileFromFile(string(argv[1]), nnzs);
  double after_nnz = rtclock();
	cout << "NNZ load done, at time of " << after_nnz - begin << endl;

  cout << "Total NNZ: " << nnzs->nnz << endl;

	// Load velocities.
	ThreeDSoa<float>* velocities = LoadCoo(string(argv[2])); 
	ThreeDSoa<float>* forces = new ThreeDSoa<float>(velocities->num_nodes);
  double after_coo = rtclock();
	cout << "Velocities load done, at time of " << after_coo - begin << endl;

  DoEuler(*nnzs, velocities, forces);

  cout << "Done." << endl;

	delete velocities;
	delete forces;
  delete nnzs;
	return 0;
}
