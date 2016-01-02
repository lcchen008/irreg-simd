// Tool used for tiling and grouping and storing the
// results into disk. The results are used for SIMD
// processing. 

#include "csr_loader.h"
#include "ds.h"
#include "tiling.h"
#include "util.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

// Output a tile.
ofstream& operator<<(ofstream& os, const Coo<int>& tile) {
  ostringstream oss;
  for (int i = 0; i < tile.nnz; ++i) {
    oss << tile.rows[i]
        << " " 
        << tile.cols[i] 
        << " " 
        << tile.vals[i] 
        << " "; 
  }
  os << oss.str();
  return os;
}

ostringstream& operator<<(ostringstream& os, const Coo<int>& tile) {
  for (int i = 0; i < tile.nnz; ++i) {
    os << tile.rows[i]
       << " " 
       << tile.cols[i] 
       << " " 
       << tile.vals[i] 
       << " "; 
  }
  return os;
}

// Output multi-thread packs.
ofstream& operator<<(ofstream& os, const vector<Coo<int> >& tiles) {
  ostringstream oss;
  for (int i = 0; i < tiles.size(); ++i) {
    oss << tiles[i];
  }
  os << oss.str();
  return os;
}

// Output three level nnzs to an output stream.
ofstream& operator<<(ofstream& os, const vector<vector<vector<Coo<int> > > >& tiles) {
  for (int i = 0; i < tiles.size(); ++i) {
    for (int j = 0; j < tiles[i].size(); ++j) {
      os << tiles[i][j]; 
      os << endl;  // Each tile (before grouping) is stored in one row in the file.
    }
  }
  return os;
}

// Output two level nnzs to an output stream.
ofstream& operator<<(ofstream& os, const vector<vector<Coo<int> > >& tiles) {
  int count = 0;
  for (int i = 0; i < tiles.size(); ++i) {
    for (int j = 0; j < tiles[i].size(); ++j) {
      for (int k = 0; k < tiles[i][j].actual_nnz; ++k) {
        os << tiles[i][j].rows[k] << " " << tiles[i][j].cols[k] << " " << tiles[i][j].vals[k] << " ";
        ++count;
      }
      os << endl;  // Each tile (before grouping) is stored in one row in the file.
    }
  }
  cout << "Total actual nnz written: " << count << endl;
  return os;
}

void WriteOffsets(const vector<vector<vector<Coo<int> > > >& tiles, ofstream& os) {
  // Offset for each thread.
  int total = 0;
  ostringstream oss;
  for (int i = 0; i < tiles.size(); ++i) {
    // Offset for each parallel unit.
    for (int j = 0; j < tiles[i].size(); ++j) {
      oss << total << " "; 
      for (int k = 0; k < tiles[i][j].size(); ++k) {
        total += tiles[i][j][k].nnz;
      }
    }
    oss << endl;
  }
  os << total << endl;
  os << oss.str();
}

// First arg: matrix market formatted input matrix.
// Second arg: tile size.
int main(int argc, char** argv) {
  cout << "Processing file: " << argv[1] << endl;
  cout << "Tile size: " << argv[2] << endl;
  double before_load = rtclock();
  MyCsr<int>* inters = CsrLoader<int>::Load(string(argv[1]));
  double after_load = rtclock();
  cout << "Load time: " << after_load - before_load << endl;

  // File containing non-zeros.
  string nnz_file_name = string(argv[1]) + ".tiling." + argv[2];
  // File containing delemiters for separating tiles/multi-thread packs.
  string offset_file_name = string(argv[1]) + ".offset." + argv[2];
  // File containing non-zeros before inter-lane conflict removal (for seq
  // execution).
  string seq_nnz_file_name = string(argv[1]) + ".seq.tiling." + argv[2];

  //////////////// Tile the matrix. ///////////////
  double before_processing = rtclock();
  // First extract the dense tiles.
  vector<Coo<int> >* peel_dense = Tiling<int>::Peel(inters, atoi(argv[2]), 0);

  cout << "Peel done." << endl;
  // Make sure that inter-thread (inter-tile) execution has no conflict. 
  vector<vector<Coo<int> > >* diagonal_pack_tiles =
      Tiling<int>::KillTilesConflicts(peel_dense);
  cout << "Kill tile conflict done." << endl;

  // Make sure that the inter-lane (inter-nnz) execution in each thread has no
  // conflict. 
  vector<vector<vector<Coo<int> > > >* multi_thread_tiles = 
      RemoveConflictAndPack(diagonal_pack_tiles);
  cout << "Remove SIMD conflict done." << endl;

  // Tile the remaining tiles, using a twice larger tile size.
  // TODO(): take care of the tiling size.
  vector<Coo<int> >* peel_sparse = Tiling<int>::Peel(inters, atoi(argv[2]), 0);
  // Make sure that inter-thread (inter-tile) execution has no conflict. 
  vector<vector<Coo<int> > >* diagonal_pack_tiles_sparse =
      Tiling<int>::KillTilesConflicts(peel_sparse);
  // Make sure that the inter-lane (inter-nnz) execution in each thread has no
  // conflict. 
  vector<vector<vector<Coo<int> > > >* multi_thread_tiles_sparse = 
      RemoveConflictAndPack(diagonal_pack_tiles_sparse);
  // Combine the dense and sparse tiles.
  for (int i = 0; i < multi_thread_tiles_sparse->size(); ++i) {
    multi_thread_tiles->push_back((*multi_thread_tiles_sparse)[i]);
  }
  CountSimdRate(*multi_thread_tiles);

  // Combine the tiles (before removing lane conflicts)
  for (int i = 0; i < diagonal_pack_tiles_sparse->size(); ++i) {
    diagonal_pack_tiles->push_back((*diagonal_pack_tiles_sparse)[i]);
  }
  double after_processing = rtclock();

  cout << "Sync steps: " << diagonal_pack_tiles->size() << endl;
  cout << "Max parallelism: " << ComputeMaxParallelism(diagonal_pack_tiles) << endl;
  cout << "Min parallelism: " << ComputeMinParallelism(diagonal_pack_tiles) << endl;
  cout << "Average parallelism: " << ComputeAverageParallelism(diagonal_pack_tiles) << endl;

  //////////////// Tile the matrix. ///////////////
  
  cout << "Tiling time: " << after_processing - before_processing << endl;

  // Write to file.
  ofstream nnz_output(nnz_file_name.c_str());
  ofstream offset_output(offset_file_name.c_str());
  ofstream seq_nnz_seq_output(seq_nnz_file_name.c_str());

  double before_write_nnz = rtclock();
  // First, write nnz file.
  nnz_output << *multi_thread_tiles;
  double after_write_nnz = rtclock();

  cout << "Write nnz time: " << after_write_nnz - before_write_nnz << endl;

  // Then, write offsets.
  double before_write_offset = rtclock();
  WriteOffsets(*multi_thread_tiles, offset_output);
  double after_write_offset = rtclock();

  cout << "Write offset time: " << after_write_offset - before_write_offset << endl;

  // Then, write seq tiling file.
  double before_write_seq = rtclock();
  seq_nnz_seq_output << *diagonal_pack_tiles;
  double after_write_seq = rtclock();

  cout << "Write seq tiling time: " << after_write_seq - before_write_seq << endl;

  FreeTileVector(peel_dense);
  FreeTileVector(peel_sparse);
  FreeTileVector(multi_thread_tiles);

  delete peel_dense;
  delete diagonal_pack_tiles;
  delete multi_thread_tiles;
  delete peel_sparse;
  delete diagonal_pack_tiles_sparse;
  delete multi_thread_tiles_sparse;
  nnz_output.close();
  offset_output.close();
  seq_nnz_seq_output.close();
  cout << "Done." << endl << endl;
  return 0;
}
