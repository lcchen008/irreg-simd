#ifndef TILING
#define TILING

// Author @linchuan.

// Util to tile and peel dense tiles from a sparse matrix.
// The util is able to tile and peel tiles with any density
// specified by users, and leave the unpeeled tiles in the
// original matrix.
#include "csr.h"
//#include "csr_loader.h"
#include "ds.h"

#include <iostream>
#include <climits>
#include <cmath>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using namespace std;

#define RED "\033[31m"
#define BLUE "\033[34m" 
#define RESET "\033[0m"

// Customized hash and equal functions for tile hash table.
struct PairHash {
	size_t operator()(const pair<int, int>& p) const {
		return p.first * 43876931 + p.second;
	}
};

struct PairEqual {
	bool operator()(const pair<int, int>& a, const pair<int, int>& b) const {
		return a.first == b.first && a.second == b.second;
	}
};

template <class ValueType>
class Tiling {
 public:
	// Peels off width*width dense squares from matrix. The
	// squares should contain at least nnz_threshold nnzs.
	// This function changes matrix, leaving only the unpeeled
	// elements.
	// The returned value is the array containing the peeled dense squares,
	// each stored as a Coo object.
  	// static
	static vector<Coo<ValueType> >* Peel(MyCsr<ValueType>* matrix, int width, int nnz_threshold) {
		// Hash table for tiling the input matrix. Key is the position of the tile
		// in the hyper-matrix (treating each tile as one element). Value is the
		// vector containing absolute coordinates in original matrix and the value.
		unordered_map<pair<int, int>, vector<tuple<int, int, ValueType> >, PairHash, PairEqual> tile_hash_table;
		// Tile the whole matrix into tiles.
		for (int i = 0; i < matrix->num_rows; ++i) {
			for (int j = matrix->rows[i]; j < matrix->rows[i+1]; ++j) {
				int hyper_row_id = i/width;				
				int hyper_col_id = matrix->cols[j]/width;
				pair<int, int> key = make_pair(hyper_row_id, hyper_col_id);
				if (tile_hash_table.find(key) == tile_hash_table.end()) {
					tile_hash_table[key] = vector<tuple<int, int, ValueType> >();
				}
				tile_hash_table[key].push_back(make_tuple(i, matrix->cols[j], matrix->vals[j]));
			}
		}

		// Clear the input matrix and then refill it with the unpeeled tiles.
		matrix->nnz = 0;
		matrix->rows.clear();
		matrix->cols.clear();
		matrix->vals.clear();

		vector<vector<int> > unpeeled_cols(matrix->num_rows);
		vector<vector<ValueType> > unpeeled_vals(matrix->num_rows);

		// Construct the peeled dense tiles.
		vector<Coo<ValueType> >* dense_tiles = new vector<Coo<ValueType> >;
		int hyper_num_rows = ceil((float)matrix->num_rows/width);
		int hyper_num_cols = ceil((float)matrix->num_cols/width);
		for (int i = 0; i < hyper_num_rows; ++i) {
			for (int j = 0; j < hyper_num_cols; ++j) {
				pair<int, int> key = make_pair(i, j);
				if (tile_hash_table.find(key) != tile_hash_table.end()) {
					if (tile_hash_table[key].size() >= nnz_threshold) {
						// Make a Coo object, and insert it to the return value.
						dense_tiles->emplace_back(tile_hash_table[key]);
            			dense_tiles->back().width = width;
					} else {  // Else restore it into the unpeeled Csr matrix.
						for (const auto& t : tile_hash_table[key]) {
							int row = get<0>(t); 
							int col = get<1>(t);
							ValueType val = get<2>(t);
							unpeeled_cols[row].push_back(col);
							unpeeled_vals[row].push_back(val);
						}
					}
				}
			}
		}

		for (int i = 0; i < unpeeled_cols.size(); ++i) {
			matrix->rows.push_back(matrix->cols.size());
			for (int j = 0; j < unpeeled_cols[i].size(); ++j) {
				matrix->cols.push_back(unpeeled_cols[i][j]);		
				matrix->vals.push_back(unpeeled_vals[i][j]);
			}
		}

		matrix->nnz = matrix->cols.size();
		matrix->rows.push_back(matrix->nnz);

		return dense_tiles;
	}

  /*
  // static
  // Transforms a tile to diagonal order. The longer diagonals appear
  // before shorter diagonals.
  template <class ValueType>
  static void TransformToDiagonalOrder(Coo<ValueType>* tile) {
    // Pack to different diagonal arrays.
    int base = tile->width - 1; // base is the diagonal id of longest diagonal.
    int total_diagonal = base*2 + 1;
    vector<vector<tuple<int, int, ValueType> > > diagonals(total_diagonal);

    for (int i = 0; i < tile->actual_nnz; ++i) {
      int row = tile->rows[i]%tile->width;
      int col = tile->cols[i]%tile->width;
      ValueType val = tile->vals[i];
      int diagonal_id = col - row;
      if ((diagonal_id + base) >= total_diagonal) {
        cout << "row: " << row << " col: " << col << endl;
      }
      diagonals[diagonal_id + base].emplace_back(tile->rows[i],
                                                 tile->cols[i], val);
    }

    // Pack to Coo again.
    vector<tuple<int, int, ValueType> > diagonal_order;

    // Begin from longest diagonal (as center diagonal) to upper and lower.
    for (int i = 0; i < diagonals[base].size(); ++i) {
      diagonal_order.push_back(diagonals[base][i]);
    }
    for (int i = 1; i <= base; ++i) {
      for (int j = 0; j < diagonals[base+i].size(); ++j) {
        diagonal_order.push_back(diagonals[base+i][j]);
      }
      for (int j = 0; j < diagonals[base-i].size(); ++j) {
        diagonal_order.push_back(diagonals[base-i][j]);
      }
    }

    tile->Refill(diagonal_order);
  }
  */

  // static
  // (grouping)
  // Transforms a Coo tile to a vector of compact Coo tiles,
  // each containing non-zeros without write confilict in both
  // row and column major.
  static vector<Coo<ValueType> >* KillConflicts(Coo<ValueType>* tile) {
    // TransformToDiagonalOrder(tile);
    // Used for checking compatibility.
    vector<unordered_set<int> > row_ids;
    vector<unordered_set<int> > col_ids;
    vector<vector<tuple<int, int, ValueType> > > nnzs;

    // For each nnz in tile.
    for (int i = 0; i < tile->actual_nnz; ++i) {
      int row = tile->rows[i];
      int col = tile->cols[i];
      ValueType val = tile->vals[i]; 
      // Find a new tile which the current nnz is compactible with.
      int j = 0;
      for (; j < row_ids.size(); ++j) {
        if (row_ids[j].size() < 16 && row_ids[j].find(row) == row_ids[j].end() 
            && col_ids[j].find(col) == col_ids[j].end()) {
          break;
        }
      }
      if (j == row_ids.size()) {
        row_ids.emplace_back();
        col_ids.emplace_back();
        nnzs.emplace_back();
      }
      row_ids[j].insert(row);
      col_ids[j].insert(col);
      nnzs[j].emplace_back(row, col, val);
    }

    // Pack as Coo tiles and return.
    vector<Coo<ValueType> >* ret = new vector<Coo<ValueType> >;
    for (int i = 0; i < nnzs.size(); ++i) {
      ret->emplace_back(nnzs[i]);

      /*
      Coo<ValueType> tmp(nnzs[i]);
      ret->push_back(tmp);
      tmp.TransferOwnership();
      */
    }

    return ret;
  }

  // Remove conflicts between tiles. Used for multi-thread execution. 
  // static
  template <class ValueType>
  static vector<vector<Coo<ValueType> > >* KillTilesConflicts(vector<Coo<ValueType> >* tiles) {
    vector<vector<Coo<ValueType> > > *non_conflict_groups = new vector<vector<Coo<ValueType> > >;
    if (!tiles->size()) { return non_conflict_groups; }
    vector<unordered_set<int> > row_ids;
    vector<unordered_set<int> > col_ids;

    int width = (*tiles)[0].width; 
    for (int i = 0; i < tiles->size(); ++i) {
      if (!(*tiles)[i].actual_nnz) {
        continue;
      }
      int tile_row = (*tiles)[i].rows[0]/width;
      int tile_col = (*tiles)[i].cols[0]/width;
      int j = 0;
      for (; j < row_ids.size(); ++j) {
        // Find one.
        if (row_ids[j].size() < 488 && row_ids[j].find(tile_row) == row_ids[j].end() &&
            col_ids[j].find(tile_col) == col_ids[j].end()) {
          break;
        }
      }
      // Could not find.
      if (j == row_ids.size()) {
        row_ids.emplace_back();
        col_ids.emplace_back();
        non_conflict_groups->emplace_back();
      }
      row_ids[j].insert(tile_row);
      col_ids[j].insert(tile_col);
      (*non_conflict_groups)[j].push_back((*tiles)[i]);
      //(*tiles)[i].TransferOwnership();
    }
    return non_conflict_groups;
  }
};

// Removes conflicts among the nnzs.
vector<Coo<int> >* RemoveConflict(vector<Coo<int> >* tiles) {
  vector<Coo<int> >* ret = new vector<Coo<int> >;
  for (int i = 0; i < tiles->size(); ++i) {
    vector<Coo<int> >* killed_conflict = Tiling<int>::KillConflicts(&(*tiles)[i]);
    for (int j = 0; j < killed_conflict->size(); ++j) {
      ret->push_back((*killed_conflict)[j]);
    }
  }
  return ret;
}

// This version is used for multi-thread execution.
// diagonals is the diagonals containing tiles.
// The return value is a three-level vector,
// the inner-most level executed by ONE thread each time.
// The input is a two-dimensional vector containing parallel
// units, each unit containing conflict-free tiles. Each tile
// is not conflict free, and this function is to further group
// each tile so that each sub-group is conflict-free among
// SIMD lanes.
vector<vector<vector<Coo<int> > > >* RemoveConflictAndPack(vector<vector<Coo<int> > >* diagonals) {
  vector<vector<vector<Coo<int> > > >* ret = new vector<vector<vector<Coo<int> > > >;
  for (int i = 0; i < diagonals->size(); ++i) {
    ret->emplace_back();
    for (int j = 0; j < (*diagonals)[i].size(); ++j) {
      vector<Coo<int> >* no_conflict = Tiling<int>::KillConflicts(&(*diagonals)[i][j]);
      ret->back().emplace_back(*no_conflict);
      // ret->back().emplace_back();
      // for (int k = 0; k < no_conflict->size(); ++k) {
      //   ret->back().back().push_back((*no_conflict)[k]);
      //   (*no_conflict)[k].TransferOwnership();
      // }
    }
  }
  return ret;
}

template <class ValueType>
inline int AverageParallelism(const vector<vector<vector<Coo<ValueType> > > >* diagonals) {
  int total_par = 0;
  for (int i = 0; i < diagonals->size(); ++i) {
    total_par += ((*diagonals)[i].size());
  }
  return total_par/diagonals->size();
}

// Computes how many tiles are there in each tile group from offset info.
inline float ComputeAverageParallelism(const vector<vector<int> >& offsets) {
	int total = 0;
	for (int i = 0; i < offsets.size(); ++i) {
		total += offsets[i].size();
	}
	return (float)(total/offsets.size()); 
}

// Computes max num. of tiles from offset info.
inline int ComputeMaxParallelism(const vector<vector<int> >& offsets) {
	int m = 0;
	for (int i = 0; i < offsets.size(); ++i) {
		m = max(m, (int)offsets[i].size());
	}
	return m; 
}

// Computes min num. of tiles from offset info.
inline int ComputeMinParallelism(const vector<vector<int> >& offsets) {
	int m = INT_MAX;
	for (int i = 0; i < offsets.size(); ++i) {
		m = min(m, (int)offsets[i].size());
	}
	return m; 
}

// Computes average num of tiles in each tile group from tiles info.
inline float ComputeAverageParallelism(const vector<vector<Coo<int> > >* tiles) {
	int total = 0;
	for (int i = 0; i < tiles->size(); ++i) {
		total += (*tiles)[i].size();
	}
	return (float)(total/tiles->size()); 
}

inline float ComputeMaxParallelism(const vector<vector<Coo<int> > >* tiles) {
	int m = 0;
	for (int i = 0; i < tiles->size(); ++i) {
		m = max(m, (int)(*tiles)[i].size());
	}
	return m; 
}

inline float ComputeMinParallelism(const vector<vector<Coo<int> > >* tiles) {
	int m = INT_MAX;
	for (int i = 0; i < tiles->size(); ++i) {
		m = min(m, (int)(*tiles)[i].size());
	}
	return m; 
}


template <class ValueType>
void CountSimdRate(const vector<vector<vector<Coo<ValueType> > > >&  diagonal_pack_tiles) {
  int t_actual_nnz = 0;
  int t_nnz = 0;
  for (int i = 0; i < diagonal_pack_tiles.size(); ++i) {
    for (int j = 0; j < diagonal_pack_tiles[i].size(); ++j) {
    	for (int k = 0; k < diagonal_pack_tiles[i][j].size(); ++k) {
      		t_actual_nnz += diagonal_pack_tiles[i][j][k].actual_nnz;
      		t_nnz += diagonal_pack_tiles[i][j][k].nnz;
    	}
  	}
  }

  cout << "total actual nnz: " << t_actual_nnz << endl;
  cout << "total nnz: " << t_nnz << endl;
  cout << RED << "[****Result****] ========> SIMD Utilization Ratio: " << (float)t_actual_nnz/t_nnz << RESET << endl;
}

template <class ValueType>
void FreeTileVector(vector<Coo<ValueType> >* tiles) {
	for (auto v : *tiles) {
		v.Free();
	}
}

template <class ValueType>
void FreeTileVector(vector<vector<Coo<ValueType> > >* tiles) {
	for (int i = 0; i < tiles->size(); ++i) {
		for (int j = 0; j < (*tiles)[i].size(); ++j) {
			(*tiles)[i][j].Free();
		}
	}
}

template <class ValueType>
void FreeTileVector(vector<vector<vector<Coo<ValueType> > > >* tiles) {
	for (int i = 0; i < tiles->size(); ++i) {
		for (int j = 0; j < (*tiles)[i].size(); ++j) {
			for (int k = 0; k < (*tiles)[i][j].size(); ++k) {
				(*tiles)[i][j][k].Free();
			}
		}
	}
}

#endif
