
/************************************************************
  !File     : gen.c
  !Written by Hwansoo Han
 ************************************************************/

#define  BOXSIZ	        50	
#define  NUM_PARTICLES  4*BOXSIZ*BOXSIZ*BOXSIZ
#define  MAXINTERACT	NUM_PARTICLES * 150 
#define  DENSITY        0.83134
#define  TOLERANCE	1.2

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "misc.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

double  v [NUM_PARTICLES][3];  /* x,y,z coordinates of each molecule */
int     inter [MAXINTERACT][2]; /* (inter1, inter2) interacting pairs */

#define x(i)	(v[(i)][0])
#define y(i)	(v[(i)][1])
#define z(i)	(v[(i)][2])

#define inter1(i) (inter[(i)][0])
#define inter2(i) (inter[(i)][1])

double   side;                  /*  length of side of box                 */
double   sideHalf;              /*  1/2 of side                           */
double   cutoffRadius;          /*  cuttoff distance for interactions     */
double   perturb;               /*  perturbs initial coordinates          */
int      ninter;                /*  number of interacting molecules pairs */
int	 numMoles;
int 	 boxSize;
int      continuous = 0;
int      randomize = 1;

#ifdef BLOCK_ADAPTIVE
void RebuildInteraction(double (*)[3], int, double, double);
int  sort_edgelist( int, int, int (*)[2]);
#endif
void InitCoordinates();
#ifdef RCB
void InitRcb();
#endif
#ifdef ZORD
void InitZord();
#endif

main(int argc, char **argv)
{
  int    i, j, prev;
  double cutoffSquare, xx, yy, zz, rd;
  char   filename[30]; 
  char   filename2[30]; 
  char   full_path[80];
  char   full_path2[80];
  FILE   *fp;

  /* read input */
  scanf("%d", &boxSize);
  scanf("%lf", &cutoffRadius);
  scanf("%s", full_path);
  strcat(full_path, "/");
  sprintf(full_path2, "%s", full_path);

  struct stat st = {0};

  if (stat(full_path, &st) == -1) {
    mkdir(full_path, 0700);
  }

  if (argc > 1) {
    if (argv[1][0] == 'n') continuous = 0;
    else if (argv[1][0] == 'r') randomize = 1;
  }
  if (continuous)
    printf("******* Continous space assumed ********\n");

  /* set parameters */
  numMoles = 4 * boxSize * boxSize * boxSize;
  side   = pow( ((double)(numMoles)/DENSITY), 0.3333333);
  sideHalf  = side * 0.5 ;
  perturb  = side/ (double)boxSize;

  printf("numMoles= %d  cutoffRadius= %.2lf  side= %.2lf  perturb= %.2lf\n", 
         numMoles, cutoffRadius, side, perturb); 

  if (numMoles > NUM_PARTICLES) 
    printf ("numMoles exceeds MAX(%d)\n", NUM_PARTICLES);

  InitCoordinates();
  if (randomize) RandomizeCoordinates(numMoles);

#ifdef BUILD_LIST
  /*........................................................*/
  printf("Build Neighbours ...\n");

#ifdef ORG_ADAPTIVE_CODE
  cutoffSquare = (cutoffRadius * TOLERANCE)*(cutoffRadius * TOLERANCE);
  ninter = 0;

  for ( i=0; i<numMoles; i++) {
    for ( j = i+1; j<numMoles; j++ ) {

      xx = x(i) - x(j);
      yy = y(i) - y(j);
      zz = z(i) - z(j);

      if (continuous) {
        if ( xx < -sideHalf ) xx += side ;
        if ( yy < -sideHalf ) yy += side ;
        if ( zz < -sideHalf ) zz += side ;
        if ( xx >  sideHalf ) xx -= side ;
        if ( yy >  sideHalf ) yy -= side ;
        if ( zz >  sideHalf ) zz -= side ;
      }

      rd = xx*xx + yy*yy + zz*zz ;

      if ( rd <= cutoffSquare) {
        inter1 (ninter) = i;
        inter2 (ninter) = j;
        ninter ++;

        if ( ninter >= MAXINTERACT - 10 )
          printf("ninter = %d \n", ninter); 

        if ( ninter >= MAXINTERACT) {
          perror("MAXINTERACT limit");
          printf("(i j) = (%d %d)\n", i, j);
          exit(0);
        }
      }
    }
  }
#else /* BLOCK_ADAPTIVE */

  RebuildInteraction(v, numMoles, cutoffRadius, side);

#endif ORG_ADAPTIVE_CODE

  sprintf(filename, "%d-%.1lf", boxSize, cutoffRadius);
  if (continuous) strcat(filename, "c");
  if (randomize) strcat(filename, "r");

  strcat(filename, ".mesh");
  strcat(full_path, filename);

  if ( !(fp = fopen(full_path, "w")) ) {
    printf ("File open failed - %s\n", full_path);
    exit(1);
  }

  /*
   *   Node numbering starts from 1 to numMoles 
   *   [1...numMoles]
   */
  printf ("Writing interaction list ...\n"); 
  fprintf(fp, "%d  %d\n", numMoles, ninter);
  i = j = 0;
  while (j < numMoles) {     
    if (j == inter1(i)) {
      fprintf(fp, "%d ", inter2(i) +1);
      i++;
    }
    else {
      fprintf(fp, "\n");
      j++;
    }
  }
  fclose(fp);
#endif BUILD_LIST

#ifdef COORD
  /*
   * Now write out coordinates
   */
  printf ("Writing Coordinates ...\n"); 
  sprintf(filename2, "%d-%.1lf", boxSize, cutoffRadius);
  if (continuous) strcat(filename2, "c");
  if (randomize) strcat(filename2, "r");
  strcat(filename2, ".xyz");
  strcat(full_path2, filename2);
  if ( !(fp = fopen(full_path2, "w")) ) {
    printf ("File open failed - %s\n", filename2);
    exit(1);
  }
  for (i = 0; i < numMoles; i++) {
    fprintf (fp, "%lg %lg %lg\n", x(i), y(i), z(i));
  }
  fclose(fp);
#endif COORD
}

/*
   !============================================================================
   !  Function : InitCoordinates()
   !  Purpose  :
   !     Initialises the coordinates of the molecules by 
   !     distribuuting them uniformly over the entire box
   !     with slight perturbations.
   !============================================================================
   */

void InitCoordinates()
{
  int siz = boxSize, siz_3;
  int n, k,  j, i, npoints;

  printf("Init Coordinates ...\n");
  siz_3 = siz * siz * siz;
  npoints = siz_3 ; 
  for ( n =0; n< npoints; n++) {
    k   = n % siz ;
    j   = (int)((n-k)/siz) % siz;
    i   = (int)((n - k - j*siz)/(siz*siz)) % siz ; 

    x(n) = i*perturb ;
    y(n) = j*perturb ;
    z(n) = k*perturb ;

    x(n+npoints) = i*perturb + perturb * 0.5 ;
    y(n+npoints) = j*perturb + perturb * 0.5;
    z(n+npoints) = k*perturb ;

    x(n+npoints*2) = i*perturb + perturb * 0.5 ;
    y(n+npoints*2) = j*perturb ;
    z(n+npoints*2) = k*perturb + perturb * 0.5;

    x(n+npoints*3) = i*perturb ;
    y(n+npoints*3) = j*perturb + perturb * 0.5 ;
    z(n+npoints*3) = k*perturb + perturb * 0.5;
  }

} 


int RandomizeCoordinates(int ntimes)
{
  int i, a, b, n= numMoles; 
  double t;

  printf("Randomly Swap Coordinates ... %d\n", ntimes);
  for (i = 0; i < ntimes; i++) {
    a = random() % n;
    b = random() % n;

    /* swap two positions */
    t = x(a); x(a) = x(b); x(b) = t;
    t = y(a); y(a) = y(b); y(b) = t;
    t = z(a); z(a) = z(b); z(b) = t;
  }
  return 0;
}

#ifdef STATS
print_stats()
{
  int nproc = 8;
  int i, p1, p2, cut_edge = 0;
  int chunk = numMoles / nproc;

  for (i=0; i<ninter; i++)     {
    p1 = inter1(i) / chunk;
    p2 = inter2(i) / chunk;

    if (p1 != p2) cut_edge++;
  }

  printf("cut-edges = %d (%.1f%%), total-edges = %d\n", 
         cut_edge, (double)cut_edge/(double)ninter*100, ninter);
}
#endif STATS

sim_on(){}
sim_off(){}
sim_stats(){}


#ifdef BLOCK_ADAPTIVE

#define xyz2n(x, y, z) ((z) * (xd * yd) + (y) * xd + (x)) 

void
find_neighbor_blk(int x, int y, int z, int xd, int yd, int zd, int *neiblk) 
{
  int i, j, k, n = 0; 
  int x1[3], y1[3], z1[3];

  x1[0] = (x == 0)? xd-1 : x-1;    
  x1[1] = x;
  x1[2] = (x == xd-1)? 0 : x+1;    
  y1[0] = (y == 0)? yd-1 : y-1;    
  y1[1] = y;
  y1[2] = (y == yd-1)? 0 : y+1;    
  z1[0] = (z == 0)? zd-1 : z-1;    
  z1[1] = z;
  z1[2] = (z == zd-1)? 0 : z+1;    

  for (i=0; i<3; i++)
    for (j=0; j<3; j++)
      for (k=0; k<3; k++) {
        neiblk[n++] = xyz2n (x1[i], y1[j], z1[k]);    
      }
}

void
RebuildInteraction(double (*xyz)[3], int n_nodes, 
                   double cutoffRadius, double side)
{
  int *nodes, *start, n_xyz[3], neiblk[27], xd,yd,zd, x,y,z;
  int i, j, k, l, m, ii, jj, n_blk;
  double sideHalf, cutoffSquare, rd, xx, yy, zz;

  printf("cutoff=%.2lf side=%lf\n", cutoffRadius, side);

  sideHalf = side * 0.5;
  cutoffSquare = (cutoffRadius * TOLERANCE)*(cutoffRadius * TOLERANCE);
  ninter = 0;

  nodes = NodeBlocking(xyz, n_nodes, (cutoffRadius * TOLERANCE), n_xyz);
  start = nodes + n_nodes;

  xd = n_xyz[0]; yd = n_xyz[1]; zd = n_xyz[2];
  n_blk = xd * yd * zd;

  if (n_blk < 27) {
    /* too few blocks cause potentially wrong result */
    printf("Too few blocks!\n");
    exit(0);
  }

#ifdef CHECK
  {
    int *check, check_OK=1;

    MALLOC(check, int, n_nodes)
        bzero((void*)check, sizeof(int)*n_nodes);
    for (i=0; i<n_blk; i++)
      for (j=start[i]; j<start[i+1]; j++) {
        k = nodes[j];
        if (check[k] == 1) printf("doubly checked (%d)\n", k);
        else check[k] = 1;
      }

    for (i=0; i<n_nodes; i++)
      if (check[i] == 0) { 
        printf("not covered (%d)\n", i);
        check_OK = 0;
      }
    FREE(check, int, n_nodes)
        if (check_OK) printf("check is OK \n");
  }
#endif CHECK

  for (l = 0; l < n_blk; l++) {
    x = l % xd;
    y = (l / xd) % yd;
    z = l / (xd * yd);

    find_neighbor_blk(x, y, z, xd, yd, zd, neiblk); 

    for (k = 0; k < 27; k++) {

      m = neiblk[k];

      for (ii = start[l]; ii < start[l+1]; ii++) {
        for (jj = start[m]; jj < start[m+1]; jj++) {

          i = nodes[ii];
          j = nodes[jj];

          if (j <= i) continue;  /* no distinction in order (i,j) */

          xx = x(i) - x(j);
          yy = y(i) - y(j);
          zz = z(i) - z(j);

          if (continuous) {
            if ( xx < -sideHalf ) xx += side ;
            if ( yy < -sideHalf ) yy += side ;
            if ( zz < -sideHalf ) zz += side ;
            if ( xx >  sideHalf ) xx -= side ;
            if ( yy >  sideHalf ) yy -= side ;
            if ( zz >  sideHalf ) zz -= side ;
          }

          rd = xx*xx + yy*yy + zz*zz ;

          if ( rd <= cutoffSquare) {
            inter1 (ninter) = i;
            inter2 (ninter) = j;
            ninter ++;

            if ( ninter >= MAXINTERACT) perror("MAXINTERACT limit");
          }
        }}
    }
  } 
  printf("ninter = %d, cutoff = %lf\n", ninter, cutoffRadius);
  sort_edgelist (ninter, n_nodes, inter);

  FREE (nodes, int, 2*n_blk+1)
}
#endif

