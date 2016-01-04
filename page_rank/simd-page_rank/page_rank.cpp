#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <sys/time.h>
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

void input(string filename) {
  ifstream fin(filename.c_str());
  string line;
  getline(fin, line);
  stringstream sin(line);
  sin >> nnodes >> nedges;

  for(int i=0;i<nnodes;i++) {
    grah.nneibor[i] = 0;
  }

  int cur = 0;
  while(getline(fin, line)) {
    int n1, n2;
    stringstream sin1(line);
    sin1 >> n1 >> n2;
    grah.n1[cur] = n1;
    grah.n2[cur] = n2;
    grah.nneibor[n1]++;
    cur++;
  }
  nedges = cur;
}

void input2(string filename, int tilesize) {
  ifstream fin(filename.c_str());
  string line;
  getline(fin, line);
  stringstream sin(line);
  sin >> nnodes >> nedges;

  for(int i=0;i<nnodes;i++) {
    grah.nneibor[i] = 0;
  }

  int cur = 0;
  while(getline(fin, line)) {
    int n, n1, n2;
    stringstream sin1(line);
    while(sin1 >> n) {
      grah.n1[cur] = n / tilesize;
      grah.n2[cur] = n % tilesize;
      cur++;
    }
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
    for(int j=0;j<nedges;j+=16) {
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
  char *filename = argv[1];
  cout << filename << endl;
  if(atoi(argv[2])==0)
    input(filename);
  else 
    input2(filename, atoi(argv[2]));
  page_rank();
  return 0;
}
