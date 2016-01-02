#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <sys/time.h>
using namespace std;

#define DUMP 0.85
#define MAX_NODES 1700000
#define MAX_EDGES 40000000


struct timeval get_time()
{
	struct timeval tt;
	gettimeofday(&tt, NULL);
    return tt;
}

struct Graph
{
  int n1[MAX_EDGES];
  int n2[MAX_EDGES];
  int nneibor[MAX_NODES];
};

int nnodes, nedges;
Graph grah;
float rank[MAX_NODES];
float sum[MAX_NODES];



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


void page_rank()
{
  for(int i=0;i<nnodes;i++) {
    rank[i] = 1.0;
    sum[i] = 0.0;
  }

  for(int i=0;i<10;i++) {
    
   struct timeval start = get_time();

    for(int j=0;j<nedges;j++) {
      int n1 = grah.n1[j];
      int n2 = grah.n2[j];
      sum[n2] += rank[n1]/grah.nneibor[n1];
    }
    struct timeval end = get_time();
		cout << (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0 << endl;

    for(int j = 0; j < nnodes; j++)
    {
      rank[j] = (1 - DUMP) / nnodes + DUMP * sum[j]; 	
    }

  }
}

void print()
{

	for(int i=0;i<nnodes;i++) {
		cout << rank[i] << " ";
	}
	cout << endl;

}


int main(int argc, char *argv[])
{
  //string filename = "../../sssp/twitter-input/higgs-social_network-tuple.txt";
  char *filename = argv[1];
  cout << filename << endl;
  if(argc==2)
    input(filename);
  else
    input2(filename, 1024);
  page_rank();
//  print();
  return 0;
}
