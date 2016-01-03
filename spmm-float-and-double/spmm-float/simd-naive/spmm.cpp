#include "../tools/csr.h"
#include "../tools/fast_hash.h"
#include "../tools/csr_loader.h"
#include "util.h"
#include "aligned_allocator.hpp"
#include <x86intrin.h>
#include <zmmintrin.h>

#if defined(__INTEL_COMPILER)
#include <malloc.h>
#else
#include <mm_malloc.h>
#endif // defined(__GNUC__)


template <class ValueType>
void SpMM(Csr<ValueType>* m1, Csr<ValueType>* m2, int num_buckets) {

  vector<FastHash<int, ValueType>* > result_map(m1->num_rows);
  for (auto& v : result_map) {
    v = new FastHash<int, ValueType>(num_buckets);
  }

  cout << "Starting SpMM..." << endl;

  float res = 0;
  double before = rtclock();
  for(int i=0;i<m1->num_rows;i++) {
    for(int j=m1->rows[i];j<m1->rows[i+1];j++) {
      int cola = m1->cols[j];
      __m512 a = _mm512_set1_ps(m1->vals[j]);
      for(int k=m2->rows[cola];k<m2->rows[cola] + m2->row_lens[cola];k+=16) {
        __m512 *pb = (__m512 *)(&(m2->vals[k]));
        __m512i *pcols = (__m512i *)(&(m2->cols[k]));
        __m512 c = _mm512_mul_ps(a, *pb);
        for(int x=0;x<16;x++) {
          int col = ((int *)pcols)[x];
          if (col == -1) {
            continue;
          }
          ValueType val = ((ValueType *)(&c))[x];
          result_map[i]->Reduce(col, val);
          res += val;
        }
      }
    }
  }
  double after = rtclock();
  cout << "res: " << res << endl;

  cout << RED << "[****Result****] ========> *SIMD Naive* time: " << after - before << " secs." << RESET << endl;

  for (auto& v : result_map) {
    delete v;
  }
}

int main(int argc, char** argv) {
  cout << "Using input: " << argv[1] << endl;
  Csr<float>* m1 = CsrLoader<float>::AlignedLoad(argv[1]); 
  cout << "m1 load done." << endl;
  cout << "m1 nnz: " << m1->nnz << endl;
  Csr<float>* m2 = CsrLoader<float>::AlignedLoad(argv[1]); 
  cout << "m2 load done." << endl;
  cout << "m2 nnz: " << m2->nnz << endl;
  
  cout << "Total rows: " << m1->num_rows << endl;
  
  // Do the SpMM computation.
  SpMM(m1, m2, atoi(argv[2])); 
  return 0;
}
