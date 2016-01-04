/*
 * sssp.cpp
 *
 *  Created on: 2015?1?5?
 *      Author: jiang
 *  Single Source Shortest Path: Bellman-Ford Algorithm
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cfloat>
#include <sstream>
#include <sys/time.h>
#include <vector>
#include <map>
#include <omp.h>
#include "sse_api.h"
using namespace std;

#define MAX_NODES 1700000
#define MAX_EDGES 100000000

struct Graph {
  __declspec(align(64)) int n1[MAX_EDGES];
  __declspec(align(64)) int n2[MAX_EDGES];
  __declspec(align(64)) float dis[MAX_EDGES];
};

int nnodes, nedges, source;
Graph grah;
float d[MAX_NODES];
vector<int> group;

struct timeval get_time() {
  struct timeval tt;
  gettimeofday(&tt, NULL);
  return tt;
}

void input(string filename, int tilesize) {
  ifstream fin(filename.c_str());
  string line;
  getline(fin, line);
  stringstream sin(line);
  sin >> nnodes >> nedges;
  int ntile = nnodes / tilesize;
  if(nnodes % tilesize) ntile++;

  int cur = 0;
  group.push_back(0);
  int tileid = 0;
  while(getline(fin, line)) {
    int n, n1, n2;
    stringstream sin1(line);
    while(sin1 >> n) {
      grah.n1[cur] = n / tilesize;
      grah.n2[cur] = n % tilesize;
      grah.dis[cur] = 1;
      cur++;
    }
    tileid++;
    if(tileid % ntile)group.push_back(cur);
  }
  nedges = cur;
  source = 0;
}

void print() {
  for(int i=0;i<nnodes;i++) {
    cout << d[i] << " ";
  }
  cout << endl;
}

int nthreads;
void bellman_ford() {

  for(int i=0;i<nnodes;i++) {
    d[i] = FLT_MAX;
  }

  d[source] = 0;
  for(int i=1;i<10;i++) {
    struct timeval start = get_time();
#pragma omp parallel for schedule(dynamic) num_threads(nthreads)
    for(int g=0;g<group.size()-1;g++) {
      for(int j=group[g];j<group[g+1];j+=16) {
        vint vn1, vn2;
        vn1.load(&(grah.n1[j]));
        vn2.load(&(grah.n2[j]));
        vfloat vd1, vd2, vdis;
        vd1.load(d, vn1, 4);
        vd2.load(d, vn2, 4);
        vdis.load(grah.dis);
        mask m = vd2 > (vd1 + vdis); 
        {
          Mask::set_mask(m, vd2);
          vd2.mask() = vd1.mask() + vdis.mask();
          vd2.mask().store(d, vn2);
        }
      }
    }
    struct timeval end = get_time();
    cout << (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0 << endl;
  }
}

int main(int argc, char *argv[]) {
  cout << argv[1] << endl;
  input(argv[1], atoi(argv[2]));
  nthreads = 2;
  cout << "number of threads: " << nthreads << endl;
  bellman_ford();
  nthreads = 4;
  cout << "number of threads: " << nthreads << endl;
  bellman_ford();
  nthreads = 8;
  cout << "number of threads: " << nthreads << endl;
  bellman_ford();
  nthreads = 16;
  cout << "number of threads: " << nthreads << endl;
  bellman_ford();
  nthreads = 32;
  cout << "number of threads: " << nthreads << endl;
  bellman_ford();
  nthreads = 40;
  cout << "number of threads: " << nthreads << endl;
  bellman_ford();
  nthreads = 50;
  cout << "number of threads: " << nthreads << endl;
  bellman_ford();
  nthreads = 61;
  cout << "number of threads: " << nthreads << endl;
  bellman_ford();
  nthreads = 80;
  cout << "number of threads: " << nthreads << endl;
  bellman_ford();
  nthreads = 122;
  cout << "number of threads: " << nthreads << endl;
  bellman_ford();
  nthreads = 150;
  cout << "number of threads: " << nthreads << endl;
  bellman_ford();
  nthreads = 180;
  cout << "number of threads: " << nthreads << endl;
  bellman_ford();
  nthreads = 200;
  cout << "number of threads: " << nthreads << endl;
  bellman_ford();
  nthreads = 244;
  cout << "number of threads: " << nthreads << endl;
  bellman_ford();

  return 0;
}
