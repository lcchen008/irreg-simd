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

// This SIMD kernel only computes the force for each tile.
// The force data will be used by the caller for further processing.
inline void DoATile(const Coo<int>& tile,
                    const ThreeDSoa<float>* node_data,
					ThreeDSoa<float>* forces,
                    float sideHalf, float cutoffSquare) {
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

void barrier(int num_threads) {
  static std::atomic<int> counter = 0;
  counter++;
  while (counter != num_threads);
}

inline void Moldyn(const PaddedNnz<int>& nnzs,
            int begin,
            int end,
            const ThreeDSoa<float>* coordinates,
            ThreeDSoa<float>* forces,
            float sideHalf,
            float cutoffSquare) {
	vfloat side(sideHalf*2);
  vfloat xi, yi, zi;
	vfloat xj, yj, zj;
  vfloat xx;
  vfloat yy;
  vfloat zz;
  vfloat rd;
	vfloat zero(0);
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

  // Process 16 eles each time.
	for (int i = begin; i < end; i+=16) {
		xi.load(coordinates->x, *(vint*)(nnzs.rows+i), 4);
		yi.load(coordinates->y, *(vint*)(nnzs.rows+i), 4);
		zi.load(coordinates->z, *(vint*)(nnzs.rows+i), 4);
	
		xj.load(coordinates->x, *(vint*)(nnzs.cols+i), 4);
		yj.load(coordinates->y, *(vint*)(nnzs.cols+i), 4);
		zj.load(coordinates->z, *(vint*)(nnzs.cols+i), 4);
	
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

    // gather forces in row direction.
		iforce_tmpx.load(forces->x, (*(vint*)(nnzs.rows+i)).mask(), 4);
		iforce_tmpy.load(forces->y, (*(vint*)(nnzs.rows+i)).mask(), 4);
		iforce_tmpz.load(forces->z, (*(vint*)(nnzs.rows+i)).mask(), 4);

    iforce_tmpx = iforce_tmpx + forcex;
    iforce_tmpy = iforce_tmpy + forcey;
    iforce_tmpz = iforce_tmpz + forcez;

		// Store in row direction.
		iforce_tmpx.store((void*)(forces->x), *(vint*)(nnzs.rows+i), 4);
		iforce_tmpy.store((void*)(forces->y), *(vint*)(nnzs.rows+i), 4);
		iforce_tmpz.store((void*)(forces->z), *(vint*)(nnzs.rows+i), 4);

    // gather forces in col direction.
    jforce_tmpx.load(forces->x, (*(vint*)(nnzs.cols+i)).mask(), 4);
		jforce_tmpy.load(forces->y, (*(vint*)(nnzs.cols+i)).mask(), 4);
		jforce_tmpz.load(forces->z, (*(vint*)(nnzs.cols+i)).mask(), 4);

    jforce_tmpx = jforce_tmpx - forcex;
    jforce_tmpy = jforce_tmpy - forcey;
    jforce_tmpz = jforce_tmpz - forcez;
		
		// Store in col direction.
		jforce_tmpx.store((void*)(forces->x), *(vint*)(nnzs.cols+i), 4);
		jforce_tmpy.store((void*)(forces->y), *(vint*)(nnzs.cols+i), 4);
		jforce_tmpz.store((void*)(forces->z), *(vint*)(nnzs.cols+i), 4);
	}
}

// Do in multi-thread fashion.
template <class ValueType>
void DoMoldyn(const PaddedNnz<ValueType>& nnzs,
              const vector<vector<int> >& offsets,
              const ThreeDSoa<float>* coordinates,
              ThreeDSoa<float>* forces,
              float sideHalf,
              float cutoffSquare,
              int n_threads) {
  double before = rtclock();
  // Do each parallel element.
  for (int i = 0; i < offsets.size(); ++i) {
    #pragma omp parallel for num_threads(n_threads)
    for (int j = 0; j < offsets[i].size() - 1; ++j) {
      Moldyn(nnzs, offsets[i][j], offsets[i][j+1], coordinates, forces, sideHalf, cutoffSquare);
    }
  }
  double after = rtclock();
  cout << RED << "[****Result****] ========> *SIMD Tiling (Our)* " << " on "
       << n_threads << " threads time: " << after - before << " secs." << RESET << endl;
}

int main(int argc, char** argv) {
  double begin = rtclock();
	cout << RESET << "NNZ file: " << string(argv[1]) << endl;
	cout << "Offset file: " << string(argv[2]) << endl;
  cout << "XYZ file: " << string(argv[3]) << endl;

  // Load NNZs and offsets. 
  vector<vector<int> >* offsets; 
  PaddedNnz<int>* nnzs;
  LoadTileFromFile(string(argv[2]), string(argv[1]), offsets, nnzs);
  double after_nnz = rtclock();
	cout << "NNZ and offsets load done, at time of " << after_nnz - begin << endl;

	// Load coordinates.
	ThreeDSoa<float>* coos = LoadCoo(string(argv[3])); 
	ThreeDSoa<float>* forces = new ThreeDSoa<float>(coos->num_nodes);
  double after_coo = rtclock();
	cout << "Coordinates load done, at time of " << after_coo - begin << endl;

  float side = POW( ((float)(coos->num_nodes)/DENSITY), 0.3333333);
	float sideHalf = 0.5 * side;
	float cutoffSquare = (cutoffRadius * cutoffRadius);

  cout << "Max parallelism: " << ComputeMaxParallelism(*offsets) << endl;
  cout << "Min parallelism: " << ComputeMinParallelism(*offsets) << endl;
  cout << "Average parallelism: " << ComputeAverageParallelism(*offsets) << endl;

  cout << "Begin..." << endl;

  DoMoldyn(*nnzs, *offsets, coos, forces, sideHalf, cutoffSquare, 1);
  DoMoldyn(*nnzs, *offsets, coos, forces, sideHalf, cutoffSquare, 2);
  DoMoldyn(*nnzs, *offsets, coos, forces, sideHalf, cutoffSquare, 4);
  DoMoldyn(*nnzs, *offsets, coos, forces, sideHalf, cutoffSquare, 8);
  DoMoldyn(*nnzs, *offsets, coos, forces, sideHalf, cutoffSquare, 16);
  DoMoldyn(*nnzs, *offsets, coos, forces, sideHalf, cutoffSquare, 32);
  DoMoldyn(*nnzs, *offsets, coos, forces, sideHalf, cutoffSquare, 40);
  DoMoldyn(*nnzs, *offsets, coos, forces, sideHalf, cutoffSquare, 50);
  DoMoldyn(*nnzs, *offsets, coos, forces, sideHalf, cutoffSquare, 60);
  DoMoldyn(*nnzs, *offsets, coos, forces, sideHalf, cutoffSquare, 61);
  DoMoldyn(*nnzs, *offsets, coos, forces, sideHalf, cutoffSquare, 80);
  DoMoldyn(*nnzs, *offsets, coos, forces, sideHalf, cutoffSquare, 120);
  DoMoldyn(*nnzs, *offsets, coos, forces, sideHalf, cutoffSquare, 122);
  DoMoldyn(*nnzs, *offsets, coos, forces, sideHalf, cutoffSquare, 150);
  DoMoldyn(*nnzs, *offsets, coos, forces, sideHalf, cutoffSquare, 180);
  DoMoldyn(*nnzs, *offsets, coos, forces, sideHalf, cutoffSquare, 200);
  DoMoldyn(*nnzs, *offsets, coos, forces, sideHalf, cutoffSquare, 240);
  DoMoldyn(*nnzs, *offsets, coos, forces, sideHalf, cutoffSquare, 244);

  cout << "Done." << endl << endl;

	delete coos;
	delete forces;
	delete offsets;
  delete nnzs;
	return 0;
}
