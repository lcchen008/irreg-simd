#ifndef LOAD_TILE_FROM_FILE_H
#define LOAD_TILE_FROM_FILE_H 

#include "ds.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple>
#include <vector>
using namespace std;

// Used for loading offsets and nnzs from tiling result.
// Return value are passed through arguments.
template <class ValueType>
void LoadTileFromFile(const string offset_file, const string tiling_file, vector<vector<int> >*& offsets, PaddedNnz<ValueType>*& nnzs) {
  ifstream offset_input(offset_file.c_str());
  ifstream tiling_input(tiling_file.c_str());
  int total_nnz; 
  offset_input >> total_nnz; 
  // Load nnzs.
  nnzs = new PaddedNnz<ValueType>(total_nnz);
  string line;
  int count = 0;
  while (getline(tiling_input, line)) {
    istringstream iss(line);
    int row, col;
    ValueType val;
    while (iss >> row) {
      iss >> col; 
      iss >> val;
      nnzs->rows[count] = row;
      nnzs->cols[count] = col;
      nnzs->vals[count] = val;
      ++count;
    }
  }

  // Load offsets.
  offsets = new vector<vector<int> >;
  while (getline(offset_input, line)) {
    offsets->emplace_back();
    istringstream iss(line);
    int offset;
    while (iss >> offset) {
      offsets->back().push_back(offset);
    }
  }
  
  // Add end offset to each parallel group, used for
  // indicating where each tile (before tiling) ends in.
  for (int i = 0; i < offsets->size() - 1; ++i) {
    (*offsets)[i].push_back((*offsets)[i+1][0]);
  }
  offsets->back().push_back(total_nnz);
}

// Used for loading nnzs from tiling result (without reducing 
// inter-lane conflicts, for seq execution).
// Return value are passed through arguments.
template <class ValueType>
void LoadTileFromFile(const string tiling_file, PaddedNnz<ValueType>*& nnzs) {
  ifstream tiling_input(tiling_file.c_str());
  vector<tuple<int, int, ValueType> > tmp;
  // Load nnzs.
  string line;
  while (getline(tiling_input, line)) {
    istringstream iss(line);
    int row, col;
    ValueType val;
    while (iss >> row) {
      iss >> col; 
      iss >> val;
      tmp.emplace_back(row, col, val);
    }
  }

  nnzs = new PaddedNnz<ValueType>(tmp.size());

  for (int i = 0; i < tmp.size(); ++i) {
    nnzs->rows[i] = get<0>(tmp[i]);
    nnzs->cols[i] = get<1>(tmp[i]);
    nnzs->vals[i] = get<2>(tmp[i]);
  }

  cout << "Load done and size is: " << nnzs->nnz << endl;
}

#endif /* LOAD_TILE_FROM_FILE_H */
