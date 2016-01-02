#ifndef TILED_CSR_H
#define TILED_CSR_H 
#include "../tools/csr_loader.h"
#include "../tools/fast_hash.h"

#include <iostream>
#include <unordered_map>
#include <x86intrin.h>
using namespace std;

template <class ValueType>
struct Tile {
 public:
  // Assume value type is of 4 bytes and lane is 64 bytes.
  ValueType values[16];

  Tile() {
    // *(__m512*)&values = _mm512_set1_ps(0);
  }

  void SetZero() {
    for (int i = 0; i < 16; ++i) {
      values[i] = 0;
    }
  }

  void SetVectorZero() {
    *(__m512*)&values = _mm512_set1_ps(0.0);
  }

  // Insert a value to the tile at local position (r, l).
  void InsertValue(int tile_width, int r, int c, ValueType value) {
    values[tile_width * r + c] = value;
  }

  // Overloaded addition operator between two tiles.
  void operator+=(const Tile<ValueType>& t) {
    *(__m512*)values = _mm512_add_ps(*(__m512*)&values, *(__m512*)&t);
  }

  // Checks if all elements equal v.
  bool operator==(const ValueType& v) {
    for (int i = 0; i < 16; ++i) {
      if ((values)[i] != v) {
        return false;
      }
    }
    return true;
  }

  ValueType& operator[](int index) {
    return (values)[index];
  }
};

template <class ValueType> 
ValueType sum(const Tile<ValueType>& v) {
	ValueType ret = 0;
	for (int i = 0; i < 16; ++i) {
		ret += v.values[i];
	}

	return ret;
  // _mm512_reduce_add_ps(v.values);
}

template<class ValueType>
ostream& operator<<(ostream& os, const Tile<ValueType>& tile) {
  for (int i = 0; i < 16; ++i) {
    os << tile.values[i] << " ";
  } 
  return os;
}

struct Compare {
  bool operator()(const pair<int, int>& l, const pair<int, int>& r) const {
    return l.first == r.first && l.second == r.second;
  }
};

struct Hash {
  size_t operator()(const pair<int, int>& key) const {
    return key.first * 68371 + key.second;
    // return hash<int>()(key.first) ^ (hash<int>()(key.second) << 1);
  }
};

size_t hash(const pair<int, int>& key) {
  return key.first * 65536 + key.second;
}

// Util class used for tiling a CSR and constructing
// tiled csr from tile hash table.
template <class ValueType>
class TiledCsr {
 public:
  // Build a tiled csr from a file.
  // static
  static Csr<Tile<ValueType> >* TiledCsrFromFile(int w, int l, string csr_file) {
    Csr<ValueType>* csr = CsrLoader<ValueType>::Load(csr_file);
    // cout << "Load done." << endl;
    // cout << "NNZ: " << csr->nnz << endl;

    // Auxiliary map for storing temp tiles.
    unordered_map<pair<int, int>, Tile<ValueType>, Hash, Compare> tiles_map;  

    // Go through the csr and group nnzs into tiles.
    for (int i = 0; i < csr->num_rows; ++i) {
      for (int j = csr->rows[i]; j < csr->rows[i+1]; ++j) {
        if (csr->vals[j]) {
          // The key is the left up corner position at tile-level. 
          pair<int, int> position = make_pair<int, int>(i/l, csr->cols[j]/w);
          if (tiles_map.find(position) == tiles_map.end()) {
            tiles_map[position] = Tile<ValueType>();
            tiles_map[position].SetZero();
          }
          tiles_map[position].InsertValue(
               w, i%l, csr->cols[j]%w, csr->vals[j]);
        }
      }
    }

    // cout << "Group done." << endl;

    // Build csr for tiles.
    // Count nnz.
    int num_rows = csr->num_rows/w;
    int num_cols = csr->num_cols/w;
    int nnz = 0;
    vector<int> rows;
    vector<int> cols;
    vector<Tile<ValueType> > vals;

    // Build the csr index for the tiles.
    for (int i = 0; i < num_rows; ++i) {
      rows.push_back(cols.size());
      for (int j = 0; j < num_cols; ++j) {
        pair<int, int> up_left = make_pair<int, int>((int)i, (int)j);
        if (tiles_map.find(up_left) != tiles_map.end()) {
          ++nnz; 
          cols.push_back(j);
          vals.push_back(tiles_map[up_left]);
        }
      }
    }

    // csr is no longer used.
    delete csr;

    // Gen tiled csr, stride is tile-wide.
    // cout << "tiled nnz: " << nnz << endl;
    Csr<Tile<ValueType> >* tile_indices =
        new Csr<Tile<ValueType> >(num_rows, num_cols, nnz); 
    for (int i = 0; i < num_rows; ++i) {
      tile_indices->rows[i] = rows[i];
    }

    // Indicates the end.
    tile_indices->rows[num_rows] = nnz;

    // Fill col indices and vals.
    for (int i = 0; i < nnz; ++i) {
      tile_indices->cols[i] = cols[i];
      tile_indices->vals[i] = vals[i];
    }

    return tile_indices;
  }
};

#endif /* TILED_CSR_H */
