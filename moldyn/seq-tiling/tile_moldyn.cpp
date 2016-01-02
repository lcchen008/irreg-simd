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
void DoMoldyn(const PaddedNnz<ValueType>& nnzs,
              const ThreeDSoa<float>* coordinates,
              ThreeDSoa<float>* forces,
              float sideHalf,
              float cutoffSquare,
              int n_threads) {
  double before = rtclock();
  int i, j;
  float xx, yy, zz;
  float rd, rrd, rrd2, rrd3, rrd4, rrd6, rrd7, r148;
  float forcex, forcey, forcez;
  float side = sideHalf * 2;
  int count = 0;
  for (int ii = 0; ii < nnzs.nnz; ++ii) {
    int i = nnzs.rows[ii];  
    int j = nnzs.cols[ii];  
    
    xx = coordinates->x[i] - coordinates->x[j];
    yy = coordinates->y[i] - coordinates->y[j];
    zz = coordinates->z[i] - coordinates->z[j];
    if (xx < -sideHalf) xx += side;
    if (yy < -sideHalf) yy += side;
    if (zz < -sideHalf) zz += side;
    if (xx > sideHalf) xx -= side;
    if (yy > sideHalf) yy -= side;
    if (zz > sideHalf) zz -= side;
    rd = (xx*xx + yy*yy + zz*zz);
    if ( rd < cutoffSquare ) {
      ++count;
      rrd   = 1.0/rd;
      rrd2  = rrd*rrd ;
      rrd3  = rrd2*rrd ;
      rrd4  = rrd2*rrd2 ;
      rrd6  = rrd2*rrd4;
      rrd7  = rrd6*rrd ;
      r148  = rrd7 - 0.5 * rrd4 ;

      forcex = xx*r148;
      forcey = yy*r148;
      forcez = zz*r148;

      forces->x[i] += forcex ;
      forces->y[i] += forcey ;
      forces->z[i] += forcez ;

      forces->x[j] -= forcex ;
      forces->y[j] -= forcey ;
      forces->z[j] -= forcez ;
    }
  }
  double after = rtclock();
  cout << RED << "[****Result****] ========> *Serial Tiling* Moldyn time: " << after - before << " secs." << RESET << endl;
  cout << "Count is: " << count << endl;
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

	// Load coordinates.
	ThreeDSoa<float>* coos = LoadCoo(string(argv[2])); 
	ThreeDSoa<float>* forces = new ThreeDSoa<float>(coos->num_nodes);
  double after_coo = rtclock();
	cout << "Coordinates load done, at time of " << after_coo - begin << endl;

  float side = POW( ((float)(coos->num_nodes)/DENSITY), 0.3333333);
	float sideHalf = 0.5 * side;
	float cutoffSquare = (cutoffRadius * cutoffRadius);

  cout << "side: " << side << endl;
  cout << "sideHalf: " << sideHalf << endl;
  cout << "cutoffSquare: " << cutoffSquare << endl;

  DoMoldyn(*nnzs, coos, forces, sideHalf, cutoffSquare, 1);

  cout << "Done." << endl << endl;

	delete coos;
	delete forces;
  delete nnzs;
	return 0;
}
