// This is a tool for loading a sparse matrix from
// Matrix Market format to a CSR format.

#ifndef CSR_LOADER_H
#define CSR_LOADER_H 

#include "align.h"
#include "csr.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

#define RED "\033[31m"
#define BLUE "\033[34m" 
#define RESET "\033[0m"

template<class ValueType>
struct CsrLoader {
  // static
  static Csr<ValueType>* Load(string filename) {
    ifstream input(filename.c_str()); 
    string line;

    // Get size info.
    int num_rows, num_cols, nnz;
    while (getline(input, line)) {
      // Skip white spaces.
      while (line[0] == ' ') {
        line.erase(0, 1);
      }
      // If it is not comment,
      // the line contains size info.
      if (line[0] != '%') {
        istringstream ss(line); 
        ss >> num_rows;
        ss >> num_cols;
        ss >> nnz;
        break;
      }
    }

    // cout << "Num of rows: " << num_rows << endl;

    // Allocate buffers.
    vector<vector<int> > cols(num_rows);
    vector<vector<ValueType> > vals(num_rows);

    while (getline(input, line)) {
      // Skip white spaces.
      while (line[0] == ' ') {
        line.erase(0, 1);
      }
      // Skip comments.
      if (line[0] != '%') {
        istringstream ss(line);
        int r, c;
        // Use default value, binary 1, if the input
        // has no value column.
        ValueType v = 1;
        ss >> r;
        ss >> c;
        ss >> v;
        cols[r-1].push_back(c-1);
        vals[r-1].push_back(v);
      }
    }

    input.close();

    // Create the object from buffers.
    Csr<ValueType>* csr = new Csr<ValueType>(num_rows, num_cols, nnz);

    csr->rows[0] = 0;
    for (int i = 1; i < num_rows; ++i) {
      csr->rows[i] = csr->rows[i-1] + cols[i-1].size(); 
    }
    csr->rows[num_rows] = nnz; 

    int count = 0;
    for (int i = 0; i < num_rows; ++i) {
      for (int j = 0; j < cols[i].size(); ++j) {
        csr->cols[count] = cols[i][j];
        csr->vals[count] = vals[i][j];
        ++count;
      } 
    }

    return csr;
  } 

  // Load the sparse matrix to an aligned fashion.
  // static
  static Csr<ValueType>* AlignedLoad(string filename) {
    ifstream input(filename.c_str()); 
    string line;

    // Get size info.
    int num_rows, num_cols, nnz;
    while (getline(input, line)) {
      // Skip white spaces.
      while (line[0] == ' ') {
        line.erase(0, 1);
      }
      // If it is not comment,
      // the line contains size info.
      if (line[0] != '%') {
        istringstream ss(line); 
        ss >> num_rows;
        ss >> num_cols;
        ss >> nnz;
        break;
      }
    }

    // Allocate buffers.
    vector<vector<int> > cols(num_rows);
    vector<vector<ValueType> > vals(num_rows);

    while (getline(input, line)) {
      // Skip white spaces.
      while (line[0] == ' ') {
        line.erase(0, 1);
      }
      // Skip comments.
      if (line[0] != '%') {
        istringstream ss(line);
        int r, c;
        // Use default value, binary 1 if the input
        // has no value column.
        ValueType v = 1;
        ss >> r;
        ss >> c;
        ss >> v;
        cols[r-1].push_back(c-1);
        vals[r-1].push_back(v);
      }
    }
    input.close();

    // Get total size after aligned.
    int total_size = 0;
    int* row_lens = new int[num_rows];
    int long_rows = 0;
    int longgest = 0;
    for (int i = 0; i < num_rows; ++i) {
      // Here we assume the SIMD length is 512 bits. 
      row_lens[i] = cols[i].size();
      longgest = max(longgest, row_lens[i]);
      if (row_lens[i] >= 16) {
        ++long_rows;
      }
      int aligned_size = align<ValueType>(64, cols[i].size());
      total_size += aligned_size;
      cols[i].resize(aligned_size, -1);
      vals[i].resize(aligned_size, 0);
    }

    //cout << "Long rows: " << long_rows << endl;
    //cout << "Max nnz: " << longgest << endl;

    // Create the object from buffers.
    Csr<ValueType>* csr = new Csr<ValueType>(num_rows, num_cols, total_size);
    csr->row_lens = row_lens;

    csr->rows[0] = 0;
    for (int i = 1; i < num_rows; ++i) {
      csr->rows[i] = csr->rows[i-1] + cols[i-1].size(); 
    }
    csr->rows[num_rows] = total_size;

    int count = 0;
    for (int i = 0; i < num_rows; ++i) {
      for (int j = 0; j < cols[i].size(); ++j) {
        csr->cols[count] = cols[i][j];
        csr->vals[count] = vals[i][j];
        ++count;
      } 
    }

    return csr;
  }
};

#endif /* CSR_LOADER_H */
