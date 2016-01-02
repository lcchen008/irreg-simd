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

template<class ValueType>
struct CsrLoader {
  // static
  static MyCsr<ValueType>* Load(string filename) {
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

    cout << "Num of rows: " << num_rows << endl;

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
    MyCsr<ValueType>* csr = new MyCsr<ValueType>(num_rows, num_cols, nnz);

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
};

#endif /* CSR_LOADER_H */
