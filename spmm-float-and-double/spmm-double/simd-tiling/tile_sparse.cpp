#include "args.h"
#include "conf.h"
#include <omp.h>
#include "../tools/csr_loader.h"
#include "../tools/fast_hash.h"
#include "tiled_csr.h"
#include "util.h"

#include <atomic>
#include <unordered_map>
#include <iostream>
#include <thread>
#include <vector>

#include <x86intrin.h>

using namespace std;

inline void FloatTileMm(Tile<float>& m1, Tile<float>& m2, __m512i* gather_index, __m512i* scatter_index, Tile<float>& mul) {
  // Conduct 4 iterations of SIMD multiplication and reduction. 
  __m512 vmp2;
  __m512 mult_res;
  __m512 res_iter;

  for (int i = 0; i < 4; ++i) {
    // Permute vm2:
    vmp2 = _mm512_i32gather_ps(gather_index[i], &m2.values, 4);
    // Mult two vectors:
    mult_res = _mm512_mul_ps(*(__m512*)&m1.values, vmp2);
    // Permute mult result (using scatter) and write to result.
    _mm512_i32scatter_ps(&res_iter, scatter_index[i], mult_res, 4);
    mul += *(Tile<float>*)&res_iter;
  }
}

void IntTileMm(Tile<int>& m1, Tile<int>& m2, __m512i* gather_index, __m512i* scatter_index, Tile<int>& mul) {
  // Load tiles.
  __m512i vm2 = _mm512_load_epi32(&m2.values);

  // Conduct 4 iterations of SIMD multiplication and reduction. 
  __m512i vmp2;
  __m512i mult_res;
  __m512i res_iter;

  for (int i = 0; i < 4; ++i) {
    // Permute vm2:
    vmp2 = _mm512_i32gather_epi32(gather_index[i], &vm2, 4);
    // Mult two vectors:
    mult_res = _mm512_mullo_epi32(*(__m512i*)&m1.values, vmp2);
    // Permute mult result (using scatter) and write to result.
    _mm512_i32scatter_epi32(&res_iter, scatter_index[i], mult_res, 4);
    mul += *(Tile<int>*)&res_iter;
  }
}

inline void DoubleTileMm(Tile<double>& m1, Tile<double>& m2, 
                         __m512i* gather_index1, 
                         __m512i* gather_index2, 
                         __m512i* scatter_index1, 
                         __m512i* scatter_index2, 
                         Tile<double>& mul) {
  __m512d vmp2_p1;
  __m512d vmp2_p2;

  __m512d mult_res1;
  __m512d mult_res2;
  Tile<double> res_iter;

  for (int i = 0; i < 4; ++i) {
    // Permute vm2:
    vmp2_p1 = _mm512_i32logather_pd(gather_index1[i], &m2.values, 8);
    vmp2_p2 = _mm512_i32logather_pd(gather_index2[i], &m2.values, 8);
    // Mult two vectors:
    mult_res1 = _mm512_mul_pd(*(__m512d*)&m1.values, vmp2_p1);
    mult_res2 = _mm512_mul_pd(*(__m512d*)(&(m1.values[0]) + 8), vmp2_p2);
    // Permute mult result (using scatter) and write to result.
    _mm512_i32loscatter_pd(&res_iter.values, scatter_index1[i], mult_res1, 8);
    _mm512_i32loscatter_pd(&res_iter.values, scatter_index2[i], mult_res2, 8);
    mul += res_iter;
  }
}

// Double version of SpMM.
template<class ValueType>
Csr<Tile<ValueType> >* SpMMD(const Csr<Tile<ValueType> >* m1,
                             const Csr<Tile<ValueType> >* m2,
                             int num_threads,
                             vector<FastHash<int, Tile<ValueType> >* >& row_results) {
  // Indices used for loading elements from m2.
  __m512i gather_index1[4];
  __m512i gather_index2[4];

  __m512i scatter_index1[4]; 
  __m512i scatter_index2[4]; 

  gather_index1[0] = _mm512_set_epi32(0, 0, 0, 0, 0, 0, 0, 0, 12, 11, 6, 1, 15, 10, 5, 0);
  gather_index2[0] = _mm512_set_epi32(0, 0, 0, 0, 0, 0, 0, 0, 14, 9, 4, 3, 13, 8, 7, 2);
  gather_index1[1] = _mm512_set_epi32(0, 0, 0, 0, 0, 0, 0, 0, 13, 8, 7, 2, 12, 11, 6, 1);
  gather_index2[1] = _mm512_set_epi32(0, 0, 0, 0, 0, 0, 0, 0, 15, 10, 5, 0, 14, 9, 4, 3);
  gather_index1[2] = _mm512_set_epi32(0, 0, 0, 0, 0, 0, 0, 0, 14, 9, 4, 3, 13, 8, 7, 2);
  gather_index2[2] = _mm512_set_epi32(0, 0, 0, 0, 0, 0, 0, 0, 12, 11, 6, 1, 15, 10, 5, 0);
  gather_index1[3] = _mm512_set_epi32(0, 0, 0, 0, 0, 0, 0, 0, 15, 10, 5, 0, 14, 9, 4, 3);
  gather_index2[3] = _mm512_set_epi32(0, 0, 0, 0, 0, 0, 0, 0, 13, 8, 7, 2, 12, 11, 6, 1);

  scatter_index1[0] = _mm512_set_epi32(0, 0, 0, 0, 0, 0, 0, 0, 4, 7, 6, 5, 3, 2, 1, 0);
  scatter_index2[0] = _mm512_set_epi32(0, 0, 0, 0, 0, 0, 0, 0, 14, 13, 12, 15, 9, 8, 11, 10);
  scatter_index1[1] = _mm512_set_epi32(0, 0, 0, 0, 0, 0, 0, 0, 5, 4, 7, 6, 0, 3, 2, 1);
  scatter_index2[1] = _mm512_set_epi32(0, 0, 0, 0, 0, 0, 0, 0, 15, 14, 13, 12, 10, 9, 8, 11);
  scatter_index1[2] = _mm512_set_epi32(0, 0, 0, 0, 0, 0, 0, 0, 6, 5, 4, 7, 1, 0, 3, 2);
  scatter_index2[2] = _mm512_set_epi32(0, 0, 0, 0, 0, 0, 0, 0, 12, 15, 14, 13, 11, 10, 9, 8);
  scatter_index1[3] = _mm512_set_epi32(0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 5, 4, 2, 1, 0, 3);
  scatter_index2[3] = _mm512_set_epi32(0, 0, 0, 0, 0, 0, 0, 0, 13, 12, 15, 14, 8, 11, 10, 9);

  double before = rtclock();
#pragma omp parallel for schedule(dynamic, 20) num_threads(num_threads)
  for (int i = 0; i < m1->num_rows; ++i) {
    // Process a row.
    for (int j = m1->rows[i]; j < m1->rows[i+1]; ++j) {
      int& col = m1->cols[j]; 
      Tile<ValueType>& val1 = m1->vals[j];
      for (int k = m2->rows[col]; k < m2->rows[col+1]; ++k) {
        int& col2 = m2->cols[k];
        Tile<ValueType>& val2 = m2->vals[k];
        // Conduct a tile-wise multiplication.
        Tile<ValueType> mul;
        DoubleTileMm(val1, val2, gather_index1,
                     gather_index2, scatter_index1,
                     scatter_index2, mul);
        row_results[i]->Reduce(col2, mul);
      }
    }
  }
  double after = rtclock();

  cout << RED << "[****Result****] ========> *SIMD Tiling (Our)* " << " on "
      << num_threads << " threads time: " << after - before << " secs." << RESET << endl;

  cout << "Doing checksum..." << endl;
  double check_sum = 0;
  for (const auto v : row_results) {
    check_sum += v->CheckSum();
  }
  cout << "Check sum: " << check_sum << endl;

  // Recover tiled_csr from hash_table.
  Csr<Tile<double> >* tc_res = new Csr<Tile<double> >(row_results, m1->num_rows, m2->num_cols);  
  return tc_res;
}

template<class ValueType>
Csr<Tile<ValueType> >* SpMM(const Csr<Tile<ValueType> >* m1,
                            const Csr<Tile<ValueType> >* m2,
                            int num_threads,
                            vector<FastHash<int, Tile<ValueType> >* >& row_results) {
  atomic<int> task_offset;
  task_offset = 0;

  __m512i gather_index[4];
  __m512i scatter_index[4]; 

  gather_index[0] = _mm512_set_epi32(14, 9, 4, 3, 13, 8, 7, 2, 12, 11, 6, 1, 15, 10, 5, 0);
  gather_index[1] = _mm512_set_epi32(15, 10, 5, 0, 14, 9, 4, 3, 13, 8, 7, 2, 12, 11, 6, 1);
  gather_index[2] = _mm512_set_epi32(12, 11, 6, 1, 15, 10, 5, 0, 14, 9, 4, 3, 13, 8, 7, 2);
  gather_index[3] = _mm512_set_epi32(13, 8, 7, 2, 12, 11, 6, 1, 15, 10, 5, 0, 14, 9, 4, 3);

  scatter_index[0] = _mm512_set_epi32(14, 13, 12, 15, 9, 8, 11, 10, 4, 7, 6, 5, 3, 2, 1, 0);
  scatter_index[1] = _mm512_set_epi32(15, 14, 13, 12, 10, 9, 8, 11, 5, 4, 7, 6, 0, 3, 2, 1);
  scatter_index[2] = _mm512_set_epi32(12, 15, 14, 13, 11, 10, 9, 8, 6, 5, 4, 7, 1, 0, 3, 2);
  scatter_index[3] = _mm512_set_epi32(13, 12, 15, 14, 8, 11, 10, 9, 7, 6, 5, 4, 2, 1, 0, 3);

  double before = rtclock();
#pragma omp parallel for schedule(dynamic, 20) num_threads(num_threads)
  for (int i = 0; i < m1->num_rows; ++i) {
    // Process a row.
    for (int j = m1->rows[i]; j < m1->rows[i+1]; ++j) {
      int& col = m1->cols[j]; 
      Tile<ValueType>& val1 = m1->vals[j];
      for (int k = m2->rows[col]; k < m2->rows[col+1]; ++k) {
        int& col2 = m2->cols[k];
        Tile<ValueType>& val2 = m2->vals[k];
        // Conduct a tile-wise multiplication.
        Tile<ValueType> mul;
        FloatTileMm(val1, val2, gather_index, scatter_index, mul);
        row_results[i]->Reduce(col2, mul);
      }
    }
  }
  double after = rtclock();

  float check_sum = 0;
  for (const auto v : row_results) {
    check_sum += v->CheckSum();
  }

  cout << RED << "[****Result****] ========> *SIMD Tiling (Our)* " << " on "
      << num_threads << " threads time: " << after - before << " secs." << RESET << endl;
  cout << "Check sum: " << check_sum << endl;

  // Recover tiled_csr from hash_table.
  Csr<Tile<float> >* tc_res = new Csr<Tile<float> >(row_results, m1->num_rows, m2->num_cols);  
  return tc_res;
}

int main(int argc, char** argv) {
  cout << "Using input: " << argv[1] << endl;
  Csr<Tile<double> >* tc1 = TiledCsr<double>::TiledCsrFromFile(4, 4, argv[1]); 
  cout << "Load and convert done." << endl;
  cout << "tc1 Total nnz: " << tc1->nnz << endl;
  Csr<Tile<double> >* tc2 = TiledCsr<double>::TiledCsrFromFile(4, 4, argv[1]);
  cout << "Load and convert done." << endl;
  cout << "tc2 Total nnz: " << tc2->nnz << endl;
  // Do the SpMM computation.
  cout << "Doing SpMM..." << endl;

  vector<FastHash<int, Tile<double> >* > row_results(tc1->num_rows);
  for (auto& v : row_results) {
    v = new FastHash<int, Tile<double> >(atoi(argv[2]));
  }

  Csr<Tile<double> >* res = SpMMD(tc1, tc2, 1, row_results);
  cout << "Size of result: " << res->nnz << endl;
  delete res;

  Csr<Tile<double> >* res0 = SpMMD(tc1, tc2, 1, row_results);
  delete res0;
  Csr<Tile<double> >* res1 = SpMMD(tc1, tc2, 2, row_results);
  delete res1;
  Csr<Tile<double> >* res2 = SpMMD(tc1, tc2, 4, row_results);
  delete res2;
  Csr<Tile<double> >* res3 = SpMMD(tc1, tc2, 8, row_results);
  delete res3;
  Csr<Tile<double> >* res4 = SpMMD(tc1, tc2, 16, row_results);
  delete res4;
  Csr<Tile<double> >* res5 = SpMMD(tc1, tc2, 32, row_results);
  delete res5;
  Csr<Tile<double> >* res6 = SpMMD(tc1, tc2, 40, row_results);
  delete res6;
  Csr<Tile<double> >* res7 = SpMMD(tc1, tc2, 50, row_results);
  delete res7;
  Csr<Tile<double> >* res8 = SpMMD(tc1, tc2, 61, row_results);
  delete res8;
  Csr<Tile<double> >* res9 = SpMMD(tc1, tc2, 80, row_results);
  delete res9;
  Csr<Tile<double> >* res10 = SpMMD(tc1, tc2, 122, row_results);
  delete res10;
  Csr<Tile<double> >* res11 = SpMMD(tc1, tc2, 150, row_results);
  delete res11;
  Csr<Tile<double> >* res12 = SpMMD(tc1, tc2, 180, row_results);
  delete res12;
  Csr<Tile<double> >* res13 = SpMMD(tc1, tc2, 200, row_results);
  delete res13;
  Csr<Tile<double> >* res14 = SpMMD(tc1, tc2, 244, row_results);
  delete res14;


  cout << "Done." << endl;
  // Free row_results.
  for (auto& v : row_results) {
    delete v;
  }

  delete tc1;
  delete tc2;
  return 0;
}
