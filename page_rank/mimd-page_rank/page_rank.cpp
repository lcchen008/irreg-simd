#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <sys/time.h>
#include "omp.h"
#include "sse_api.h"
using namespace std;

#define DUMP 0.85
#define MAX_NODES 1700000
#define MAX_EDGES 100000000

struct timeval get_time() {
  struct timeval tt;
  gettimeofday(&tt, NULL);
  return tt;
}

struct Graph {
  __declspec(align(64)) int n1[MAX_EDGES];
  __declspec(align(64)) int n2[MAX_EDGES];
  __declspec(align(64)) float nneibor[MAX_NODES];
};

int nnodes, nedges;
Graph grah;
__declspec(align(64)) float rank_val[MAX_NODES];
__declspec(align(64)) float sum[MAX_NODES];
vector<int> group;
int nthreads;

void input(string filename, int tilesize) {
  ifstream fin(filename.c_str());
  string line;
  getline(fin, line);
  stringstream sin(line);
  sin >> nnodes >> nedges;
  int ntile = nnodes / tilesize;
  if(nnodes % tilesize) ntile++;

  for(int i=0;i<nnodes;i++) {
    grah.nneibor[i] = 0;
  }

  int cur = 0;
  group.push_back(0);
  int tileid = 0;
  while(getline(fin, line)) {
    int n, n1, n2;
    stringstream sin1(line);
    while(sin1 >> n) {
      grah.n1[cur] = n / tilesize;
      grah.n2[cur] = n % tilesize;
      cur++;
    }
    tileid++;
    if(tileid % ntile)group.push_back(cur);
  }
  nedges = cur;
}

void page_rank() {
  for(int i=0;i<nnodes;i++) {
    rank_val[i] = 1.0;
    sum[i] = 0.0;
  }

  for(int i=0;i<10;i++) {
    struct timeval start = get_time();

#pragma omp parallel for schedule(dynamic) num_threads(nthreads)
    for(int g=0;g<group.size()-1;g++) {
      for(int j=group[g];j<group[g+1];j+=16) {
        vint vn1;
        vn1.load(&(grah.n1[j]));
        vint vn2;
        vn2.load(&(grah.n2[j]));
        vfloat vsum;
        vsum.load(sum, vn2, 4);
        vfloat vrank;
        vrank.load(rank_val, vn1, 4);
        vfloat vneibor;
        vneibor.load(grah.nneibor, vn1, 4);
        vsum += vrank / vneibor;
        vsum.store(sum, vn2, 4);
      }
    }
    struct timeval end = get_time();
    cout << (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0 << endl;

    for(int j = 0; j < nnodes; j++)
    {
      rank_val[j] = (1 - DUMP) / nnodes + DUMP * sum[j]; 	
    }
  }
}

void print() {
  for(int i=0;i<nnodes;i++) {
    cout << rank_val[i] << " ";
  }
  cout << endl;
}

int main(int argc, char *argv[]) {
  cout << argv[1] << endl;
  input(argv[1], atoi(argv[2]));
  nthreads = 2;
  cout << nthreads << endl;
  page_rank();
  nthreads = 4;
  cout << nthreads << endl;
  page_rank();
  nthreads = 8;
  cout << nthreads << endl;
  page_rank();
  nthreads = 16;
  cout << nthreads << endl;
  page_rank();
  nthreads = 32;
  cout << nthreads << endl;
  page_rank();
  nthreads = 40;
  cout << nthreads << endl;
  page_rank();
  nthreads = 50;
  cout << nthreads << endl;
  page_rank();
  nthreads = 61;
  cout << nthreads << endl;
  page_rank();
  nthreads = 80;
  cout << nthreads << endl;
  page_rank();
  nthreads = 122;
  cout << nthreads << endl;
  page_rank();
  nthreads = 150;
  cout << nthreads << endl;
  page_rank();
  nthreads = 180;
  cout << nthreads << endl;
  page_rank();
  nthreads = 200;
  cout << nthreads << endl;
  page_rank();
  nthreads = 244;
  cout << nthreads << endl;
  page_rank();
  return 0;
}
