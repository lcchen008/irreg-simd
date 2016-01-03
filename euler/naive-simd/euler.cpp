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
void DoEuler(Csr<float>& nnzs,
             const ThreeDSoa<float>* velocities,
             ThreeDSoa<float>* forces) {
  vfloat xi, yi, zi;
  vfloat xj, yj, zj;
  vfloat a0, a1;
  vfloat r0, r1, r2;

  vfloat iforce_tmpx;
  vfloat iforce_tmpy;
  vfloat iforce_tmpz;
  vfloat jforce_tmpx;
  vfloat jforce_tmpy;
  vfloat jforce_tmpz;

  double before = rtclock();
  for (int i = 0; i < nnzs.num_rows; ++i) {
    xi = velocities->x[i];
    yi = velocities->y[i];
    zi = velocities->z[i];
    for (int j = nnzs.rows[i]; j < nnzs.rows[i+1]; j+=16) {
      vfloat edge_data(*(vfloat*)(nnzs.vals+j));

      xj.load(velocities->x, *(vint*)(nnzs.cols+j), 4);
      yj.load(velocities->y, *(vint*)(nnzs.cols+j), 4);
      zj.load(velocities->z, *(vint*)(nnzs.cols+j), 4);

      // Each lane does one nnz (interaction).
      a0 = (edge_data * xi + 
            edge_data * yi +
            edge_data * zi)/3.0;
      a1 = (edge_data * xj +
            edge_data * yj +
            edge_data * zj)/3.0;

      r0 = a0 * xi + a1 * xj + edge_data;
      r1 = a0 * yi + a1 * yj + edge_data;
      r2 = a0 * zi + a1 * zj + edge_data;

      // gather forces in col direction.
      jforce_tmpx.load(forces->x, (*(vint*)(nnzs.cols+j)), 4);
      jforce_tmpy.load(forces->y, (*(vint*)(nnzs.cols+j)), 4);
      jforce_tmpz.load(forces->z, (*(vint*)(nnzs.cols+j)), 4);

      jforce_tmpx = jforce_tmpx - r0;
      jforce_tmpy = jforce_tmpy - r1;
      jforce_tmpz = jforce_tmpz - r2;

      // Store in col direction.
      jforce_tmpx.store((void*)(forces->x), *(vint*)(nnzs.cols+j), 4);
      jforce_tmpy.store((void*)(forces->y), *(vint*)(nnzs.cols+j), 4);
      jforce_tmpz.store((void*)(forces->z), *(vint*)(nnzs.cols+j), 4);

      // Store in row direction.
      forces->x[i] += _mm512_reduce_add_ps(r0);
      forces->y[i] += _mm512_reduce_add_ps(r1);
      forces->z[i] += _mm512_reduce_add_ps(r2);
    }
  }
  double after = rtclock();
  cout << RED << "[****Result****] ========> *SIMD Naive* Euler time: " << after - before << " secs." << RESET << endl;
}

int main(int argc, char** argv) {
  double begin = rtclock();
  cout << "NNZ file: " << string(argv[1]) << endl;
  cout << "XYZ file: " << string(argv[2]) << endl;

  // Load NNZs. 
  Csr<float>* nnzs = AdjacencyCsrLoader<float>::Load(string(string(argv[1])));
  // Allocate vals.
  nnzs->vals = (float*)_mm_malloc(sizeof(float)*nnzs->nnz, 64);
  for (int i = 0; i < nnzs->nnz; ++i) {
    nnzs->vals[i] = 1;
  }
  double after_nnz = rtclock();
  cout << "NNZ load done, at time of " << after_nnz - begin << endl;

  // Load velocities.
  ThreeDSoa<float>* velocities = LoadCoo(string(argv[2])); 
  // Allocate local buf (forces).
  ThreeDSoa<float>* forces = new ThreeDSoa<float>(velocities->num_nodes);
  double after_velocities = rtclock();
  cout << "Velocities load done, at time of " << after_velocities - begin << endl;

  DoEuler(*nnzs, velocities, forces);

  cout << "Done." << endl;

  delete velocities;
  delete forces;
  delete nnzs;
  return 0;
}
