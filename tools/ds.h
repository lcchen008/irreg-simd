#ifndef DS_H
#define DS_H 

#include <iostream>
#include <tuple>
#include "align.h"
using namespace std;

// Struct of arrays for moldyn 3D data.
template <class ValueType>
class ThreeDSoa {
 public:
  int num_nodes;
  ValueType* x;
  ValueType* y;
  ValueType* z;

  ThreeDSoa (int num_nodes) {
    this->num_nodes = num_nodes;
    x = (ValueType*)_mm_malloc(sizeof(ValueType)*num_nodes, 64);  
    y = (ValueType*)_mm_malloc(sizeof(ValueType)*num_nodes, 64);  
    z = (ValueType*)_mm_malloc(sizeof(ValueType)*num_nodes, 64);  
  } 

  void SetXyz(const vector<tuple<ValueType, ValueType, ValueType> >& xyz) {
    for (int i = 0; i < num_nodes; ++i) {
      x[i] = get<0>(xyz[i]);
      y[i] = get<1>(xyz[i]);
      z[i] = get<2>(xyz[i]);
    }
  }

  ~ThreeDSoa() {
    _mm_free(x);
    _mm_free(y);
    _mm_free(z);
  }
};

// This class is used for storing the padded nnzs continuously.
template <class ValueType>
class PaddedNnz {
 public:
  int nnz;
  int* rows;
  int* cols;
  ValueType* vals;

  // nnz is the total nnz after padding, which means,
  // it is a multiple of 16.
  PaddedNnz(int nnz) {
    this->nnz = nnz; 
    rows = (int*)_mm_malloc(sizeof(int)*nnz, 64); 
    cols = (int*)_mm_malloc(sizeof(int)*nnz, 64); 
    vals = (ValueType*)_mm_malloc(sizeof(ValueType)*nnz, 64); 
  }
	
  ~PaddedNnz() {
    _mm_free(rows);
    _mm_free(cols);
    _mm_free(vals);
  }
};

// This class is used as containers for tiled sparse
// matrices.
template <class ValueType>
class Coo {
 public:
	int nnz;
  int actual_nnz;

	// Dimensional size of the tile.
	int width;

	// The following three items should be aligned.
	int* rows;
	int* cols;
	ValueType* vals;

	Coo(const vector<tuple<int, int, ValueType> >& elements) {
		// Allocate aligned spaces.
    this->actual_nnz = elements.size();
		this->nnz = align<ValueType>(64, elements.size());
		rows = (int*)_mm_malloc(sizeof(int)*nnz, 64);
		cols = (int*)_mm_malloc(sizeof(int)*nnz, 64);
		vals = (ValueType*)_mm_malloc(sizeof(ValueType)*nnz, 64);
		// Copy elements.
		for (int i = 0; i < elements.size(); ++i) {
			rows[i] = get<0>(elements[i]);
			cols[i] = get<1>(elements[i]);
			vals[i] = get<2>(elements[i]);
		}
		// Padding.
		for (int i = elements.size(); i < nnz; ++i) {
			rows[i] = rows[0];
			cols[i] = cols[0];
			vals[i] = vals[0];
		}
	}

  void Refill(const vector<tuple<int, int, ValueType> >& elements) {
		_mm_free(rows);
		_mm_free(cols);
		_mm_free(vals);

    this->actual_nnz = elements.size();
	  this->nnz = align<ValueType>(64, elements.size());
	  rows = (int*)_mm_malloc(sizeof(int)*nnz, 64);
	  cols = (int*)_mm_malloc(sizeof(int)*nnz, 64);
	  vals = (ValueType*)_mm_malloc(sizeof(ValueType)*nnz, 64);
	  // Copy elements.
	  for (int i = 0; i < elements.size(); ++i) {
	  	rows[i] = get<0>(elements[i]);
	  	cols[i] = get<1>(elements[i]);
	  	vals[i] = get<2>(elements[i]);
	  }
	  // Padding.
	  for (int i = elements.size(); i < nnz; ++i) {
	  	rows[i] = rows[0];
	  	cols[i] = cols[0];
	  	vals[i] = vals[0];
	  }
  }

	void TransferOwnership() {
	  rows = 0;
	  cols = 0;
	  vals = 0;
	}

  void Free() {
    if (rows) {
		  _mm_free(rows);
      rows = 0;
    }
    if (cols) {
		  _mm_free(cols);
      cols = 0;
    }
    if (vals) {
		  _mm_free(vals);
      vals = 0;
    }
  }

	~Coo() {
    /*
    cout << "rows: " << rows << endl;
    if (rows) {
		  _mm_free(rows);
      rows = 0;
    }
    cout << "cols: " << cols << endl;
    if (cols) {
		  _mm_free(cols);
      cols = 0;
    }
    cout << "vals: " << vals << endl;
    if (vals) {
		  _mm_free(vals);
      vals = 0;
    }
    */
	}
};

// Operator to output the value of a Coo tile.
template <class ValueType>
ostream& operator<<(ostream& os, const Coo<ValueType>& tile) {
	for (int i = 0; i < tile.nnz; ++i) {
		os << "(" << tile.rows[i] << ", " << tile.cols[i] << ", " << tile.vals[i] << ") ";
	}
	return os;
}

#endif /* DS_H */
