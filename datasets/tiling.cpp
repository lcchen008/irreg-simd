#include "tiling.h"
#include <stdlib.h>

using namespace std;


int main(int argc, char *argv[]) 
{
  char filename[200];
  Graph g;
  g.input(argv[1]);
  for(int tilesize=1024; tilesize<=8192; tilesize*=2) {
    g.tiling(tilesize);
    sprintf(filename, "%s-%d", argv[1], tilesize);
    g.output_tiling_blocks(filename);
  }

  return 0;

}
