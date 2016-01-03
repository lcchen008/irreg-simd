#include "conf.h"
#include "../../tools/adjacency_csr_loader.h"
#include "../../tools/aligned_csr.h"
#include "../../tools/csr_loader.h"
#include "../../tools/ds.h"
#include "../../tools/load_tile_from_file.h"
#include "../../tools/tiling.h"
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
void DoMoldyn(Csr<float>& nnzs,
              const ThreeDSoa<float>* coordinates,
              ThreeDSoa<float>* forces,
              float sideHalf,
              float cutoffSquare,
              int n_threads) {
  vfloat xi, yi, zi;
  vfloat xj, yj, zj;
  vfloat xx, yy, zz;

  vfloat rd;
  vfloat zero(0);
  vfloat side(sideHalf*2);
  vfloat min_sideHalf(-sideHalf);
  vfloat plus_sideHalf(sideHalf);
  vfloat vcutoffSquare(cutoffSquare);

  MASK::vfloat rrd; 
  MASK::vfloat rrd2;
  MASK::vfloat rrd3;
  MASK::vfloat rrd4;
  MASK::vfloat rrd6;
  MASK::vfloat rrd7;
  MASK::vfloat r148;

  MASK::vfloat iforce_tmpx;
  MASK::vfloat iforce_tmpy;
  MASK::vfloat iforce_tmpz;
  MASK::vfloat jforce_tmpx;
  MASK::vfloat jforce_tmpy;
  MASK::vfloat jforce_tmpz;

  MASK::vfloat forcex;
  MASK::vfloat forcey;
  MASK::vfloat forcez;

  double before = rtclock();
  for (int i = 0; i < nnzs.num_rows; ++i) {
    xi = coordinates->x[i];
    yi = coordinates->y[i];
    zi = coordinates->z[i];
    for (int j = nnzs.rows[i]; j < nnzs.rows[i+1]; j+=16) {
      xj.load(coordinates->x, *(vint*)(nnzs.cols+j), 4);
      yj.load(coordinates->y, *(vint*)(nnzs.cols+j), 4);
      zj.load(coordinates->z, *(vint*)(nnzs.cols+j), 4);

      // Each lane does one nnz (interaction).
      xx = xi - xj;
      yy = yi - yj;
      zz = zi - zj;

      Mask::set_mask(xx < min_sideHalf, xx);
      xx.mask() += side.mask();
      Mask::set_mask(yy < min_sideHalf, yy);
      yy.mask() += side.mask();
      Mask::set_mask(zz < min_sideHalf, yy);
      zz.mask() += side.mask();

      Mask::set_mask(xx > min_sideHalf, xx);
      xx.mask() -= side.mask();
      Mask::set_mask(yy > min_sideHalf, yy);
      yy.mask() -= side.mask();
      Mask::set_mask(zz > min_sideHalf, yy);
      zz.mask() -= side.mask();
      rd = xx * xx + yy * yy + zz * zz;

      Mask::set_mask(rd < vcutoffSquare, zero);

      rrd = (MASK::vfloat)1.0/rd.mask(); 
      rrd2 = rrd*rrd;
      rrd3 = rrd2*rrd;
      rrd4 = rrd2*rrd2;
      rrd6 = rrd2*rrd4;
      rrd7 = rrd6*rrd;
      r148 = rrd7 - rrd4 * 0.5;

      forcex = xx.mask()*r148;
      forcey = yy.mask()*r148;
      forcez = zz.mask()*r148;

      // gather forces in col direction.
      jforce_tmpx.load(forces->x, (*(vint*)(nnzs.cols+j)).mask(), 4);
      jforce_tmpy.load(forces->y, (*(vint*)(nnzs.cols+j)).mask(), 4);
      jforce_tmpz.load(forces->z, (*(vint*)(nnzs.cols+j)).mask(), 4);

      jforce_tmpx = jforce_tmpx - forcex;
      jforce_tmpy = jforce_tmpy - forcey;
      jforce_tmpz = jforce_tmpz - forcez;

      // Store in col direction.
      jforce_tmpx.store((void*)(forces->x), *(vint*)(nnzs.cols+j), 4);
      jforce_tmpy.store((void*)(forces->y), *(vint*)(nnzs.cols+j), 4);
      jforce_tmpz.store((void*)(forces->z), *(vint*)(nnzs.cols+j), 4);

      // Store in row direction.
      forces->x[i] += _mm512_mask_reduce_add_ps(Mask::m, forcex);
      forces->y[i] += _mm512_mask_reduce_add_ps(Mask::m, forcey);
      forces->z[i] += _mm512_mask_reduce_add_ps(Mask::m, forcez);
    }
  }
  double after = rtclock();
  cout << RED << "[****Result****] ========> *SIMD Naive* Moldyn time: " << after - before << " secs." << RESET << endl;
}

int main(int argc, char** argv) {
  double begin = rtclock();
  cout << "NNZ file: " << string(argv[1]) << endl;
  cout << "XYZ file: " << string(argv[2]) << endl;

  // Load NNZs. 
  Csr<float>* nnzs = AdjacencyCsrLoader<float>::Load(string(string(argv[1])));
  double after_nnz = rtclock();
  cout << "NNZ load done, at time of " << after_nnz - begin << endl;

  // Load coordinates.
  ThreeDSoa<float>* coos = LoadCoo(string(argv[2])); 
  ThreeDSoa<float>* forces = new ThreeDSoa<float>(coos->num_nodes);
  double after_coo = rtclock();
  cout << "Coordinates load done, at time of " << after_coo - begin << endl;

  float side = POW( ((float)(coos->num_nodes)/DENSITY), 0.3333333);
  float sideHalf = 0.5 * side;
  float cutoffSquare = (cutoffRadius * cutoffRadius);

  DoMoldyn(*nnzs, coos, forces, sideHalf, cutoffSquare, 1);

  cout << "Done." << endl << endl;

  delete coos;
  delete forces;
  delete nnzs;
  return 0;
}
