#include "../tools/csr_loader.h"
#include "../tools/fast_hash.h"
#include "util.h"

#include <atomic>
#include <unordered_map>
#include <iostream>
#include <vector>

#include <x86intrin.h>

using namespace std;

template<class ValueType>
Csr<ValueType>* SpMM(const Csr<ValueType>* m1, const Csr<ValueType>* m2, vector<FastHash<int, ValueType>* >& result_map) {
  cout << "Starting SpMM..." << endl;

  float a;

  double before = rtclock();
  for (int i = 0; i < m1->num_rows; ++i) {
    for (int j = m1->rows[i]; j < m1->rows[i+1]; ++j) {
      int col = m1->cols[j];
      ValueType val = m1->vals[j];

      for (int k = m2->rows[col]; k < m2->rows[col+1]; ++k) {
        int col2 = m2->cols[k];
        ValueType val2 = m2->vals[k];
        ValueType mul = val * val2;
        a += mul;
        result_map[i]->Reduce(col2, mul);
      }
    }
  }

  double after = rtclock();
  cout << "a: " << a << endl;
  cout << RED << "[****Result****] ========> *Serial* time: " << after - before << " secs." << RESET << endl;

  // Recover csr from hash table.
  Csr<ValueType>* res = new Csr<ValueType>(result_map, m1->num_rows, m2->num_cols);

  return res;
}

int main(int argc, char** argv) {
  cout << "Using input: " << argv[1] << endl;
  cout << "Loading input..." << endl;
  Csr<float>* m1 = CsrLoader<float>::Load(argv[1]); 
  cout << "m1 load done." << endl;
  cout << "m1 nnz: " << m1->nnz << endl;
  Csr<float>* m2 = CsrLoader<float>::Load(argv[1]); 
  cout << "m2 load done." << endl;
  cout << "Total rows: " << m1->num_rows << endl;
  vector<FastHash<int, float >* > row_results(m1->num_rows);
  for (auto& v : row_results) {
    v = new FastHash<int, float>(atoi(argv[2]));
  }

  // Do the SpMM computation.
  Csr<float>* res = SpMM(m1, m2, row_results); 
  cout << "Result size: " << res->nnz << endl;

  cout << "Done." << endl;

  // Free row_results.
  for (auto& v : row_results) {
    delete v;
  }

  delete m1;
  delete m2;
  delete res;
  return 0;
}
