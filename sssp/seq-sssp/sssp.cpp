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
#include <vector>
#include <stdlib.h>
#include <map>
#include <sys/time.h>
using namespace std;


#define MAX_NODES 1700000
#define MAX_EDGES 100000000


struct Graph {
  int n1[MAX_EDGES];
  int n2[MAX_EDGES];
  float dis[MAX_EDGES];
};

int nnodes, nedges, source;
Graph grah;
float d[MAX_NODES];


struct timeval get_time()
{
  struct timeval tt;
  gettimeofday(&tt, NULL);
  return tt;
}

/* the first line of the input is the number of nodes and the number of edges
 * the following lines are 3-element tuple indicating node1, node2 and their distance
 * the last line is the source node for single source shortest path.
 * */
void input(string filename) {
  ifstream fin(filename.c_str());
  string line;
  getline(fin, line);
  stringstream sin(line);
  sin >> nnodes >> nedges;

  int cur = 0;
  while(getline(fin, line)) {
    int n1, n2;
    float dis;
    stringstream sin1(line);
    sin1 >> n1 >> n2;
    //cout << n1 << " " << n2 << " " << dis << endl;
    grah.n1[cur] = n1;
    grah.n2[cur] = n2;
    grah.dis[cur] = 1;
    cur++;
  }
  nedges = cur;
  source = 0;
}

void input2(string filename, int tilesize) {
  ifstream fin(filename.c_str());
  string line;
  getline(fin, line);
  stringstream sin(line);
  sin >> nnodes >> nedges;

  int cur = 0;
  while(getline(fin, line)) {
    int n, n1, n2;
    stringstream sin1(line);
    while(sin1 >> n) {
      grah.n1[cur] = n / tilesize;
      grah.n2[cur] = n % tilesize;
      grah.dis[cur] = 1;
      cur++;
    }
  }
  nedges = cur;
  source = 0;
}


void print()
{
  for(int i=0;i<nnodes;i++) {
    cout << d[i] << " ";
  }
  cout << endl;
}

void bellman_ford() {
  for(int i=0;i<nnodes;i++) {
    d[i] = FLT_MAX;
  }
  d[source] = 0;
  for(int i=1;i<10;i++) {
    struct timeval start = get_time();
    for(int j=0;j<nedges;j++) {
      int n1 = grah.n1[j];
      int n2 = grah.n2[j];
      float d1 = d[n1];
      float d2 = d[n2];
      float w = grah.dis[j];
      if(d2 > d1 + w) {
        d[n2] = d1 + w;
      }
    }

    struct timeval end = get_time();
    cout << (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0 << endl;
  }
}

int main(int argc, char *argv[])
{
  char *filename = argv[1];
  cout << filename <<endl;
  if(argc==2)
    input(filename);
  else 
    input2(filename, 1024);
  bellman_ford();

  return 0;
}
