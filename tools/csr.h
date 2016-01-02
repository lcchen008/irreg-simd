#ifndef MYCSR_H
#define MYCSR_H 

#include <iostream>
#include <vector>
using namespace std;

template <class ValueType>
class MyCsr {
 public:
  int num_rows;
  int num_cols;
  int nnz;
  vector<int> rows;
  vector<int> cols;
  vector<ValueType> vals;

  MyCsr() {}

  MyCsr(int num_rows, int num_cols, int nnz) {
    this->num_rows = num_rows;
    this->num_cols = num_cols;
    this->nnz = nnz;
    this->rows.resize(num_rows + 1);
    this->cols.resize(nnz);
    this->vals.resize(nnz);
  }

  void print() {
    for (int i = 0; i < num_rows; ++i) {
      cout << "Row: " << i << ":" << endl;
      for (int j = rows[i]; j < rows[i+1]; ++j) {
        cout << "(Col: " << cols[j] << " Val: " << vals[j] << "), ";
      }
      cout << endl;
    }
  }

  ~MyCsr() {
  }
};

#endif /* MYCSR_H */
