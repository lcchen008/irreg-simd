// Aligned csr used for naive simd execution.

#ifndef ALIGNED_CSR_H
#define ALIGNED_CSR_H 

template <class ValueType>
struct Csr {
  int num_rows;
  int num_cols;
  int nnz;
  int* rows; 
  int* cols;
  ValueType* vals;
  // Indicates the size of each row.
  // To be set by users.
  // Only used for aligned CSRs.
  int* row_lens;

  Csr(int num_rows, int num_cols, int nnz) {
    this->num_rows = num_rows;
    this->num_cols = num_cols;
    this->nnz = nnz;
    rows = new int[num_rows + 1]; 

    // Here we assume the lane width is 64 bytes.
    cols = (int*)_mm_malloc(sizeof(int)*nnz, 64);
    vals = 0;

    row_lens = 0;
  }

  ~Csr() {
    delete [] rows;
    _mm_free(cols);
    if (row_lens) {
      delete [] row_lens;
    }
    if (vals) {
      _mm_free(vals);
    }
  }
};

#endif /* ALIGNED_CSR_H */
