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
  // Conduct 4 iterations of SIMD multiplication and reduction. 
  __m512i vmp2;
  __m512i mult_res;
  __m512i res_iter;

  for (int i = 0; i < 4; ++i) {
    // Permute vm2:
    vmp2 = _mm512_i32gather_epi32(gather_index[i], &m2.values, 4);
    // Mult two vectors:
    mult_res = _mm512_mullo_epi32(*(__m512i*)&m1.values, vmp2);
    // Permute mult result (using scatter) and write to result.
    _mm512_i32scatter_epi32(&res_iter, scatter_index[i], mult_res, 4);
    mul += *(Tile<int>*)&res_iter;
  }
}

template <class ValueType>
void Do(atomic<int>* task_offset,
        const Csr<Tile<ValueType> >* mm1, const Csr<Tile<ValueType> >* mm2,
        vector<FastHash<int, Tile<ValueType> >* >& row_results, __m512i* gather_index,
        __m512i* scatter_index) {
  int row_index;

  float check_sum = 0;

  while (1) {
    row_index = task_offset->fetch_add(BLOCK_SIZE, std::memory_order_relaxed);
    if (row_index >= mm1->num_rows) {
      break;
    }
    for (int i = row_index;
         i < row_index + BLOCK_SIZE && i < mm1->num_rows;
         ++i) {
      // Process a row.
      for (int j = mm1->rows[i]; j < mm1->rows[i+1]; ++j) {
        int& col = mm1->cols[j]; 
        Tile<ValueType>& val1 = mm1->vals[j];

        for (int k = mm2->rows[col]; k < mm2->rows[col+1]; ++k) {
          int& col2 = mm2->cols[k];
          Tile<ValueType>& val2 = mm2->vals[k];
          // Conduct a tile-wise multiplication.
          Tile<ValueType> mul;
          FloatTileMm(val1, val2, gather_index, scatter_index, mul);
          row_results[i]->Reduce(col2, mul);
          for (int x = 0; x < 16; ++x) {
            check_sum += mul[x];
          }
        }
      }
    }
  }
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
  Tile<ValueType>* muls =(Tile<ValueType>*)_mm_malloc(
      sizeof(Tile<ValueType>)*num_threads, 64);
#pragma omp parallel for schedule(dynamic, 20) num_threads(num_threads)
  for (int i = 0; i < m1->num_rows; ++i) {
    // Process a row.
    Tile<ValueType>& mul = muls[omp_get_thread_num()];
    for (int j = m1->rows[i]; j < m1->rows[i+1]; ++j) {
      int& col = m1->cols[j]; 
      Tile<ValueType>& val1 = m1->vals[j];
      for (int k = m2->rows[col]; k < m2->rows[col+1]; ++k) {
        int& col2 = m2->cols[k];
        Tile<ValueType>& val2 = m2->vals[k];
        // Conduct a tile-wise multiplication.
        mul.SetVectorZero();
        FloatTileMm(val1, val2, gather_index, scatter_index, mul);
        row_results[i]->Reduce(col2, mul);
      }
    }
  }
  _mm_free(muls);
  double after = rtclock();

  cout << RED << "[****Result****] ========> *SIMD Tiling (Our)* " << " on "
      << num_threads << " threads time: " << after - before << " secs." << RESET << endl;

  float check_sum = 0;
  for (const auto v : row_results) {
    check_sum += v->CheckSum();
  }

  cout << "Check sum: " << check_sum << endl;
  cout << endl;

  Csr<Tile<float> >* tc_res = new Csr<Tile<float> >(row_results, m1->num_rows, m2->num_cols);  
  return tc_res;
}

int main(int argc, char** argv) {
  cout << "Using input: " << argv[1] << endl;
  Csr<Tile<float> >* tc1 = TiledCsr<float>::TiledCsrFromFile(4, 4, argv[1]); 
  cout << "First load done..." << endl;
  Csr<Tile<float> >* tc2 = TiledCsr<float>::TiledCsrFromFile(4, 4, argv[1]);
  cout << "Second load done..." << endl;

  vector<FastHash<int, Tile<float> >* > row_results(tc1->num_rows);
  for (auto& v : row_results) {
    v = new FastHash<int, Tile<float> >(atoi(argv[2]));
  }

  cout << "Load done and doing SpMM..." << endl;

  Csr<Tile<float> >* res = SpMM(tc1, tc2, 1, row_results);
  delete res;

  Csr<Tile<float> >* res0 = SpMM(tc1, tc2, 1, row_results);
  delete res0;
  Csr<Tile<float> >* res1 = SpMM(tc1, tc2, 2, row_results);
  delete res1;
  Csr<Tile<float> >* res2 = SpMM(tc1, tc2, 4, row_results);
  delete res2;
  Csr<Tile<float> >* res3 = SpMM(tc1, tc2, 8, row_results);
  delete res3;
  Csr<Tile<float> >* res4 = SpMM(tc1, tc2, 16, row_results);
  delete res4;
  Csr<Tile<float> >* res5 = SpMM(tc1, tc2, 32, row_results);
  delete res5;
  Csr<Tile<float> >* res6 = SpMM(tc1, tc2, 40, row_results);
  delete res6;
  Csr<Tile<float> >* res7 = SpMM(tc1, tc2, 50, row_results);
  delete res7;
  Csr<Tile<float> >* res8 = SpMM(tc1, tc2, 61, row_results);
  delete res8;
  Csr<Tile<float> >* res9 = SpMM(tc1, tc2, 80, row_results);
  delete res9;
  Csr<Tile<float> >* res10 = SpMM(tc1, tc2, 122, row_results);
  delete res10;
  Csr<Tile<float> >* res11 = SpMM(tc1, tc2, 150, row_results);
  delete res11;
  Csr<Tile<float> >* res12 = SpMM(tc1, tc2, 180, row_results);
  delete res12;
  Csr<Tile<float> >* res13 = SpMM(tc1, tc2, 200, row_results);
  delete res13;
  Csr<Tile<float> >* res14 = SpMM(tc1, tc2, 244, row_results);
  cout << "Size of result: " << res14->nnz << endl;
  delete res14;

  // Free row_results.
  for (auto& v : row_results) {
    delete v;
  }

  delete tc1;
  delete tc2;
  return 0;
}
