#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <set>

using namespace std;

#define MAX_NODES 1700000
#define MAX_EDGES 40000000

int tile_width;

class group {
  set<int> n1;
  set<int> n2;
  int num;

 public:
  vector<int> edges;
  group(const int e) {
    n1.insert(e / tile_width);
    n2.insert(e % tile_width);
    edges.push_back(e);
    num = 0;
  }
  bool isConflictWith(int e) {
    return n2.find(e % tile_width) != n2.end() || n1.find(e / tile_width)!=n1.end();
  }
  void addEdge(const int &e) {
    n1.insert(e / tile_width);
    n2.insert(e % tile_width);
    num++;
    edges.push_back(e);
  }
  void print(ofstream &fout) {
    for(int j=0;j<edges.size();j++) {
      fout << edges[j] << " ";
    }
  }
};


class Graph {

  int node1[MAX_EDGES];
  int node2[MAX_EDGES];

  int nnodes, nedges, nsize;

  map<int, vector<int> > blocks;

 public:

  void input(string filename) {
    ifstream fin(filename.c_str());
    
    char outfile[200];
    sprintf(outfile, "%s-orig", filename.c_str());
    ofstream fout(outfile);

    string line;
    getline(fin, line);
    stringstream sin(line);
    sin >> nnodes >> nedges;
    fout << nnodes << " " << nedges;

    int cur = 0;
    while(getline(fin, line)) {
      int n1, n2;
      float dis;
      stringstream sin1(line);
      sin1 >> n1 >> n2 >> dis;
      node1[cur] = n1-1;
      node2[cur] = n2-1;
      fout << n1-1 << " " << n2-1 << endl;
      cur++;
    }
  }

  void grouping(vector<int> &block)
  {
    vector<group> groups;
    for(int i=0;i<block.size();i++) {
      int flag = 1;
      for(int j=0;j<groups.size();j++) {
        if(!groups[j].isConflictWith(block[i])){
          groups[j].addEdge(block[i]);
          flag = 0;
          break;
        } 
      }

      if(flag) {
        group g(block[i]);
        groups.push_back(g);
      }
    }


    block.clear();
    for(int k=0;k<groups.size();k++) {
      vector<int> edges = groups[k].edges;
      for(int p=0;p<edges.size();p++) {
        int index = edges[p];
        block.push_back(index);
      }
      if(edges.size() % 16 != 0) {
        for(int q=0; q<16-edges.size()%16;q++) {
          int index = edges[0];
          block.push_back(index);
        }
      }
    }
  }


  void tiling(int tw)
  {
    tile_width = tw;
    nsize = nnodes / tile_width;
    if(nnodes % tile_width) ++nsize;

    cout << "tile size: " << tile_width << endl;


    for(int i=0;i<nedges;++i) {
      int row = *(node1+i);
      int col = *(node2+i);
      int nrow = (row) / tile_width;
      int ncol = (col) / tile_width;
      row %= tile_width;
      col %= tile_width;
      int index = row * tile_width + col;
      blocks[nrow*nsize+ncol].push_back(index);
    }


    for(int j=0;j<nsize;j++) {
      for(int i=0;i<nsize;i++) {
        int index = i*nsize+j;
        int bsize = blocks[index].size();
        if(bsize) {
          grouping(blocks[index]);
        }
      }
    }


  }

  void output_tiling_blocks(string filename) {
    ofstream fout(filename.c_str());

    fout << nnodes << " " << nedges << endl;


    for(int j=0;j<nsize;j++) {
      for(int i=0;i<nsize;i++) {
        ostringstream out;
        int index = i*nsize+j;
        int count = 0;
        for(int k=0;k<blocks[index].size();k++) {
          out << blocks[index][k] << " ";
        }
        fout << out.str() << endl;
        blocks[index].clear();
      }
    }

  }

};
