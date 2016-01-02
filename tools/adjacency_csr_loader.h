// This is a tool for loading a sparse matrix from
// adjacency list format to a CSR format (Aligned).

#ifndef ADJACENCY_CSR_LOADER_H
#define ADJACENCY_CSR_LOADER_H 

#include "align.h"
#include "aligned_csr.h"
#include <fstream> 
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

template <class ValueType>
class AdjacencyCsrLoader {
 public:
	static Csr<ValueType>* Load (string filename) {
    ifstream input(filename.c_str());
    int nnz, num_rows;
    string first_line;
    getline(input, first_line);
    istringstream ss(first_line);
    ss >> num_rows;
    ss >> nnz;
    vector<vector<int> > cols(num_rows);
    string line;
    int line_no = 0;
    int edge;
    while (getline(input, line)) {
      istringstream iss(line);
      while (iss >> edge) {
        cols[line_no].push_back(edge - 1);
      }  
    }

    cout << "Read file done." << endl;

    int total_nnz = 0;
    int* row_lens = new int[num_rows];
    vector<int> rows;

    int actual_size;
    for (int i = 0; i < num_rows; ++i) {
      rows.push_back(total_nnz);
      row_lens[i] = cols[i].size(); 
      actual_size = align<int>(64, cols[i].size());
      total_nnz += actual_size; 
      cols[i].resize(actual_size, 0);
    }

    cout << "Resize done." << endl;

    Csr<ValueType>* ret = new Csr<ValueType>(num_rows, num_rows, total_nnz);

    cout << "Init csr done." << endl;

    int count = 0;
    for (int i = 0; i < num_rows; ++i) {
      ret->rows[i] = rows[i];
      for (int j = 0; j < cols[i].size(); ++j) {
        ret->cols[count] = cols[i][j];
        ++count;
      }
    }

    ret->rows[num_rows] = total_nnz;
    ret->row_lens = row_lens;
    return ret;
  }
};

#endif /* ADJACENCY_CSR_LOADER_H */
