#include "../tools/csr_loader.h"
#include "../tools/fast_hash.h"
#include "util.h"

#include <atomic>
#include <unordered_map>
#include <iostream>
#include <vector>

#include <x86intrin.h>

#include "mkl.h"

using namespace std;

void SpMM(const char* trans, const int* request, 
          const int* sort, const int* m, 
          const int* n, const int* k, 
          float* a, int* ja, int* ia, 
          float* b, int* jb, int* ib, 
          float* c, int* jc, int* ic, 
          int* nzmax, int* info, int num_threads) {
  cout << "Running with " << num_threads << " threads "; 
  mkl_set_num_threads(num_threads);
  double before, after;
  before = rtclock();
  mkl_scsrmultcsr(trans, request, sort, m, n, k, a, ja, ia, b, jb, ib, c, jc, ic, nzmax, info);
  after = rtclock();
  cout << "execution time: " << after - before << endl;
}

int main(int argc, char** argv) {
  cout << "Using input: " << argv[1] << endl;
  Csr<float>* m1 = CsrLoader<float>::Load(argv[1]); 
  cout << "m1 load done." << endl;
  cout << "m1 nnz: " << m1->nnz << endl;

  int info=1;
  int m = m1->num_rows;
  int n = m1->num_cols;
  int k = m1->num_cols;

  float* a = m1->vals;
  int* ja = m1->cols;
  int* ia = m1->rows;

  float* b = m1->vals;
  int* jb = m1->cols;
  int* ib = m1->rows;

  // Convert to 1 based.
  for (int i = 0; i < m1->nnz; ++i) {
    ++ja[i];
  }
  
  for (int i = 0; i < m+1; ++i) {
    ++ia[i];
  }

  int nzmax = atoi(argv[2]);
  int *ic = (int *)malloc(sizeof(int)*(m+1));
  float *c = (float *)malloc(sizeof(float)*nzmax);
  int *jc = (int *)malloc(sizeof(int)*nzmax);
 
  char trans = 'N';
  int request = 0;
  int sort = 8;

  SpMM(&trans, &request, &sort, &m, &n, &k, a, ja, ia, b, jb, ib, c, jc, ic, &nzmax, &info, 1);
  SpMM(&trans, &request, &sort, &m, &n, &k, a, ja, ia, b, jb, ib, c, jc, ic, &nzmax, &info, 1);
  SpMM(&trans, &request, &sort, &m, &n, &k, a, ja, ia, b, jb, ib, c, jc, ic, &nzmax, &info, 2);
  SpMM(&trans, &request, &sort, &m, &n, &k, a, ja, ia, b, jb, ib, c, jc, ic, &nzmax, &info, 4);
  SpMM(&trans, &request, &sort, &m, &n, &k, a, ja, ia, b, jb, ib, c, jc, ic, &nzmax, &info, 8);
  SpMM(&trans, &request, &sort, &m, &n, &k, a, ja, ia, b, jb, ib, c, jc, ic, &nzmax, &info, 16);
  SpMM(&trans, &request, &sort, &m, &n, &k, a, ja, ia, b, jb, ib, c, jc, ic, &nzmax, &info, 32);
  SpMM(&trans, &request, &sort, &m, &n, &k, a, ja, ia, b, jb, ib, c, jc, ic, &nzmax, &info, 40);
  SpMM(&trans, &request, &sort, &m, &n, &k, a, ja, ia, b, jb, ib, c, jc, ic, &nzmax, &info, 50);
  SpMM(&trans, &request, &sort, &m, &n, &k, a, ja, ia, b, jb, ib, c, jc, ic, &nzmax, &info, 61);
  SpMM(&trans, &request, &sort, &m, &n, &k, a, ja, ia, b, jb, ib, c, jc, ic, &nzmax, &info, 80);
  SpMM(&trans, &request, &sort, &m, &n, &k, a, ja, ia, b, jb, ib, c, jc, ic, &nzmax, &info, 122);
  SpMM(&trans, &request, &sort, &m, &n, &k, a, ja, ia, b, jb, ib, c, jc, ic, &nzmax, &info, 150);
  SpMM(&trans, &request, &sort, &m, &n, &k, a, ja, ia, b, jb, ib, c, jc, ic, &nzmax, &info, 180);
  SpMM(&trans, &request, &sort, &m, &n, &k, a, ja, ia, b, jb, ib, c, jc, ic, &nzmax, &info, 200);
  SpMM(&trans, &request, &sort, &m, &n, &k, a, ja, ia, b, jb, ib, c, jc, ic, &nzmax, &info, 244);

  free(ic);
  free(c);
  free(jc);

  delete m1;
  return 0;
}
