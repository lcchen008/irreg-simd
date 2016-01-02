#include "conf.h"
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

void OutputTilesToFile(vector<Coo<int> >* tiles, string filename, int num_rows, int num_cols, int nnz) {
  ofstream output(filename.c_str());
  output << num_rows << " " << num_cols << " " << nnz << endl; 
  for (int i = 0; i < tiles->size(); ++i) {
    for (int j = 0; j < (*tiles)[i].actual_nnz; ++j) {
      output << (*tiles)[i].rows[j] << " ";
      output << (*tiles)[i].cols[j] << " ";
      output << 1 << endl;
    }
  } 
  output.close();
}

inline void Euler(const PaddedNnz<float>& nnzs,
                  const int begin,
                  const int end,
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

  // Process 16 eles each time.
	for (int i = begin; i < end; i += 16) {
      vfloat edge_data(*(vfloat*)(nnzs.vals+i));
	
      // Load velocities
		  xi.load(velocities->x, *(vint*)(nnzs.rows+i), 4);
		  yi.load(velocities->y, *(vint*)(nnzs.rows+i), 4);
		  zi.load(velocities->z, *(vint*)(nnzs.rows+i), 4);
	
		  xj.load(velocities->x, *(vint*)(nnzs.cols+i), 4);
		  yj.load(velocities->y, *(vint*)(nnzs.cols+i), 4);
		  zj.load(velocities->z, *(vint*)(nnzs.cols+i), 4);

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

      // Reduce
      // row direction.
      // gather forces in row direction.
	  	iforce_tmpx.load(forces->x, *(vint*)(nnzs.rows+i), 4);
	  	iforce_tmpy.load(forces->y, *(vint*)(nnzs.rows+i), 4);
	  	iforce_tmpz.load(forces->z, *(vint*)(nnzs.rows+i), 4);

      iforce_tmpx = iforce_tmpx + r0;
      iforce_tmpy = iforce_tmpy + r1;
      iforce_tmpz = iforce_tmpz + r2;

	  	// Store in row direction.
	  	iforce_tmpx.store((void*)(forces->x), *(vint*)(nnzs.rows+i), 4);
	  	iforce_tmpy.store((void*)(forces->y), *(vint*)(nnzs.rows+i), 4);
	  	iforce_tmpz.store((void*)(forces->z), *(vint*)(nnzs.rows+i), 4);

      // gather forces in col direction.
  	  jforce_tmpx.load(forces->x, *(vint*)(nnzs.cols+i), 4);
	  	jforce_tmpy.load(forces->y, *(vint*)(nnzs.cols+i), 4);
	  	jforce_tmpz.load(forces->z, *(vint*)(nnzs.cols+i), 4);

      jforce_tmpx = jforce_tmpx - r0;
      jforce_tmpy = jforce_tmpy - r1;
      jforce_tmpz = jforce_tmpz - r2;
	  	
	  	// Store in col direction.
	  	jforce_tmpx.store((void*)(forces->x), *(vint*)(nnzs.cols+i), 4);
	  	jforce_tmpy.store((void*)(forces->y), *(vint*)(nnzs.cols+i), 4);
	  	jforce_tmpz.store((void*)(forces->z), *(vint*)(nnzs.cols+i), 4);
	}
}

// Do in multi-thread fashion.
template <class ValueType>
void DoEuler(const PaddedNnz<ValueType>& nnzs,
             const vector<vector<int> >& offsets,
             const ThreeDSoa<float>* velocities,
             ThreeDSoa<float>* forces,
             int n_threads) {
  double before = rtclock();
  for (int i = 0; i < offsets.size(); ++i) {
    #pragma omp parallel for num_threads(n_threads)
    for (int j = 0; j < offsets[i].size() - 1; ++j) {
     // Euler(nnzs, offsets[i][j], offsets[i][j+1], velocities, forces);
      Euler(nnzs,
            offsets[i][j],
            offsets[i][j+1],
            velocities,
            forces);
    }
  }
  double after = rtclock();
  cout << RED << "[****Result****] ========> *SIMD Tiling (Our)* " << " on "
       << n_threads << " threads time: " << after - before << " secs." << RESET << endl;
}

int main(int argc, char** argv) {
  double begin = rtclock();
	cout << "NNZ file: " << string(argv[1]) << endl;
	cout << "Offset file: " << string(argv[2]) << endl;
	cout << "XYZ file: " << string(argv[3]) << endl;

  // Load NNZs and offsets. 
  vector<vector<int> >* offsets; 
  PaddedNnz<float>* nnzs;
  LoadTileFromFile(string(argv[2]), string(argv[1]), offsets, nnzs);
  double after_nnz = rtclock();
	cout << "NNZ and offsets load done, at time of " << after_nnz - begin << endl;

	// Load velocities.
	ThreeDSoa<float>* velocities = LoadCoo(string(argv[3])); 
  // Allocate local buf (forces).
	ThreeDSoa<float>* forces = new ThreeDSoa<float>(velocities->num_nodes);
  double after_velocities = rtclock();
	cout << "Velocities load done, at time of " << after_velocities << endl;

  cout << "Max parallelism: " << ComputeMaxParallelism(*offsets) << endl;
  cout << "Min parallelism: " << ComputeMinParallelism(*offsets) << endl;
  cout << "Average parallelism: " << ComputeAverageParallelism(*offsets) << endl;

  cout << "Begin..." << endl;

  DoEuler(*nnzs, *offsets, velocities, forces, 1);
  DoEuler(*nnzs, *offsets, velocities, forces, 2);
  DoEuler(*nnzs, *offsets, velocities, forces, 4);
  DoEuler(*nnzs, *offsets, velocities, forces, 8);
  DoEuler(*nnzs, *offsets, velocities, forces, 16);
  DoEuler(*nnzs, *offsets, velocities, forces, 32);
  DoEuler(*nnzs, *offsets, velocities, forces, 40);
  DoEuler(*nnzs, *offsets, velocities, forces, 50);
  DoEuler(*nnzs, *offsets, velocities, forces, 61);
  DoEuler(*nnzs, *offsets, velocities, forces, 80);
  DoEuler(*nnzs, *offsets, velocities, forces, 122);
  DoEuler(*nnzs, *offsets, velocities, forces, 150);
  DoEuler(*nnzs, *offsets, velocities, forces, 180);
  DoEuler(*nnzs, *offsets, velocities, forces, 200);
  DoEuler(*nnzs, *offsets, velocities, forces, 244);

  cout << "Done." << endl;

	delete velocities;
	delete forces;
  delete offsets;
  delete nnzs;
	return 0;
}
