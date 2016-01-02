#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <limits.h>
#include <math.h>
#include "misc.h"
#include "metis.h"
#include "timer.h"

typedef unsigned char byte;

#define L1_CACHE      32768
#define CACHE_LINE_SZ 32
#define UNIT_SZ       8

int LR_param_override = 0;
int **LR_index = NULL;
int LEVEL_arg = 12;
int npart_arg = 4096;	
int init_pSize = 4;
int nPass = 4;
int nShift = 3;
static double (*__LR_coordinate__)[3] = NULL;
int  DIMENSION =	3;	/* used in RCB routies */

#define MAX_PASS 10 

#define MAXLINE		1280000
#define EXTRA		17000

#ifdef SIMULATE
#define NI 500000 * sizeof(int)
#define EI 5000000 * sizeof(int)

/*
#if   defined(RCB)
#	define MAXBUF max(3*NI+2*EI, 7*NI) + EXTRA
#elif defined(KMETIS)
#	define MAXBUF max(4*NI+3*EI, 7*NI) + EXTRA
#elif defined(G_PART)
#	define MAXBUF max(3*NI+2*EI, 7*NI) + EXTRA
#elif defined(CPACK)
#	define MAXBUF max(3*NI+2*EI, 7*NI) + EXTRA
#else
#	define MAXBUF MAXLINE + EXTRA
#endif
*/

#define MAXBUF 4*NI+3*EI+ EXTRA 

byte malloc_buf[MAXBUF];
byte *ma_cur_ptr = malloc_buf;
byte *ma_end_ptr = malloc_buf + sizeof(malloc_buf);
byte *ma_postponed = NULL;
#endif /* SIMULATE */

/*
 * read graph file
 *
 * FILE FORMAT
 * #nodes #edges
 * adjacent nodes for 1st node
 * adjacent nodes for 2nd node
 *   ...
 * adjacent nodes for nth node
 *
 * (NOTE) node number starts from 1 not 0
 */

void
ReadInputGraph(char *filename, int *n_nodes, int *n_edges, int el[][2])
  /* read into edge-list */
{
  int i, k, cnt, edge;
  char *line, *oldstr, *newstr;
  FILE *fpin;

  MALLOC(line, char, MAXLINE+1)

      if ((fpin = fopen(filename, "r")) == NULL) {
        printf("Failed to open file %s\n", filename);
        exit(1);
      }

  do {
    fgets(line, MAXLINE, fpin);
  } while (line[0] == '%' && !feof(fpin));

  if (feof(fpin)) {
    *n_nodes = *n_edges = 0;
    FREE(line, char, MAXLINE+1)
        return;
  }

  sscanf(line, "%d %d", n_nodes, n_edges);
  printf("nodes= %d  edges= %d\n", *n_nodes, *n_edges); 

  /* Start reading the graph file */
  for (k=0, i=0; i< *n_nodes; i++) {
    if(!feof(fpin))
      fgets(line, MAXLINE, fpin);
    //} while (line[0]=='%'&&!feof(fpin));
    oldstr = line;
    newstr = NULL;

    if (strlen(line) == MAXLINE) {
      printf ("\nBuffer for fgets not big enough!\n");
      exit (1);
    }
    cnt = 0;
    while (1 && !feof(fpin)) {
      edge = (int)strtol(oldstr, &newstr, 10);
      oldstr = newstr;

      if (edge <= 0)  break;    /* no more edges */

      //if (edge >= i+1) { /* count only once */
#ifdef CSTYLE 			/* [0 .. n_edges-1] */
        el[k][0] = i;
        el[k][1] = edge -1;
#else			        /* [1 .. n_edges] */
        el[k][0] = i + 1;
        el[k][1] = edge;
#endif /* CSTYLE */

        k++;
        cnt++;
      //}
    } 
  }

  printf("Read done...\n");

  fclose(fpin);

  if (k != *n_edges) {
    printf("Actual number of edges is different (%d)\n", k);
    exit(1);
  }

  FREE(line, char, MAXLINE+1)
}

void
ReadInputGraph2(char *filename, int *n_nodes, int *n_edges, int *from, int *partners)
  /* read into partner-list */
{
  int i, k, edge;
  char *line, *oldstr, *newstr;
  FILE *fpin;

  MALLOC(line, char, MAXLINE+1)

      if ((fpin = fopen(filename, "r")) == NULL) {
        printf("Failed to open file %s\n", filename);
        exit(1);
      }

  do {
    fgets(line, MAXLINE, fpin);
  } while (line[0] == '%' && !feof(fpin));

  if (feof(fpin)) {
    *n_nodes = *n_edges = 0;
    FREE(line, char, MAXLINE+1)
        return;
  }

  sscanf(line, "%d %d", n_nodes, n_edges);
  printf("nodes= %d  edges= %d\n", *n_nodes, *n_edges); 

  /* Start reading the graph file */
  for (k=0, i=0; i< *n_nodes; i++) 
  {
    do 
    {
      fgets(line, MAXLINE, fpin);
    } while (line[0] == '%' && !feof(fpin));

    oldstr = line;
    newstr = NULL;

    if (strlen(line) == MAXLINE) 
    {
      printf ("\nBuffer for fgets not big enough!\n");
      exit (1);
    }

    from[i] = k;
    while (1 && !feof(fpin)) 
    {
      edge = (int)strtol(oldstr, &newstr, 10);
      oldstr = newstr;

      if (edge <= 0)   break;    /* no more edges */

      if (edge >= i+1) { /* count only once */
#ifdef CSTYLE 			/* [0 .. n_edges-1] */
        partners[k++] = edge-1;
#else				/* [1 .. n_edges] */
        partners[k++] = edge;
#endif /* CSTYLE */
      }
    } 
  }

  from[*n_nodes] = k;  /* end sentinel */

  fclose(fpin);

  if (k != *n_edges) {
    printf("Actual number of edges is different (%d)\n", k);
    exit(1);
  }

  FREE(line, char, MAXLINE+1)
}

void
ReadCoord(char *filename, int n_nodes, float xyz[][3])
{
  int i;
  FILE *fpin;

  if ((fpin = fopen(filename, "r")) == NULL) {
    printf("Failed to open file %s\n", filename);
    exit(1);
  }

  for (i = 0; ; i++) {
    if (fscanf(fpin, "%f %f %f", 
               &(xyz[i][0]), &(xyz[i][1]), &(xyz[i][2])) == EOF)
      break;
  } 

  if (i != n_nodes) {
    printf ("Actual number of nodes is different (%d)\n", i);
    exit (1); 
  }

  fclose(fpin);
}

void
ReadEdgeList(char *filename, int *n_nodes, int *n_edges, int el[][2])
{
  int i;
  FILE *fpin;

  if ((fpin = fopen(filename, "r")) == NULL) {
    printf("Failed to open file %s\n", filename);
    exit(1);
  }

  fscanf(fpin, "%d %d", n_nodes, n_edges);
  printf("nodes= %d  edges= %d\n", *n_nodes, *n_edges);

  for (i=0; i < *n_edges; i++) {
    fscanf(fpin, "%d %d", &(el[i][0]), &(el[i][1]));
  }

  fclose(fpin);
}

#ifdef INSPECTOR	/* LW inspector */

#define LOCAL(i)  (lb <= (i) && (i) < ub)

void
inspector(int (*el)[2], int ne, int nn, int my_id, int nproc,
          int (*both)[2],  int *bcnt,
          int (*left)[2],  int *lcnt,
          int (*right)[2], int *rcnt)
{
  int i, lb, ub, ep0, ep1, localL, localR;
#ifdef TIME
  double temp_lap;
#endif

  if (my_id == 0) { 
    printf ("LW Inspecting ...\n");
#ifdef TIME
    temp_lap = get_timer();
#endif
  }

#ifdef CSTYLE
  lb = max (nn * my_id / nproc, 0);
  ub = min (nn * (my_id + 1) / nproc, nn);
#else
  lb = max (nn * my_id / nproc + 1, 1);
  ub = min (nn * (my_id + 1) / nproc + 1, nn + 1);
#endif

  /* search through edge list and claasify */
  *bcnt = *lcnt = *rcnt = 0;
  for (i = 0; i < ne; i++) {
    ep0 = el[i][0];
    ep1 = el[i][1];

    localL = LOCAL(ep0);
    localR = LOCAL(ep1);

    if (localL && localR) {
      both[*bcnt][0] = ep0;
      both[(*bcnt)++][1] = ep1;
    }
    else if (localL) {
      left[*lcnt][0] = ep0;
      left[(*lcnt)++][1] = ep1;
    }
    else if (localR) {
      right[*rcnt][0] = ep0;
      right[(*rcnt)++][1] = ep1;
    }
  }

#ifdef TIME
  if (my_id == 0) printf("LW-INSP time = %.2lf\n", get_timer() - temp_lap);
#endif

  printf("[%d] %8d + %8d + %8d = %8d\n", my_id, *bcnt, *lcnt, *rcnt, 
         *bcnt + *lcnt + *rcnt);
}

void
inspector2(int *ptnr, int *from, int nn, int my_id, int nproc,
           int *bothptnr, int *bfrom,
           int *iptnr,    int *ifrom,
           int *jptnr,    int *jfrom)
{
  int i, p, lb, ub, pt, bp, ip, jp, localI, localP;
#ifdef TIME
  double temp_lap;
#endif

  if (my_id == 0) { 
    printf ("LW Inspecting(2) ...\n");
#ifdef TIME
    temp_lap = get_timer();
#endif
  }

#ifdef CSTYLE
  lb = max (nn * my_id / nproc, 0);
  ub = min (nn * (my_id + 1) / nproc, nn);
#else
  lb = max (nn * my_id / nproc + 1, 1);
  ub = min (nn * (my_id + 1) / nproc + 1, nn + 1);
#endif

  for (bp=ip=jp=i=0; i<nn; i++) {
    bfrom[i] = bp;
    ifrom[i] = ip;
    jfrom[i] = jp;
    for (p=from[i]; p<from[i+1]; p++) {
      pt = ptnr[p];

      localI = LOCAL(i+1);
      localP = LOCAL(pt);

      if (localI && localP) {
        bothptnr[bp++] = pt;
      }
      else if (localI) {
        iptnr[ip++] = pt;
      }
      else if (localP) {
        jptnr[jp++] = i+1;
      }
    }
  }
  bfrom[nn] = bp;
  ifrom[nn] = ip;
  jfrom[nn] = jp;

#ifdef TIME
  if (my_id == 0) printf("LW-INSP time = %.2lf\n", get_timer() - temp_lap);
#endif
  printf ("[%d] %8d + %8d + %8d = %8d\n", my_id, bp, ip, jp, bp+ip+jp);

}

#endif /* INSPECTOR */


#define SORT_ALT

int (*edge_list)[2];
#define edge_left(i) edge_list[i][0]
#define edge_right(i) edge_list[i][1]

#ifdef SORT
static int mycomp (const void *i, const void *j)
{
  int ii, jj;

  ii = *(int *)i; 
  jj = *(int *)j; 
  if (edge_left(ii) > edge_left(jj))
    return (1);
  if (edge_left(ii) < edge_left(jj))
    return (-1);

  /* edge_left values are same */
  if (edge_right(ii) > edge_right(jj))
    return (1);
  if (edge_right(ii) < edge_right(jj))
    return (-1);

  /* edge_left and edge_right values are same */
  return (0);
}

sort_edgelist(int n_edges, int n_nodes, int (*el)[2])
{
  int i;
  int *idx, *tmp2;
  int *tmp;

  printf("Sorting...");

  edge_list = el;
  MALLOC (idx, int, n_edges * 3)
      tmp = idx + n_edges;
  tmp2 = tmp + n_edges;

  for (i = 0; i < n_edges; i++) {
    tmp[i] = edge_left(i);
    tmp2[i] = edge_right(i);
  }

  for (i = 0; i < n_edges; i++)
    idx[i] = i; 

  qsort ( (char*) idx, n_edges, sizeof(int), mycomp);

  for (i = 0; i < n_edges; i++) {
    edge_left(i)  = tmp[idx[i]];
    edge_right(i) = tmp2[idx[i]];
  }

  FREE(idx, int, n_edges * 3)
}
#endif /* SORT */

#ifdef SORT_ALT
sort_edgelist(int n_edges, int n_nodes, int (*el)[2])
{
  int i, *count, *count2, *tmp, *tmp2, *p;
  /* printf("Sorting(alt) ..."); */

  edge_list = el;
  MALLOC (tmp, int, (n_edges*2 + n_nodes*2))
      tmp2 = tmp + n_edges;
  count = tmp2 + n_edges;
  count2 = count + n_nodes;

  for (i = 0; i < n_nodes; i++)
    count[i] = 0; 

  for (i = 0; i < n_edges; i++)
#ifdef CSTYLE
    count[edge_left(i)]++;
#else
  count[edge_left(i)-1]++;
#endif

  /* calculate the first place of each edge_left node */
  count[n_nodes -1] = n_edges - count[n_nodes -1];
  for (i = n_nodes -2; i >= 0; i--)
    count[i] = count[i+1] - count[i];

  /* spare extra copy for future use */
  for (i = 0; i < n_nodes; i++)
    count2[i] = count[i]; 

  for (i = 0; i < n_edges; i++) {
#ifdef CSTYLE
    tmp[count[edge_left(i)]] = edge_left(i);	
    tmp2[count[edge_left(i)]] = edge_right(i);	
    count[edge_left(i)]++;
#else
    tmp[count[edge_left(i)-1]] = edge_left(i);	
    tmp2[count[edge_left(i)-1]] = edge_right(i);	
    count[edge_left(i)-1]++;
#endif
  }

  /* place list in edge_left order */
  for (i = 0; i < n_edges; i++) {
    edge_left(i) = tmp[i];
    edge_right(i) = tmp2[i];
  }

  /* bubble sort edge_right elements */
  for (i = 0; i < n_nodes-1; i++) {
    bubblesort(count2[i], count2[i+1]-1);
  }
  bubblesort(count2[n_nodes-1], n_edges-1);

  FREE(tmp, int, (n_edges*2 + n_nodes*2))
      /* printf(" done!\n"); */
}

int
bubblesort(int start, int end)
{
  int a, i, j; 

  if (end - start <= 0) return 0;

  for (i=start; i < end; i++) {
    for (j=end; j > i; j--) {
      if (edge_right(j-1) > edge_right(j)) {
        a = edge_right(j); 	
        edge_right(j) = edge_right(j-1);
        edge_right(j-1) = a;
      }
    }}
  return 1;
}

int
bubblesort2(int *partners, int n)
{
  int a, i, j; 

  if (n <= 0) return 0;

  for (i=0; i < n-1; i++) {
    for (j=n-1; j > i; j--) {
      if (partners[j-1] > partners[j]) {
        a = partners[j]; 	
        partners[j] = partners[j-1];
        partners[j-1] = a;
      }
    }}
  return 1;
}

static int partnercomp (const void *i, const void *j)
{
  int ii, jj;

  ii = *(int *)i; 
  jj = *(int *)j; 
  if (ii > jj)
    return (1);
  if (ii < jj)
    return (-1);

  /* edge_left and edge_right values are same */
  return (0);
}

sortlist(int *list, int n) {

  if (n <= 20) bubblesort2(list, n);
  else {
    qsort(list, n, sizeof(int), partnercomp);
  }
}

#endif /* SORT_ALT */

void Adjust_EdgeList (int n_nodes, int n_edges, int (*el)[2], int *ind, int *ind2)
{
  int i, j;

  /* adjust edge list with index */
  for (i=0; i<n_edges; i++) {
#ifdef CSTYLE
    el[i][0] = ind2[el[i][0]];
    el[i][1] = ind2[el[i][1]];
#else
    el[i][0] = ind2[el[i][0] -1] + 1;
    el[i][1] = ind2[el[i][1] -1] + 1;
#endif /* CSTYLE */

    /* if operation is commutative, we can swap two items
       if (el[i][0] > el[i][1]) {
       j = el[i][0];
       el[i][0] = el[i][1];
       el[i][1] = j;
       }
       */
  }

  sort_edgelist(n_edges, n_nodes, el);
}

void Adjust_PartnerList (int n_nodes, int n_ptnrs, int *ptnr, int *from, int *ind, int *ind2)  
{ 
  int *t_ptnr, *t_from;
  int i, j, k;

  MALLOC (t_from, int, n_nodes+1 + n_ptnrs)
      t_ptnr = t_from + n_nodes + 1;          

  for (k=0, i=0; i<n_nodes; i++) {	
    t_from[i] = k;		
    for (j=from[ind[i]]; j<from[ind[i]+1]; j++)
#ifdef CSTYLE
      t_ptnr[k++] = ind2[ptnr[j]];
#else
    t_ptnr[k++] = ind2[ptnr[j] -1] +1;
#endif /* CSTYLE */
  }
  t_from[n_nodes] = k;

  for (i=0; i<=n_nodes; i++)
    from[i] = t_from[i];

  for (i=0; i<n_ptnrs; i++)
    ptnr[i] = t_ptnr[i];

  FREE (t_from, int, n_nodes+1 + n_ptnrs)

#ifdef COMPUTE_SORT
      /* sort the partners within a list */
      for (i=0; i<n_nodes; i++) {
        t_ptnr = &(ptnr[from[i]]);
        k = from[i+1] - from[i];
        sortlist(t_ptnr, k);	    
      }
#endif

}

Convert2Edgelist(int n_nodes, int n_edges, int (*lr)[2], int *partners, int *from) 
{
  int i, j, k;

  for (k=i=0; i<n_nodes; i++) {
    for (j=from[i]; j<from[i+1]; j++) {
#ifdef CSTYLE
      lr[k][0] = i;
#else
      lr[k][0] = i+1;
#endif
      lr[k++][1] = partners[j];
    }
  }
  if (k != n_edges) {printf("[C2E]Something wrong %d != %d\n", n_edges, k); exit(0); }
}

Convert2Partner(int n_nodes, int n_edges, int (*lr)[2], int *partners, int *from) 
{
  int i, j;

  from[0] = i = j = 0;
  while (j < n_nodes && i < n_edges) {

#ifdef CSTYLE
    if (j == lr[i][0]) 
#else
      if (j == lr[i][0] - 1)
#endif
      {
        partners[i] = lr[i][1]; 
        i++;
      }
      else {
        j++;
        from[j] = i;
      }
  }
  while (j <= n_nodes) 
    from[++j] = n_edges;
}

/*-----------------------------------------------------------------------*
 *  Convert2FPartner() and Expand2FPartner() change edge-list and        *
 *  partner list to METIS graph which names node ranging [0 .. n-1]      *
 *-----------------------------------------------------------------------*/
Convert2FPartner(int n_nodes, int n_edges, int (*lr)[2], int *fpartners, int *ffrom) 
{ 
  int i, j, *n;                            	

  MALLOC (n, int, n_nodes)			

      for (i=0; i<n_nodes; i++) n[i] = 0;		

  for (i=0; i<n_edges; i++) {			
#ifdef CSTYLE
    n[lr[i][0]]++;						
    n[lr[i][1]]++;				
#else
    n[lr[i][0] -1]++;				
    n[lr[i][1] -1]++;				
#endif
  }					
  for (ffrom[0]=i=0; i<n_nodes; i++) {
    ffrom[i+1] = ffrom[i] + n[i];
    n[i] = ffrom[i];
  }

  for (i=0; i<n_edges; i++) {
#ifdef CSTYLE
    j = lr[i][1];
    fpartners[n[j]++] = lr[i][0];
#else 
    j = lr[i][1] -1;
    fpartners[n[j]++] = lr[i][0] -1;
#endif
  }
  for (i=0; i<n_edges; i++) {
#ifdef CSTYLE
    j = lr[i][0];
    fpartners[n[j]++] = lr[i][1];
#else
    j = lr[i][0] -1;
    fpartners[n[j]++] = lr[i][1] -1;
#endif
  }
  FREE (n, int, n_nodes)			
}

Expand2FPartner(int n_nodes, int n_edges, int *partners, int *from, int *fpartners, int *ffrom) 
{ 
  int i, j, *n;                            	

  MALLOC (n, int, n_nodes)			

      for (i=0; i<n_nodes; i++) n[i] = 0;		

  for (i=0; i<n_nodes; i++) {
    n[i] += from[i+1] - from[i];
    for (j=from[i]; j<from[i+1]; j++) {			
#ifdef CSTYLE
      n[partners[j]]++;				
#else
      n[partners[j] -1]++;				
#endif
    }}
    for (ffrom[0]=i=0; i<n_nodes; i++) {
      ffrom[i+1] = ffrom[i] + n[i];
      n[i] = ffrom[i];
    }

    for (i=0; i<n_nodes; i++) {
      for (j=from[i]; j<from[i+1]; j++) {			
#ifdef CSTYLE
        fpartners[n[partners[j]]++] = i;
#else
        fpartners[n[partners[j]-1]++] = i;
#endif
      }}
    for (i=0; i<n_nodes; i++) {
      for (j=from[i]; j<from[i+1]; j++) {			
#ifdef CSTYLE
        fpartners[n[i]++] = partners[j];
#else
        fpartners[n[i]++] = partners[j] -1;
#endif
      }}
    FREE (n, int, n_nodes)			
}


/*========= RCB ===========================================*/


static int cur_dimension;
static double (*XYZ)[3];

int compar(const void *a, const void *b)
{
  double *xx, *yy;
  int dim = cur_dimension;

  xx = &(XYZ[*((int *)a)][0]);
  yy = &(XYZ[*((int *)b)][0]);

  if (*(xx + dim) > *(yy + dim)) 
    return 1;
  if (*(xx + dim) < *(yy + dim)) 
    return -1;

  dim = (dim + 1) % DIMENSION;

  if (*(xx + dim) > *(yy + dim)) 
    return 1;
  if (*(xx + dim) < *(yy + dim)) 
    return -1;

  if (DIMENSION == 3) {
    dim = (dim + 1) % DIMENSION;

    if (*(xx + dim) > *(yy + dim)) 
      return 1;
    if (*(xx + dim) < *(yy + dim)) 
      return -1;
  }

  return 0;
}

void 
PureRcbPartition(int n_nodes, double xyz[][3], int *ind)
{
  int levels, i,j,k;
  int npart, *part, *ind2;		/* beginning of a part */
#ifdef TIME
  double tmp_time;
#endif

  XYZ = xyz;	 /* let the comparison routine know the coordinates */

  /*
   *  (Note)  2^LEVEL + 1 == NPART
   */
  levels = LEVEL_arg;
  j = 1;
  j <<= levels;
  j++;

  MALLOC (part, int, j)

      printf("Levels = %d\n", levels);

  for (i=0; i<n_nodes; i++)
    ind[i] = i;

  /*    if (levels == 0) return;   */

  part[0] = 0; 
  part[1] = n_nodes;
  npart = 1;

  /* RCB partition */ 
  for (i=0 ; i<levels; i++, npart<<=1) {
    cur_dimension = i % DIMENSION;

    for (j=npart; j > 0; j--) {
      qsort ((void *)(&ind[part[j-1]]), part[j]-part[j-1],
             sizeof(int), compar);

      part[j<<1] = part[j];
      part[(j<<1)-1] = (part[j] + part[j-1]) >>1;
    }
  }

  /*
   *  Returning with rearrange index, ind[]
   *  ind[i]  : new -> old	[0...n_nodes-1]
   */
}

void 
RcbPartition0(int n_nodes, int n_ptnrs, double xyz[][3], int (*el)[2], int *ind)
{
  int levels, i,j,k;
  int npart, *part, *ind2;		/* beginning of a part */
#ifdef TIME
  double tmp_time, tmp_time2;

  tmp_time = get_timer();
#endif


  XYZ = xyz;	 /* let the comparison routine know the coordinates */

  /*
   *  (Note)  2^LEVEL + 1 == NPART
   */
  levels = LEVEL_arg;
  j = 1;
  j <<= levels;
  j++;

  MALLOC (part, int, j)

      /* printf("Levels = %d\n", levels); */

      for (i=0; i<n_nodes; i++)
        ind[i] = i;

  part[0] = 0; 
  part[1] = n_nodes;
  npart = 1;

  if (levels == 0) return;

  /* RCB partition */ 
  for (i=0 ; i<levels; i++, npart<<=1) {
    cur_dimension = i % DIMENSION;

    for (j=npart; j > 0; j--) {
      qsort ((void *)(&ind[part[j-1]]), part[j]-part[j-1],
             sizeof(int), compar);

      part[j<<1] = part[j];
      part[(j<<1)-1] = (part[j] + part[j-1]) >>1;
    }
  }

  /*
   *  ind[i]  : new -> old	[0...n_nodes-1]
   *  ind2[i] : old -> new	[0...n_nodes-1]
   */
#ifdef TIME
  tmp_time2 = get_timer();
#endif
  MALLOC (ind2, int, n_nodes) 
      for (i=0; i<n_nodes; i++) ind2[ind[i]] = i; 

  Adjust_EdgeList(n_nodes, n_ptnrs, el, ind, ind2);  

  FREE (ind2, int, n_nodes) /* ind2 */
#ifdef TIME
      printf("(Pure_time = %.2lf (+ %.2lf) - ", tmp_time2 - tmp_time, 
             get_timer() - tmp_time2);
#endif
}

void 
RcbPartition(int n_nodes, int n_ptnrs, double xyz[][3], int *ptnr, int *from, int *ind)
{
  int levels, i,j,k;
  int npart, *part, *ind2;		/* beginning of a part */
#ifdef TIME
  double tmp_time, tmp_time2;

  tmp_time = get_timer();
#endif


  XYZ = xyz;	 /* let the comparison routine know the coordinates */

  /*
   *  (Note)  2^LEVEL + 1 == NPART
   */
  levels = LEVEL_arg;
  j = 1;
  j <<= levels;
  j++;

  MALLOC (part, int, j)

      /* printf("Levels = %d\n", levels); */

      for (i=0; i<n_nodes; i++)
        ind[i] = i;

  part[0] = 0; 
  part[1] = n_nodes;
  npart = 1;

  if (levels == 0) return;

  /* RCB partition */ 
  for (i=0 ; i<levels; i++, npart<<=1) {
    cur_dimension = i % DIMENSION;

    for (j=npart; j > 0; j--) {
      qsort ((void *)(&ind[part[j-1]]), part[j]-part[j-1],
             sizeof(int), compar);

      part[j<<1] = part[j];
      part[(j<<1)-1] = (part[j] + part[j-1]) >>1;
    }
  }

  /*
   *  ind[i]  : new -> old	[0...n_nodes-1]
   *  ind2[i] : old -> new	[0...n_nodes-1]
   *  
   */
#ifdef TIME
  tmp_time2 = get_timer();
#endif
  MALLOC (ind2, int, n_nodes) 
      for (i=0; i<n_nodes; i++) ind2[ind[i]] = i; 

  Adjust_PartnerList(n_nodes, n_ptnrs, ptnr, from, ind, ind2);  

  FREE (ind2, int, n_nodes) /* ind2 */
#ifdef TIME
      printf("(Pure_time = %.2lf (+ %.2lf) - ", tmp_time2 - tmp_time, 
             get_timer() - tmp_time2);
#endif
}

void 
rcbprocess(int n_nodes, int n_edges, int (*el)[2], int n, long (*repos)[2])
{
  int *ind, i, siz, siz0 = 0; 
  byte *ptr, *tx = NULL;
  va_list ap;
  int *partners, *from;
#ifdef TIME
  double temp_lap, temp_lap2, temp_lap3;
#endif

  /* printf("RCB ... \n"); */
#ifdef TIME
  temp_lap = get_timer();
#endif

  if (LR_index == NULL) MALLOC (ind, int, n_nodes)
  else ind = *LR_index;

#ifdef CONVERT
  MALLOC (partners, int, n_nodes + n_edges + 1)
      from = partners + n_edges;
  Convert2Partner(n_nodes, n_edges, el, partners, from);
  RcbPartition(n_nodes, n_edges, __LR_coordinate__, partners, from, ind); 
  Convert2Edgelist(n_nodes, n_edges, el, partners, from);
  FREE (partners, int, n_nodes + n_edges + 1)
#else
      RcbPartition0(n_nodes, n_edges, __LR_coordinate__, el, ind);
#endif

#ifdef TIME
  temp_lap2 = get_timer();
#endif

  N_REPOSITION()
      if (LR_index == NULL) FREE(ind, int, n_nodes)
      else *LR_index = ind;

#ifdef TIME
  temp_lap3 = get_timer();
  printf("RCB time = %.2lf (%.2lf + %.2lf)\n", temp_lap3 - temp_lap, temp_lap2 - temp_lap, temp_lap3 - temp_lap2);	
#endif
}

void
rcbprocess2(int n_nodes, int n_edges, int *partners, int *from, int n, long (*repos)[2])
{
  int *ind, i, siz, siz0 = 0; 
  byte *ptr, *tx = NULL;
  va_list ap;
#ifdef TIME
  double temp_lap, temp_lap2, temp_lap3;
#endif

  /* printf("RCB ... \n"); */
#ifdef TIME
  temp_lap = get_timer();
#endif

  if (LR_index == NULL) MALLOC (ind, int, n_nodes)
  else ind = *LR_index;

  RcbPartition(n_nodes, n_edges, __LR_coordinate__, partners, from, ind);

#ifdef TIME
  temp_lap2 = get_timer();
#endif

  N_REPOSITION()
      if (LR_index == NULL) FREE(ind, int, n_nodes)
      else *LR_index = ind;


#ifdef TIME
  temp_lap3 = get_timer();
  printf("RCB time = %.2lf (%.2lf + %.2lf)\n", temp_lap3 - temp_lap, temp_lap2 - temp_lap, temp_lap3 - temp_lap2);
#endif
}

/*========= METIS ===========================================*/

void
MetisPartition0(int n_nodes, int n_ptnrs, int (*el)[2], int *ind)
{
  int *cnt, *ind2;
  int i, j, k, a, npart;
#ifdef TIME
  double tmp_time;
#endif

  npart = npart_arg;
  /* printf("Npart= %d\n", npart); */
  MALLOC (ind2, int, n_nodes)
      MALLOC (cnt, int, npart)

      for (i=0; i<npart; i++) cnt[i] = 0;

  for (i=0; i<n_nodes; i++) {
    a = ind[i];
    if (a < 0 || a >= npart) printf("out of range = %d\n",a);
    cnt[a]++;	
    ind2[i] = a;
  }

  cnt[npart-1] = n_nodes - cnt[npart-1];
  for (i=npart-1; i; i--)
    cnt[i-1] = cnt[i] - cnt[i-1];

  for (i=0; i<n_nodes; i++) 
    ind2[i] = cnt[ind2[i]]++;

  FREE(cnt, int, npart) 

      /*
       *  ind[i]  : new -> old        [0...n_nodes-1]
       *  ind2[i] : old -> new        [0...n_nodes-1]
       */
#ifdef TIME
      tmp_time = get_timer();
#endif

  for (i=0; i<n_nodes; i++) ind[ind2[i]] = i; 

  Adjust_EdgeList(n_nodes, n_ptnrs, el, ind, ind2);  

  FREE (ind2, int, n_nodes)
#ifdef TIME
      printf("(Adjust_time = %.2lf) - ", get_timer() - tmp_time);
#endif

}

void
MetisPartition(int n_nodes, int n_ptnrs, int *ptnr, int *from, int *ind)
{
  int *cnt, *ind2;
  int i, j, k, a, npart;
#ifdef TIME
  double tmp_time;
#endif

  npart = npart_arg;
  /* printf("Npart= %d\n", npart); */
  MALLOC (ind2, int, n_nodes)
      MALLOC (cnt, int, npart)

      for (i=0; i<npart; i++) cnt[i] = 0;

  for (i=0; i<n_nodes; i++) {
    a = ind[i];
    if (a < 0 || a >= npart) printf("out of range = %d\n",a);
    cnt[a]++;	
    ind2[i] = a;
  }

  cnt[npart-1] = n_nodes - cnt[npart-1];
  for (i=npart-1; i; i--)
    cnt[i-1] = cnt[i] - cnt[i-1];

  for (i=0; i<n_nodes; i++) 
    ind2[i] = cnt[ind2[i]]++;

  FREE(cnt, int, npart) 

      /*
       *  ind[i]  : new -> old        [0...n_nodes-1]
       *  ind2[i] : old -> new        [0...n_nodes-1]
       */
#ifdef TIME
      tmp_time = get_timer();
#endif

  for (i=0; i<n_nodes; i++) ind[ind2[i]] = i; 

  Adjust_PartnerList(n_nodes, n_ptnrs, ptnr, from, ind, ind2);  

  FREE (ind2, int, n_nodes)
#ifdef TIME
      printf("(Adjust_time = %.2lf) - ", get_timer() - tmp_time);
#endif

}


void
metisprocess(int n_nodes, int n_edges, int (*el)[2], int n, long (*repos)[2])
{
  int *ind, i, siz, siz0 = 0; 
  byte *ptr, *tx = NULL;
  va_list ap;
  GraphType graph;
  int nparts, options[10];
  int numflag = 0, wgtflag = 0, edgecut;
  int *full_partners, *full_from;
  int *partners, *from;
#ifdef TIME
  double temp_lap, temp_lap2, temp_lap3;
#endif

  /* printf("METIS ... \n"); */
#ifdef TIME
  temp_lap = get_timer();
#endif

  if (LR_index == NULL) MALLOC (ind, int, n_nodes)
  else ind = *LR_index;

  /*-----  set up graph structure for METIS library --------------*/
  MALLOC (full_partners, int, 2*n_edges + n_nodes + 1)
      full_from = full_partners + 2*n_edges; 

  Convert2FPartner(n_nodes, n_edges, el, full_partners, full_from);
  graph.nvtxs = n_nodes;
  graph.xadj = full_from;
  graph.adjncy = full_partners;
  graph.vwgt = NULL;
  graph.adjwgt = NULL;

  nparts = npart_arg; options[0] = 0;
#ifdef METIS_LIB
  METIS_PartGraphKway(&graph.nvtxs, graph.xadj, graph.adjncy, graph.vwgt, 
                      graph.adjwgt, &wgtflag, &numflag, &nparts, options, &edgecut, ind);
#endif

#ifdef CONVERT
  partners = full_partners; 
  from = partners + n_edges;
  Convert2Partner(n_nodes, n_edges, el, partners, from);
  MetisPartition(n_nodes, n_edges, partners, from, ind); 
  Convert2Edgelist(n_nodes, n_edges, el, partners, from);
  FREE (full_partners, int, 2*n_edges + n_nodes + 1)
#else
      FREE (full_partners, int, 2*n_edges + n_nodes + 1)

      MetisPartition0(n_nodes, n_edges, el, ind); 
#endif

#ifdef TIME
  temp_lap2 = get_timer();
#endif

  N_REPOSITION()
      if (LR_index == NULL) FREE(ind, int, n_nodes)
      else *LR_index = ind;

#ifdef TIME
  temp_lap3 = get_timer();
  printf("METIS time = %.2lf (%.2lf + %.2lf)\n", temp_lap3 - temp_lap, temp_lap2 - temp_lap, temp_lap3 - temp_lap2);
#endif
}

void
metisprocess2(int n_nodes, int n_edges, int *partners, int *from, int n, long (*repos)[2])
{
  int *ind, i, siz, siz0 = 0; 
  byte *ptr, *tx = NULL;
  va_list ap;
  GraphType graph;
  int nparts, options[10];
  int numflag = 0, wgtflag = 0, edgecut;
  int *full_partners, *full_from;
#ifdef TIME
  double temp_lap, temp_lap2, temp_lap3;
#endif

  /* printf("METIS ... \n"); */
#ifdef TIME
  temp_lap = get_timer();
#endif

  if (LR_index == NULL) MALLOC (ind, int, n_nodes)
  else ind = *LR_index;

  /* set up graph structure for METIS library routine */
  MALLOC (full_partners, int, 2*n_edges + n_nodes + 1)
      full_from = full_partners + 2*n_edges; 

  Expand2FPartner(n_nodes, n_edges, partners, from, full_partners, full_from);
  graph.nvtxs = n_nodes;
  graph.xadj = full_from;
  graph.adjncy = full_partners;
  graph.vwgt = NULL;
  graph.adjwgt = NULL;

  nparts = npart_arg; options[0] = 0;
#ifdef METIS_LIB
  METIS_PartGraphKway(&graph.nvtxs, graph.xadj, graph.adjncy, graph.vwgt, 
                      graph.adjwgt, &wgtflag, &numflag, &nparts, options, &edgecut, ind);
#endif

  FREE (full_partners, int, 2*n_edges + n_nodes + 1)

      MetisPartition(n_nodes, n_edges, partners, from, ind); 

#ifdef TIME
  temp_lap2 = get_timer();
#endif

  N_REPOSITION()
      if (LR_index == NULL) FREE(ind, int, n_nodes)
      else *LR_index = ind;

#ifdef TIME
  temp_lap3 = get_timer();
  printf("METIS time = %.2lf (%.2lf + %.2lf)\n", temp_lap3 - temp_lap, temp_lap2 - temp_lap, temp_lap3 - temp_lap2);
#endif
}

/*========= GPART ===========================================*/

int 
SortDegree(int *degree, int n_nodes, int *ind, int BiggestFirst)
{
  int  i, max_degree, *hcnt;
  int s = 0;

  max_degree = 0;
  for (i=0; i<n_nodes; i++)
    if (max_degree < degree[i]) max_degree = degree[i];

  MALLOC (hcnt, int, max_degree+2)

      /* Generate histogram */
      for (i=0; i<= max_degree; i++) 
        hcnt[i] = 0;

  for (i=0; i<n_nodes; i++) {
    hcnt[degree[i]]++;
  }

  /* Sort */
  if (BiggestFirst) {
    hcnt[0] = n_nodes - hcnt[0];  /* sentinel */
    for (i=1; i <=max_degree; i++)
      hcnt[i] = hcnt[i-1] - hcnt[i]; 

  } else {
    hcnt[max_degree+1] = n_nodes;  /* sentinel */
    for (i=max_degree; i >= 0; i--)
      hcnt[i] = hcnt[i+1] - hcnt[i]; 
  }

  for (i=0; i<n_nodes; i++)
    ind[hcnt[degree[i]]++] = i;

  FREE(hcnt, int, max_degree+2)

      return max_degree;
}

#define ROOT(rt, lf) { \
  (rt) = (lf); \
  while (nodes[rt] >= 0) (rt) = nodes[rt]; \
}

int
omit_localedges(int *from, int *ptnr, int n_nodes)
{
  int i,j, begin, cur;

  begin = cur = 0;
  for (i=0; i<n_nodes; i++) {
    for (j=begin; j < from[i+1]; j++) {
      if (ptnr[j] != -1) {
        ptnr[cur++] = ptnr[j];	
      }
    }
    begin = from[i+1];
    from[i+1] = cur;	
  }
}

/* use this after flattening the trees */
#define ROOT2(rt, lf)   (rt) = (nodes[lf] < 0)? (lf) : nodes[lf]

/* Merge into first tree */
#define MERGE(rt, rt2) { \
  if (rt <= rt2) {	\
    nodes[rt] += nodes[rt2];	\
    nodes[rt2] = rt;		\
  } else {				\
    nodes[rt2] += nodes[rt];	\
    nodes[rt] = rt2;		\
    rt = rt2;			\
  }					\
}
#define MERGE2(rt, rt2) { \
  if (ind2[rt] <= ind2[rt2]) {	\
    nodes[rt] += nodes[rt2];	\
    nodes[rt2] = rt;		\
  } else {				\
    nodes[rt2] += nodes[rt];	\
    nodes[rt] = rt2;		\
    rt = rt2;			\
  }					\
}

#define LTE_LIMIT(rt, rt2) (-(nodes[rt] + nodes[rt2]) <= cur_max_part)
#define GT_LIMIT(rt) (-(nodes[rt]) > cur_max_part)
#define GTE_LIMIT(rt) (-(nodes[rt]) >= cur_max_part)
#define EQ_LIMIT(rt) (-(nodes[rt]) == cur_max_part)


int *
G_PartPartition0(int n_nodes, int n_ptnrs, int (*el)[2], int *ind)
{
  int  i, j, k, t, n_pieces, root, root2, p, done, iter, real_i;
  int  max_degree, init_max_part, cur_max_part, m, max;
  int  *nodes, *degree, *cnt, *ind2, *nodes2, *tp;
  int  *sort_ord, *from2, *from, *ptnr2;
  int  pass, npass, nshift;
#ifdef TIME
  double  tmp_time,  tmp_time2;
  double  tmp_time3, tmp_time4=0.0;
#endif
#ifdef SIMULATE
  int  *org_ind = ind;
#endif
#ifdef TIME
  tmp_time = get_timer();
#endif

  /* printf ("Hierarchical Clustering (3)...\n"); */

  MALLOC (ind2, int, n_nodes)
      MALLOC (sort_ord, int, n_nodes*6 + n_ptnrs + 2)
      degree = sort_ord + n_nodes;
  nodes = degree + n_nodes;
  nodes2 = nodes + n_nodes;
  ptnr2 = nodes2 + n_nodes;
  from2 = ptnr2 + n_ptnrs;
  from  = from2 + n_nodes + 1;

  /*
   *  Sort by degree and make new partner list (ptnr2, from2)
   */
  for (i=0; i<n_nodes; i++)  degree[i] = 0;
  for (i=0; i<n_ptnrs; i++)
#ifdef CSTYLE
    degree[el[i][0]] ++;
#else
  degree[el[i][0]-1] ++;
#endif /* CSTYLE */

  max_degree = SortDegree(degree, n_nodes, sort_ord, 1);

  from[0] = 0;
  for (i=1; i<=n_nodes; i++)
    from[i] = from[i-1] + degree[i-1];

  for (t=i=0; i<n_nodes; i++) {
    from2[i] = t;
    k = sort_ord[i];
    for (j=from[k]; j<from[k+1]; j++)
      ptnr2[t++] = el[j][1];	
  } 
  from2[n_nodes] = n_ptnrs;

  /*
   *  nodes[] - if root, record number of nodes in subtree in NEGATIVE VALUE 
   *            else   , hold index of to parent (i.e. pointer to parent) 
   */
  for (i=0; i<n_nodes; i++) { 
    nodes[i] = -1; 
    ind[i] = ind2[i] = i;
  }

#ifdef VERBOSE
  printf("Preprocess time = %.2lf\n", get_timer() - tmp_time);
#endif
  /*
   * Gradually clustering with hierarchical tree 
   * Merging into larger pieces 
   */
  n_pieces = n_nodes;	/* initial number of partitioned pieces */
  init_max_part = init_pSize; 
  npass = nPass; nshift = nShift;
  /*
     printf("[InitMaxPart=%d (Pass,Shift)=(%d, %d) MaxDegree= %d node= %d]\n", 
     init_max_part, npass, nshift, max_degree, n_nodes);
     */

  cnt = degree; /* rename degree[] with cnt[] */ 
  cur_max_part = init_max_part;
  for (pass = 0; pass < npass && n_pieces > 1; pass++, cur_max_part<<=nshift)
  { 
#ifdef VERBOSE
    printf("cur_max_part=%d n_pieces=%d\n", cur_max_part, n_pieces);
#endif
    for (i=0; i<n_nodes; i++) {
      real_i = sort_ord[i];
      ROOT (root, ind2[real_i])
          if (GTE_LIMIT(root)) continue;
      for (j=from2[i]; j<from2[i+1]; j++) {
#ifdef CSTYLE
        p = ptnr2[j];
#else
        p = ptnr2[j] - 1;
#endif /* CSTYLE */
        ROOT (root2, ind2[p])	/* find neighbor's root */
            if (root == root2) ptnr2[j] = -1;
            else if (LTE_LIMIT(root, root2)) {
              ptnr2[j] = -1;
              MERGE (root, root2)
                  n_pieces--;
              if (EQ_LIMIT(root)) break; 
            }
      }
    }
#ifdef VERBOSE
    tmp_time3 = get_timer();
#endif
    omit_localedges(from2, ptnr2, n_nodes);

    /* Reordering groups */
    m = 0; 
    for (i=0; i<n_nodes; i++) {
      if (nodes[i] < 0) {
        nodes2[m] = nodes[i]; /* copy num of children */
        ind2[m] = ind[i];
        cnt[i] = m+1;	      /* new starting postion of children */
        nodes[i] = -(m+1);    /* new root position in -[1..n] */
        m += (-nodes2[m]);    /* slide to next tree position */
      }
      else {
        ROOT (t, i)
            nodes2[cnt[t]] = -nodes[t]-1;   /* copy new root location */
        ind2[cnt[t]++] = ind[i];

      }
    }

    /* swap */
    tp = nodes; nodes = nodes2; nodes2 = tp; 
    tp = ind; ind = ind2; ind2 = tp; /* swap */
    for (i=0; i<n_nodes; i++) ind2[ind[i]] = i; /* make inverse */
#ifdef VERBOSE
    tmp_time4 += (get_timer() - tmp_time3);
#endif
  }

#ifdef VERBOSE
  printf("n_pieces= %d Sorting time = %.2lf\n", n_pieces, tmp_time4);
#endif

  FREE (sort_ord, int, n_nodes*6 + n_ptnrs + 2)

#ifdef TIME
      tmp_time2 = get_timer();
#endif
  /*
   *  ind[i]  : new -> old        [0...n_nodes-1]
   *  ind2[i] : old -> new        [0...n_nodes-1]
   */
  Adjust_EdgeList(n_nodes, n_ptnrs, el, ind, ind2);  

#ifdef SIMULATE
  if (org_ind != ind) {
    for (i=0; i<n_nodes; i++) ind2[i] = ind[i];
    tp = ind; ind = ind2; ind2 = tp; 
  }
#endif
  FREE (ind2, int, n_nodes)
#ifdef TIME
      printf("(Pure_time = %.2lf (+%.2lf) - ", 
             tmp_time2 - tmp_time,
             get_timer() - tmp_time2);
#endif

  return ind;
}

int *
G_PartPartition(int n_nodes, int n_ptnrs, int *ptnr, int *from, int *ind)
{
  int  i, j, k, t, n_pieces, root, root2, p, done, iter, real_i;
  int  max_degree, init_max_part, cur_max_part, m, max;
  int  *nodes, *degree, *cnt, *ind2, *nodes2, *tp;
  int  *sort_ord, *from2, *ptnr2;
  int  pass, npass, nshift;
#ifdef TIME
  double  tmp_time,  tmp_time2;
  double  tmp_time3, tmp_time4=0.0;
#endif
#ifdef SIMULATE
  int  *org_ind = ind;
#endif
#ifdef TIME
  tmp_time = get_timer();
#endif

  /* printf ("Hierarchical Clustering (3)...(%d %d)\n", n_nodes, n_ptnrs); */

  MALLOC (ind2, int, n_nodes)
      MALLOC (sort_ord, int, n_nodes*5 + n_ptnrs + 1)
      degree = sort_ord + n_nodes;
  nodes = degree + n_nodes;
  nodes2 = nodes + n_nodes;
  ptnr2 = nodes2 + n_nodes;
  from2 = ptnr2 + n_ptnrs;

  /*
   *  Sort by degree and make new partner list (ptnr2, from2)
   */
  for (i=0; i<n_nodes; i++)
    degree[i] = from[i+1] - from[i];        /* degree of each node */

  max_degree = SortDegree(degree, n_nodes, sort_ord, 1);

  for (t=i=0; i<n_nodes; i++) {
    from2[i] = t;
    k = sort_ord[i];
    for (j=from[k]; j<from[k+1]; j++)
      ptnr2[t++] = ptnr[j];	
  } 
  from2[n_nodes] = n_ptnrs;

  /*
   *  nodes[] - if root, record number of nodes in subtree in NEGATIVE VALUE 
   *            else   , hold index of to parent (i.e. pointer to parent) 
   */
  for (i=0; i<n_nodes; i++) { 
    nodes[i] = -1; 
    ind[i] = ind2[i] = i;
  }

#ifdef VERBOSE
  printf("Preprocess time = %.2lf\n", get_timer() - tmp_time);
#endif
  /*
   * Gradually clustering with hierarchical tree 
   * Merging into larger pieces 
   */
  n_pieces = n_nodes;	/* initial number of partitioned pieces */
  init_max_part = init_pSize; 
  npass = nPass; nshift = nShift;
  /*
     printf("[InitMaxPart=%d (Pass,Shift)=(%d, %d) MaxDegree= %d node= %d]\n", 
     init_max_part, npass, nshift, max_degree, n_nodes);
     */
  cnt = degree; /* rename degree[] with cnt[] */ 
  cur_max_part = init_max_part;
  for (pass = 0; pass < npass && n_pieces > 1; pass++, cur_max_part<<=nshift)
  { 
#ifdef VERBOSE
    printf("cur_max_part=%d n_pieces=%d\n", cur_max_part, n_pieces);
#endif
    for (i=0; i<n_nodes; i++) {
      real_i = sort_ord[i];
      ROOT (root, ind2[real_i])
          if (GTE_LIMIT(root)) continue;
      for (j=from2[i]; j<from2[i+1]; j++) {
#ifdef CSTYLE
        p = ptnr2[j];
#else
        p = ptnr2[j] - 1;
#endif /* CSTYLE */
        ROOT (root2, ind2[p])	/* find neighbor's root */
            if (root == root2) ptnr2[j] = -1;
            else if (LTE_LIMIT(root, root2)) {
              ptnr2[j] = -1;
              MERGE (root, root2)
                  n_pieces--;
              if (EQ_LIMIT(root)) break; 
            }
      }
    }
#ifdef VERBOSE
    tmp_time3 = get_timer();
#endif
    omit_localedges(from2, ptnr2, n_nodes);

    /* Reordering groups */
    m = 0; 
    for (i=0; i<n_nodes; i++) {
      if (nodes[i] < 0) {
        nodes2[m] = nodes[i]; /* copy num of children */
        ind2[m] = ind[i];
        cnt[i] = m+1;	      /* new starting postion of children */
        nodes[i] = -(m+1);    /* new root position in -[1..n] */
        m += (-nodes2[m]);    /* slide to next tree position */
      }
      else {
        ROOT (t, i)
            nodes2[cnt[t]] = -nodes[t]-1;   /* copy new root location */
        ind2[cnt[t]++] = ind[i];

      }
    }

    /* swap */
    tp = nodes; nodes = nodes2; nodes2 = tp; 
    tp = ind; ind = ind2; ind2 = tp; /* swap */
    for (i=0; i<n_nodes; i++) ind2[ind[i]] = i; /* make inverse */
#ifdef VERBOSE
    tmp_time4 += (get_timer() - tmp_time3);
#endif
  }

#ifdef VERBOSE
  printf("n_pieces= %d Sorting time = %.2lf\n", n_pieces, tmp_time4);
#endif

  FREE (sort_ord, int, n_nodes*5 + n_ptnrs + 1)

#ifdef TIME
      tmp_time2 = get_timer();
#endif
  /*
   *  ind[i]  : new -> old        [0...n_nodes-1]
   *  ind2[i] : old -> new        [0...n_nodes-1]
   */
  Adjust_PartnerList(n_nodes, n_ptnrs, ptnr, from, ind, ind2);  

#ifdef SIMULATE
  if (org_ind != ind) {
    for (i=0; i<n_nodes; i++) ind2[i] = ind[i];
    tp = ind; ind = ind2; ind2 = tp; 
  }
#endif
  FREE (ind2, int, n_nodes)
#ifdef TIME
      printf("(Pure_time = %.2lf (+%.2lf) - ", 
             tmp_time2 - tmp_time,
             get_timer() - tmp_time2);
#endif

  return ind;
}

void
gpartprocess(int n_nodes, int n_edges, int (*el)[2], int n, long (*repos)[2])
{
  int *ind, i, siz, siz0 = 0; 
  byte *ptr, *tx = NULL;
  int  *partners, *from;
  va_list ap;
#ifdef TIME
  double temp_lap, temp_lap2, temp_lap3, tmp_time;
#endif

  /* printf("G-PART ... \n"); */
#ifdef TIME
  temp_lap = get_timer();
#endif

  if (LR_index == NULL) MALLOC (ind, int, n_nodes)
  else ind = *LR_index;

#ifdef CONVERT
  /* convert edge list to partner list */
  MALLOC(partners, int, n_nodes+n_edges+1)
      from = partners + n_edges;
  Convert2Partner(n_nodes, n_edges, el, partners, from);
  ind = G_PartPartition(n_nodes, n_edges, partners, from, ind); 
  Convert2Edgelist(n_nodes, n_edges, el, partners, from);
  FREE(partners, int, n_nodes+n_edges+1)
#else
      ind = G_PartPartition0(n_nodes, n_edges, el, ind);
#endif

#ifdef TIME
  temp_lap2 = get_timer();
#endif

  N_REPOSITION()
      if (LR_index == NULL) FREE(ind, int, n_nodes)
      else *LR_index = ind;

#ifdef TIME
  temp_lap3 = get_timer();
  printf("GPART time = %.2lf (%.2lf + %.2lf)\n", temp_lap3 - temp_lap, temp_lap2 - temp_lap, temp_lap3 - temp_lap2);
#endif
}

void
gpartprocess2(int n_nodes, int n_edges, int *partners, int *from, int n, long (*repos)[2])
{
  int *ind, i, siz, siz0 = 0; 
  byte *ptr, *tx = NULL;
  va_list ap;
#ifdef TIME
  double temp_lap, temp_lap2, temp_lap3;
#endif

  /* printf("G-PART ... \n"); */
#ifdef TIME
  temp_lap = get_timer();
#endif

  if (LR_index == NULL) MALLOC (ind, int, n_nodes)
  else ind = *LR_index;

  ind = G_PartPartition(n_nodes, n_edges, partners, from, ind); 

#ifdef TIME
  temp_lap2 = get_timer();
#endif

  N_REPOSITION()
      if (LR_index == NULL) FREE(ind, int, n_nodes)
      else *LR_index = ind;

#ifdef TIME
  temp_lap3 = get_timer();
  printf("GPART time = %.2lf (%.2lf + %.2lf)\n", temp_lap3 - temp_lap, temp_lap2 - temp_lap, temp_lap3 - temp_lap2);
#endif
}

/*========= CPACK ===========================================*/

void
CPack0(int n_nodes, int n_ptnrs, int (*el)[2], int *ind)
{
  int  i, j, k, p, q, n = 0;
  int  *taken, *ind2;
#ifdef TIME
  double tmp_time, tmp_time2;
#endif
#ifdef TIME
  tmp_time = get_timer();
#endif

  MALLOC (taken, int, n_nodes)
      bzero((void*) taken, n_nodes*sizeof(int));

  for (i=0; i<n_ptnrs; i++) {
#ifdef CSTYLE
    p = el[i][0];
    q = el[i][1];
#else
    p = el[i][0] -1;
    q = el[i][1] -1;
#endif /* CSTYLE */
    if (taken[p] == 0) {ind[n++] = p; taken[p] = 1;}
    if (taken[q] == 0) {ind[n++] = q; taken[q] = 1;}
  }
  if (n != n_nodes) { printf("something messed up! (%d != %d)\n", n_nodes, n); exit(1); }
  FREE (taken, int, n_nodes)

      /*
       *  ind[i]  : new -> old        [0...n_nodes-1]
       *  ind2[i] : old -> new        [0...n_nodes-1]
       */

#ifdef TIME
      tmp_time2 = get_timer();
#endif
  MALLOC (ind2, int, n_nodes) 
      for (i=0; i<n_nodes; i++) ind2[ind[i]] = i; 

  Adjust_EdgeList(n_nodes, n_ptnrs, el, ind, ind2);  

  FREE (ind2, int, n_nodes) 
#ifdef TIME
      printf("(Pure_time = %.2lf (+ %.2lf) - ", 
             tmp_time2 - tmp_time,
             get_timer() - tmp_time2);
#endif

}

void
CPack(int n_nodes, int n_ptnrs, int *ptnr, int *from, int *ind)
{
  int  i, j, k, p, n = 0;
  int  *taken, *ind2;
#ifdef TIME
  double tmp_time, tmp_time2;
#endif
#ifdef TIME
  tmp_time = get_timer();
#endif

  MALLOC (taken, int, n_nodes)
      bzero((void*) taken, n_nodes*sizeof(int));

  for (i=0; i<n_nodes; i++) {
    for (j=from[i]; j<from[i+1]; j++) { 
#ifdef CSTYLE
      p = ptnr[j];
#else
      p = ptnr[j] - 1;
#endif /* CSTYLE */
      if (taken[i] == 0) {ind[n++] = i; taken[i] = 1;}
      if (taken[p] == 0) {ind[n++] = p; taken[p] = 1;}
    }
  }
  if (n != n_nodes) { printf("something messed up! (%d != %d)\n", n_nodes, n); exit(1); }
  FREE (taken, int, n_nodes)

      /*
       *  ind[i]  : new -> old        [0...n_nodes-1]
       *  ind2[i] : old -> new        [0...n_nodes-1]
       */

#ifdef TIME
      tmp_time2 = get_timer();
#endif
  MALLOC (ind2, int, n_nodes) 
      for (i=0; i<n_nodes; i++) ind2[ind[i]] = i; 

  Adjust_PartnerList(n_nodes, n_ptnrs, ptnr, from, ind, ind2);  

  FREE (ind2, int, n_nodes) 
#ifdef TIME
      printf("(Pure_time = %.2lf (+ %.2lf) - ", 
             tmp_time2 - tmp_time,
             get_timer() - tmp_time2);
#endif

}

void
cpackprocess(int n_nodes, int n_edges, int (*el)[2], int n, long (*repos)[2])
{
  int *ind, i, siz, siz0 = 0; 
  byte *ptr, *tx = NULL;
  va_list ap;
  int *partners, *from;
#ifdef TIME
  double temp_lap, temp_lap2, temp_lap3;
#endif

  /* printf("CPACK ... \n"); */
#ifdef TIME
  temp_lap = get_timer();
#endif

  if (LR_index == NULL) MALLOC (ind, int, n_nodes)
  else ind = *LR_index;

#ifdef CONVERT
  /* convert edge list to partner list */
  MALLOC (partners, int, n_edges+n_nodes+1)
      from = partners + n_edges;
  Convert2Partner(n_nodes, n_edges, el, partners, from);
  CPack(n_nodes, n_edges, partners, from, ind); 
  Convert2Edgelist(n_nodes, n_edges, el, partners, from);
  FREE (partners, int, n_edges+n_nodes+1)
#else
      CPack0(n_nodes, n_edges, el, ind);
#endif

#ifdef TIME
  temp_lap2 = get_timer();
#endif

  N_REPOSITION()
      if (LR_index == NULL) FREE(ind, int, n_nodes)
      else *LR_index = ind;

#ifdef TIME
  temp_lap3 = get_timer();
  printf("CPACK time = %.2lf (%.2lf + %.2lf)\n", temp_lap3 - temp_lap, temp_lap2 - temp_lap, temp_lap3 - temp_lap2);
#endif
}

void
cpackprocess2(int n_nodes, int n_edges, int *partners, int *from, int n, long (*repos)[2])
{
  int *ind, i, siz, siz0 = 0; 
  byte *ptr, *tx = NULL;
  va_list ap;
#ifdef TIME
  double temp_lap, temp_lap2, temp_lap3;
#endif

  /* printf("CPACK ... \n"); */
#ifdef TIME
  temp_lap = get_timer();
#endif

  if (LR_index == NULL) MALLOC (ind, int, n_nodes)
  else ind = *LR_index;

  CPack(n_nodes, n_edges, partners, from, ind); 

#ifdef TIME
  temp_lap2 = get_timer();
#endif

  N_REPOSITION()
      if (LR_index == NULL) FREE(ind, int, n_nodes)
      else *LR_index = ind;

#ifdef TIME
  temp_lap3 = get_timer();
  printf("CPACK time = %.2lf (%.2lf + %.2lf)\n", temp_lap3 - temp_lap, temp_lap2 - temp_lap, temp_lap3 - temp_lap2);
#endif
}

/*========= BFS ===========================================*/

void
BFSearch0(int n_nodes, int n_ptnrs, int (*el)[2], int *ind) 
{
  int i, j, k, degree, max, root=0, p, cur, last, reverse = 1;
  int *ind2, *taken, n_taken, *expanded, n_expanded, *from;
#ifdef TIME
  double tmp_time, tmp_time2;
  double tmp_time3, tmp_time4=0;
#endif

#ifdef TIME
  tmp_time = get_timer();
#endif

  MALLOC (ind2, int, n_nodes)
      MALLOC (taken, int, 2*n_nodes+1)
      from = taken + n_nodes;
  expanded = ind2;  /* rename */
  bzero ((void*) taken, (2*n_nodes+1) * sizeof(int)); /* zero taken, from */
  bzero ((void*) expanded, n_nodes * sizeof(int));    /* zero expanded */

  for (i=0; i<n_ptnrs; i++)
#ifdef CSTYLE
    from[el[i][0]] ++;
#else
  from[el[i][0]-1] ++;
#endif /* CSTYLE */

  from[n_nodes] = n_ptnrs;
  for (i=n_nodes-1; i>=0; i--)
    from[i] = from[i+1] - from[i];

  n_taken = n_expanded = 0;
  cur = last = root;
  taken[cur] = 1;
  ind[n_taken++] = cur;
  while (1) {
    for (i = from[cur]; i<from[cur+1]; i++) {
#ifdef CSTYLE
      p = el[i][1];
#else
      p = el[i][1] - 1;
#endif /* CSTYLE */
      if (!taken[p]) { taken[p] = 1; ind[n_taken++] =p; }
    }

    if (n_taken == n_nodes) break;  /* out of while */
    expanded[cur] = 1;
    n_expanded++;
    for (i=n_expanded; i<n_taken; i++) {
      k = ind[i];
      if (!expanded[k]) { cur = k; break; } 
    }
    if (i == n_taken) { /* need a new root to start */ 
      for (i=last+1; i<n_nodes; i++) {
        if (taken[i] == 0) {
          cur = i;
          taken[cur] = 1;
          ind[n_taken++] = cur; 
          last = cur;
          break;
        }
      }
    }
  } 
  FREE (taken, int, 2*n_nodes+1)

#ifdef NON_REVERSE
      reverse = 0;
#endif
  if (reverse) {
    j = n_nodes >> 1;
    for (i=0; i<j; i++) {
      k = ind[i]; ind[i] = ind[n_nodes-i-1]; ind[n_nodes-i-1] = k;
    }
  }

  if (n_taken != n_nodes) printf("Something goes wrong! (n_taken = %d)\n", n_taken);
  /*
   *  ind[i]  : new -> old        [0...n_nodes-1]
   *  ind2[i] : old -> new        [0...n_nodes-1]
   */
#ifdef TIME
  tmp_time2 = get_timer();
#endif
  for (i=0; i<n_nodes; i++) ind2[ind[i]] = i; 

  Adjust_EdgeList(n_nodes, n_ptnrs, el, ind, ind2);  

  FREE (ind2, int, n_nodes) 
#ifdef TIME
      printf("(Pure_time = %.2lf (+ %.2lf) - ", 
             tmp_time2 - tmp_time,
             get_timer() - tmp_time2);
#endif
}

void
BFSearch(int n_nodes, int n_ptnrs, int *ptnr, int *from, int *ind) 
{
  int i, j, k, degree, max, root=0, p, cur, last, reverse = 1;
  int *ind2, *taken, n_taken, *expanded, n_expanded;
#ifdef TIME
  double tmp_time, tmp_time2;
  double tmp_time3, tmp_time4=0;
#endif

#ifdef TIME
  tmp_time = get_timer();
#endif

  MALLOC (ind2, int, n_nodes)
      MALLOC (taken, int, n_nodes)
      expanded = ind2;  /* rename */
  bzero ((void*) taken, n_nodes * sizeof(int));    /* zero taken */
  bzero ((void*) expanded, n_nodes * sizeof(int)); /* zero expanded */

  n_taken = n_expanded = 0;
  cur = last = root;
  taken[cur] = 1;
  ind[n_taken++] = cur;
  while (1) {
    for (i = from[cur]; i<from[cur+1]; i++) {
#ifdef CSTYLE
      p = ptnr[i];
#else
      p = ptnr[i] - 1;
#endif /* CSTYLE */
      if (!taken[p]) { taken[p] = 1; ind[n_taken++] =p; }
    }

    if (n_taken == n_nodes) break;  /* out of while */
    expanded[cur] = 1;
    n_expanded++;
    for (i=n_expanded; i<n_taken; i++) {
      k = ind[i];
      if (!expanded[k]) { cur = k; break; } 
    }
    if (i == n_taken) { /* need a new root to start */ 
      for (i=last+1; i<n_nodes; i++) {
        if (taken[i] == 0) {
          cur = i;
          taken[cur] = 1;
          ind[n_taken++] = cur; 
          last = cur;
          break;
        }
      }
    }
  } 
  FREE (taken, int, n_nodes)
#ifdef NON_REVERSE
      reverse = 0;
#endif
  if (reverse) {
    j = n_nodes >> 1;
    for (i=0; i<j; i++) {
      k = ind[i]; ind[i] = ind[n_nodes-i-1]; ind[n_nodes-i-1] = k;
    }
  }

  if (n_taken != n_nodes) printf("Something goes wrong! (n_taken = %d)\n", n_taken);
  /*
   *  ind[i]  : new -> old        [0...n_nodes-1]
   *  ind2[i] : old -> new        [0...n_nodes-1]
   */
#ifdef TIME
  tmp_time2 = get_timer();
#endif
  for (i=0; i<n_nodes; i++) ind2[ind[i]] = i; 

  Adjust_PartnerList(n_nodes, n_ptnrs, ptnr, from, ind, ind2);  

  FREE (ind2, int, n_nodes) 
#ifdef TIME
      printf("(Pure_time = %.2lf (+ %.2lf) - ", 
             tmp_time2 - tmp_time,
             get_timer() - tmp_time2);
#endif
}

void
bfsprocess(int n_nodes, int n_edges, int (*el)[2], int n, long (*repos)[2])
{
  int *ind, i, siz, siz0 = 0; 
  byte *ptr, *tx = NULL;
  va_list ap;
  int *partners, *from;
#ifdef TIME
  double temp_lap, temp_lap2, temp_lap3;
#endif

#ifdef NON_REVERSE
  /* printf("BFS ... \n"); */
#else
  /* printf("Reverse BFS ...\n"); */
#endif
#ifdef TIME
  temp_lap = get_timer();
#endif

  if (LR_index == NULL) MALLOC (ind, int, n_nodes)
  else ind = *LR_index;

#ifdef CONVERT
  /* convert edge list to partner list */
  MALLOC (partners, int, n_edges+n_nodes+1)
      from = partners + n_edges;
  Convert2Partner(n_nodes, n_edges, el, partners, from);
  BFSearch(n_nodes, n_edges, partners, from, ind); 
  Convert2Edgelist(n_nodes, n_edges, el, partners, from);
  FREE (partners, int, n_edges+n_nodes+1)
#else
      BFSearch0(n_nodes, n_edges, el, ind);
#endif

#ifdef TIME
  temp_lap2 = get_timer();
#endif

  N_REPOSITION()
      if (LR_index == NULL) FREE(ind, int, n_nodes)
      else *LR_index = ind;

#ifdef TIME
  temp_lap3 = get_timer();
  printf("BFS time = %.2lf (%.2lf + %.2lf)\n", temp_lap3 - temp_lap, temp_lap2 - temp_lap, temp_lap3 - temp_lap2);
#endif
}

void
bfsprocess2(int n_nodes, int n_edges, int *partners, int *from, int n, long (*repos)[2])
{
  int *ind, i, siz, siz0 = 0; 
  byte *ptr, *tx = NULL;
  va_list ap;
#ifdef TIME
  double temp_lap, temp_lap2, temp_lap3;
#endif

#ifdef NON_REVERSE
  /* printf("BFS ... \n"); */
#else
  /* printf("Reverse BFS ...\n"); */
#endif
#ifdef TIME
  temp_lap = get_timer();
#endif

  if (LR_index == NULL) MALLOC (ind, int, n_nodes)
  else ind = *LR_index;

  BFSearch(n_nodes, n_edges, partners, from, ind); 

#ifdef TIME
  temp_lap2 = get_timer();
#endif

  N_REPOSITION()
      if (LR_index == NULL) FREE(ind, int, n_nodes)
      else *LR_index = ind;

#ifdef TIME
  temp_lap3 = get_timer();
  printf("BFS time = %.2lf (%.2lf + %.2lf)\n", temp_lap3 - temp_lap, temp_lap2 - temp_lap, temp_lap3 - temp_lap2);
#endif
}


/*========= ADAPTIVE ===========================================*/
#ifdef RANDOM_ADAPT_ORG
static int rand_num[50] = { /* pre-generated random numbers */
  53, 14, 27,  2, 31, 85, 30, 11, 67, 50, 
  7, 39, 29, 16, 79,  3, 99, 90, 40, 26,
  32, 29, 82, 64, 15, 61, 73, 75, 38, 23, 
  40, 43, 37, 68, 46, 69,  5, 28, 80, 24,
  78, 39, 63, 59,  7, 42, 63, 59, 32, 55};
#endif

int
get_rand_node (int n_nodes, int *taken)
{
  int r;

  do {
    r = random() % n_nodes;
  } while (taken[r]); 
  taken[r] = 1;

  return r;
}

void
adapt_graph(int num, ...)
{
  int i, ntimes, r1, r2, *ind=NULL, *ind2=NULL, *p;
  byte *ptr, *tx=NULL;
  int  siz, siz0 = 0, coordi_swap;
  int n_nodes, n_edges, (*el)[2], n;
  double percent;
  long (*repos)[2];
  va_list ap;

  va_start(ap, num);
  percent = va_arg(ap, double);
  n_nodes = va_arg(ap, int);
  n_edges = va_arg(ap, int);
  el = (int (*)[2]) va_arg(ap, int*);
  n = va_arg(ap, int);

#ifdef RANDOM_ADAPT_ORG
  {
    static int i=0, ipercent;

    ipercent = (int) percent; /* upper limit */
    percent = (double) (rand_num[i%50] % ipercent + 1);
    i++;
    printf("[%.0lf%% nodes random swapping]\n", percent);
  }    
#endif
  MALLOC2 (repos, long, n)
      for (i=0; i<n; i++) {
        repos[i][0] = (long) va_arg(ap, byte*);
        repos[i][1] = (long) va_arg(ap, int);
      }
  va_end(ap);

  ntimes = (int) (n_nodes * percent/100.0);
  ntimes >>= 1;

  MALLOC (ind2, int, 2*n_nodes)
      ind = ind2 + n_nodes;
  for (i=0; i<n_nodes; i++) ind2[i] = i;
  bzero ((void*)ind, n_nodes * sizeof(int));

  /* printf ("%d swaps = %2.1lf%% of %d nodes\n", ntimes,percent,n_nodes); */
  for (i=0; i<ntimes; i++) {
    r1 = get_rand_node(n_nodes, ind);
    r2 = get_rand_node(n_nodes, ind);

    ind2[r1] = r2;
    ind2[r2] = r1;
  }

  for (i=0; i<n_nodes; i++) ind[ind2[i]] = i; 

  Adjust_EdgeList (n_nodes, n_edges, el, ind, ind2);
  N_REPOSITION()

      FREE (ind2, int, 2*n_nodes)
      FREE (repos, long, n)
}

void
adapt_graph2(int num, ...)
{
  int i, j, k, ntimes, r1, r2, *ind2, *p;
  int *t_from, *t_ptnr, *ind;
  byte *ptr, *tx=NULL;
  int  siz, siz0 = 0;
  int n_nodes, n_edges, *partners, *from, n;
  double percent;
  long (*repos)[2];
  va_list ap;

  va_start(ap, num);
  percent = va_arg(ap, double);
  n_nodes = va_arg(ap, int);
  n_edges = va_arg(ap, int);
  partners = va_arg(ap, int*);
  from = va_arg(ap, int*);
  n = va_arg(ap, int);

#ifdef RANDOM_ADAPT_ORG
  {
    static int i=0, ipercent;

    ipercent = (int) percent; /* upper limit */
    percent = (double) (rand_num[i%50] % ipercent + 1);
    i++;
    printf("[%.0lf%% nodes random swapping]\n", percent);
  }    
#endif
  MALLOC2 (repos, long, n)
      for (i=0; i<n; i++) {
        repos[i][0] = (long) va_arg(ap, byte*);
        repos[i][1] = (long) va_arg(ap, int);
      }
  va_end(ap);

  ntimes = (int) (n_nodes * percent/100.0);
  ntimes >>= 1;

  MALLOC (ind2, int, 2*n_nodes)
      ind = ind2 + n_nodes;
  for (i=0; i<n_nodes; i++) ind2[i] = i;
  bzero ((void*)ind, n_nodes * sizeof(int));

  /* printf ("%d swaps = %2.1lf%% of %d nodes\n", ntimes,percent,n_nodes); */
  for (i=0; i<ntimes; i++) {
    r1 = get_rand_node(n_nodes, ind);
    r2 = get_rand_node(n_nodes, ind);

    ind2[r1] = r2;
    ind2[r2] = r1;
  }

  for (i=0; i<n_nodes; i++) ind[ind2[i]] = i; 

  /*
   *  ind[i]  : new -> old        [0...n_nodes-1]
   *  ind2[i] : old -> new        [0...n_nodes-1]
   */

  Adjust_PartnerList (n_nodes, n_edges, partners, from, ind, ind2);
  N_REPOSITION()

      FREE (ind2, int, 2*n_nodes)
      FREE (repos, long, n)
}


#ifdef STATS
#define RUD(a, k) { \
  if (last_app[a] >= 0) { \
    dist = (dist * n_reuse)/(double)(n_reuse+1) + (double)(k-last_app[a])/(n_reuse+1); \
    n_reuse++;  \
  } \
  last_app[a] = k; \
}
#define DIST_LIMIT 1000
#define GROUP      4
#ifdef RDIST
#define RUD2(a, k) { 				\
  int l = a / GROUP;				\
  if (last_app[l] >= 0) {			\
    dist =  (double)(k -last_app[l]); 	\
    if (dist <= DIST_LIMIT) {		\
      tot_dist +=  dist;			\
      n_reuse++;  			\
    }					\
    else {					\
      tot_dist2 += dist;			\
      n_reuse2++;				\
    }					\
  } 						\
  last_app[l] = k;				\
}
#else
#define RUD2(a, k) { 				\
  int l = a / GROUP;				\
  if (last_app[l] >= 0) {			\
    dist =  (double)(k -last_app[l]); 	\
    if (dist <= DIST_LIMIT) {		\
      n_reuse++;  			\
    }					\
  } 						\
  last_app[l] = k;				\
}
#endif
double reuse_distance(int n_nodes, int *partners, int *from)
{
  int i, j, k, *last_app, n_reuse; 
  double dist, tot_dist, avg_dist;
  double tot_dist2, avg_dist2, begin_time;
  int n_reuse2;

#ifdef TIME
  begin_time = get_timer();
#endif
  MALLOC (last_app, int, n_nodes)
      for (i=0; i<n_nodes; i++) last_app[i] = -1;

  n_reuse = 0; tot_dist = 0.0; 
  n_reuse2 = 0; tot_dist2 = 0.0; 
  for (k=i=0; i<n_nodes; i++) {
    for (j=from[i]; j<from[i+1]; j++) {
      RUD2(i, k);           k++; 
      RUD2(partners[j], k); k++;
    }
  } 
  FREE (last_app, int, n_nodes)

#ifdef TIME
      printf("Reuse_time = %.2lf\n", get_timer() - begin_time);
#endif
#ifdef RDIST
  if (n_reuse != 0) avg_dist = tot_dist / n_reuse;
  else  avg_dist = 0.0;
  printf("tot_dist = %le  n_reuse = %d  avg-dist = %le\n", tot_dist, n_reuse, avg_dist); 
  if (n_reuse2 != 0) avg_dist2 = tot_dist2 / n_reuse2;
  else  avg_dist2 = 0.0;
  printf("tot_dist2 = %le  n_reuse2 = %d  avg-dist2 = %le\n", tot_dist2, n_reuse2, avg_dist2); 
  return (avg_dist);
#else
  printf("n_reuse = %d\n", n_reuse); 
  return (n_reuse);
#endif
}
#endif

#ifdef BLOCK_ADAPTIVE

#define roundup(r) ((int)(r) + 1)  /* 2.0 -> 3,  2.3 -> 3, 2.7 ->3 */

int *
NodeBlocking (double (*xyz)[3], int n_nodes, double side, int *n_xyz)
{
  printf("Node Blocking...\n");
  printf("DBL_MAX: %f\n", DBL_MAX);
  double max_xyz[3], min_xyz[3];
  int i, j, k, n_blk, n_x, n_y, n_z, n_th;
  int *nodes, *start, *nodes2, *start2;

  max_xyz[0] = max_xyz[1] = max_xyz[2] = -DBL_MAX;
  min_xyz[0] = min_xyz[1] = min_xyz[2] = DBL_MAX;

  for (i=0; i<n_nodes; i++) {
    if (xyz[i][0] > max_xyz[0]) max_xyz[0] = xyz[i][0];	
    if (xyz[i][1] > max_xyz[1]) max_xyz[1] = xyz[i][1];	
    if (xyz[i][2] > max_xyz[2]) max_xyz[2] = xyz[i][2];	

    if (xyz[i][0] < min_xyz[0]) min_xyz[0] = xyz[i][0];	
    if (xyz[i][1] < min_xyz[1]) min_xyz[1] = xyz[i][1];	
    if (xyz[i][2] < min_xyz[2]) min_xyz[2] = xyz[i][2];	
  }

  /* last fractions will be merged to last cubes */
  n_xyz[0] = (int)((max_xyz[0] - min_xyz[0]) / side);
  n_xyz[1] = (int)((max_xyz[1] - min_xyz[1]) / side);
  n_xyz[2] = (int)((max_xyz[2] - min_xyz[2]) / side);

  n_blk = n_xyz[0] * n_xyz[1] * n_xyz[2]; /* need n cubes */

  printf("MAX[%lf %lf %lf] MIN[%lf %lf %lf]\n", 
         max_xyz[0], max_xyz[1], max_xyz[2],
         min_xyz[0], min_xyz[1], min_xyz[2]);
  printf("n_blk = %d  [%d %d %d]\n", n_blk, n_xyz[0], n_xyz[1], n_xyz[2]);


  MALLOC (nodes, int, n_nodes + n_blk+1)
      start = nodes + n_nodes;
  MALLOC (nodes2, int, n_nodes + n_blk+1)
      start2 = nodes2 + n_nodes;
  bzero ((void *)start, (n_blk+1)*sizeof(int));
  for (i=0; i<n_nodes; i++) {
    n_x = (int)((xyz[i][0] - min_xyz[0]) / side);
    n_y = (int)((xyz[i][1] - min_xyz[1]) / side);
    n_z = (int)((xyz[i][2] - min_xyz[2]) / side);

    if (n_x == n_xyz[0])  n_x--; 
    if (n_y == n_xyz[1])  n_y--; 
    if (n_z == n_xyz[2])  n_z--; 

    n_th = n_z * (n_xyz[0]*n_xyz[1]) + n_y * n_xyz[0] + n_x;

    nodes2[i] = n_th;
    if (n_th >= n_blk) printf("Something is going wrong!\n");
    start[n_th]++;
  }


  start[n_blk] = n_nodes;
  for (i=n_blk-1; i>=0; i--) {
    start[i] = start[i+1] - start[i]; 	
    start2[i] = start[i];
  }

  for (i=0; i<n_nodes; i++) {
    n_th = nodes2[i];
    k = start2[n_th];
    start2[n_th]++; 
    nodes[k] = i;
  }

  FREE (nodes2, int, n_nodes + n_blk+1)
      return nodes;
}
#endif 

#define NO_OPT    0
#define RCB_OPT   1
#define METIS_OPT 2
#define GPART_OPT 3
#define RBFS_OPT  4
#define CPACK_OPT 5
static int __LR_choice__ = 0;
static int LR_adaptive = 0;	   /* set from customer code */
static int LR_count = 0; 	   /* initialized to 0 */
static double LR_times[10];
static double LR_ovrhd[10];
static double LR_b, LR_Ov, LR_m, LR_t;
static int LR_AdaptiveDecided = 0; /* turn on after costmodel */
static int LR_overriden = 0;       /* turn when users set LR_opt_freq */
static int LR_n_iter;
static double LR_opt_freq;
static double LR_last_opt_freq;
static int LR_opt_called = 0;      /* consider last two sampling iterations */
static int LR_iter_cnt = 0;
static int LR_sampling_iter;
static int LR_orig_sample;
static int LR_param_set = 0; 
static double before_opt_time = 0.0;
static double after_opt_time = 0.0;
static double lap_time = 0.0;
static double prev_lap_time = 0.0;
static double LR_ac_time = 0.0;
static int LR_apply;
static int LR_opt_called_since_GPART;
static double gpart_time, gpart_ovrhd, gpart_m;
static int LR_sample[10] = {NO_OPT,NO_OPT,RCB_OPT,GPART_OPT,RBFS_OPT,NO_OPT};
static int LR_is_periodic = 0;
static int LR_last_iter = 0;
static int LR_last_lap;
static int LR_apply_next_change = 0;
int LR_apply_cnt = 0;

#define dabs(a) (((a) >= 0.0)? (a) : -(a))

void
LR_set_coordinate(double (*xyz)[3])
{
  __LR_coordinate__ = xyz;
}

void
LR_set_choice(int optimization)
{
  __LR_choice__ = optimization;
}

void 
LR_set_adaptive(int n_iter, int adapt_freq)
{
  if (adapt_freq >= n_iter) LR_adaptive = 0;
  LR_adaptive = 1;
  LR_n_iter = (int)((double)n_iter/(double)adapt_freq + 0.5);
  /* adaptive unit iteration */
  /* printf("t = %d  (%d/%d)\n", LR_n_iter, n_iter, adapt_freq); */
}

void
LR_set_override_adaptive(int opt_freq) 
{
  LR_adaptive = 1;
  LR_AdaptiveDecided = 1;
  LR_overriden = 1;
  LR_opt_freq = (double)opt_freq;
}

void
LR_set_parameters(int n_nodes, int n_edges, int unit_sz)
{
  int level, npart, psize, npass, nshift;
  int L1_cache, part_sz; 

  LR_param_set = 1;
  L1_cache = L1_CACHE;

  /* for RCB, METIS */
  npart = 1; level = 0;
  part_sz = (int)((double)n_nodes / (double)npart + 0.5) * unit_sz;
  while (part_sz > L1_cache) {
    npart <<= 1;
    level++;
    part_sz = (int)((double)n_nodes / (double)npart + 0.5) * unit_sz;
  } 
  LEVEL_arg = level;
  npart_arg = npart;

  /* for GPART */
  init_pSize = 4; 
  psize = init_pSize;  /* predefined start size */
  nShift = 3; 	 /* predefined */

  part_sz = psize * unit_sz;
  npass = 1;
  while (part_sz < L1_cache) {
    npass++;
    psize <<=nShift; 
    part_sz = psize * unit_sz;
  }
  nPass = npass;

  printf("[PARAMETERS] (Level,nPart)=(%d,%d), (nPass,nShift)=(%d,%d)\n",
         LEVEL_arg, npart_arg, nPass, nShift); 
}

void 
LR_reorder_map(int **idx)
{
  LR_index = idx;
}

void 
LR_AdaptiveCostModel(double b, double Ov, double m, double t)
{
  double n0;
  double tmp_time;

#ifdef TIME
  tmp_time = get_timer();
#endif 
  LR_last_opt_freq = LR_opt_freq;

  if (m < 0.0) n0 = 1;
  else n0  = t * sqrt(m/(2*Ov));
  if (n0 < 1.0)  n0 = 1.0;
  if (n0 == 0) LR_opt_freq = t + 1;
  else  LR_opt_freq = t/n0;

#ifdef LAP_PRINT
  printf("    <Choice:%d> n0=%.2lf b=%.2lf m=%.2lf Ov=%.2lf t=%.2lf freq=%.2lf (time=%.3lf ms)\n", 
         GPART_OPT, n0, b, m, Ov, t, LR_opt_freq, (get_timer()-tmp_time)/1000.0); 
#endif
}


void
LR_AdaptiveDecision()
{
  static int cnt = 0;
  double n;

  before_opt_time = get_timer();
  prev_lap_time = lap_time;
  lap_time = before_opt_time - after_opt_time;
  if (LR_iter_cnt == LR_last_iter) lap_time -= LR_ac_time;
#ifdef LAP_PRINT
  printf ("    Lap_time = %.2lf (%d,%d,case%d)\n", lap_time, cnt++, LR_opt_called,LR_count);
#endif

  if (LR_overriden) return;

  if (LR_count != 0) LR_times[LR_count-1] = prev_lap_time;

  switch (LR_count) {
    case 0: __LR_choice__ = NO_OPT; /* no Opt */
            break;
    case 1: __LR_choice__ = GPART_OPT; /* GPART */
            LR_opt_called = 0;
            break;
    case 2: __LR_choice__ = NO_OPT; /* no Opt */
            LR_b = lap_time;
            LR_Ov = LR_ovrhd[1];
            LR_opt_called = 1;
            break;
    case 3:
            LR_m = lap_time - prev_lap_time;
            LR_t = LR_n_iter - 1;

            __LR_choice__ = GPART_OPT;

            LR_AdaptiveCostModel(LR_b, LR_Ov, LR_m, LR_t);
            LR_AdaptiveDecided = 1;
            LR_opt_called = 2;
            LR_opt_called_since_GPART = 1;
            break;
    case 4: /* GPART_OPT called last lap */
            LR_b  = lap_time;
            LR_Ov = LR_ovrhd[3];
            LR_t -= (LR_opt_called_since_GPART+1);
            LR_AdaptiveCostModel(LR_b, LR_Ov, LR_m, LR_t);
            LR_opt_called = 1;
            LR_opt_called_since_GPART = 0;
            break;
    case 5: /* GPART_OPT called previous lap before last lap */
            LR_opt_called_since_GPART++;
            LR_m = lap_time - prev_lap_time; 
            LR_AdaptiveCostModel(LR_b, LR_Ov, LR_m, LR_t);
            break;
    case 6: /* GPART_OPT not called within 2 last laps */
            n = (double)(LR_opt_called_since_GPART++);
            LR_m = (n*n*LR_m + lap_time+prev_lap_time - 2*LR_b)/((n+1)*(n+1));
            LR_AdaptiveCostModel(LR_b, LR_Ov, LR_m, LR_t);
            break;
    default:
            printf("Unreachable code\n");
            exit(0);
  } 
}

void
LR_access_change_begin()
{
  LR_ac_time = get_timer();
}

void
LR_access_changed()
{
  int lap;
  double t1, t2;

  lap = LR_iter_cnt - LR_last_iter;
  LR_last_iter = LR_iter_cnt;
  if (LR_last_lap == 0)  /* be optimistic - assume periodic first */
    LR_is_periodic = 1;
  else {
    t1 = ((double)LR_last_lap) * 0.85;
    t2 = ((double)LR_last_lap) * 1.15;
    if (t1 <= lap && lap <= t2) 
      LR_is_periodic = 1;
    else LR_is_periodic = 0;
  }

  LR_last_lap = lap;
  LR_ac_time = get_timer() - LR_ac_time;  /* time elapsed rebuilding access */

  printf("Periodic = (%d %c) (%.2lf %.2lf)\n", lap, LR_is_periodic?'T':'F', t1, t2);
}

int
Nearest0 (int iteration, double freq)
{
  int f;

  f = (int) freq;
  if (iteration % f == 0) return 1;
  return 0;
}

int
Nearest (int iteration, double freq)
{
  double f0, f, f1, i0, i, i1;
  static int LR_last = 0;
  int ret = 0;

  if (iteration == 0) {
    LR_last = 1;
    return 1;
  }

  f0 = ((double)(iteration-1)) / freq;
  f = ((double)iteration) / freq;
  f1 = ((double)(iteration+1)) / freq;

  f0 = modf(f0, &i0); 
  f = modf(f, &i); 
  f1 = modf(f1, &i1); 

  /* printf ("(%.2lf %.2lf %.2lf) last=%d)\n", f0+i0, f+i, f1+i1, LR_last); */
  if (i0 < i) {
    if ((1.0 - f0) > f)     ret = 1;
    else if (LR_last == 0)  ret = 1;
  }
  else if (i < i1 && (1.0 - f) < f1)     ret = 1;

  if (LR_last_opt_freq > LR_opt_freq) {
    f = (LR_opt_called_since_GPART+1) / freq;
    /* printf("f=%lf\n", f); */
    if (f >= 1.0) ret = 1;
  }

  LR_last = ret;
  return  ret;
}

void
LocalityReordering (int num, ...)
{
  va_list ap;
  int n_nodes, n_edges, (*el)[2], n, unit_sz, i, apply;
  long  (*repos)[2];

#if defined(ADAPTIVE_ORG) || defined(ADAPTIVE_GP)
  LR_AdaptiveDecision();
#endif

#ifdef RCB
  __LR_choice__ = 1;
#elif METIS
  __LR_choice__ = 2;
#elif GPART
  __LR_choice__ = 3;
#elif BFS
  __LR_choice__ = 4;
#elif CPACK
  __LR_choice__ = 5;
#endif

  if (LR_adaptive == 0) apply = 1;
  else if (LR_AdaptiveDecided == 0) {
    apply = 1;
  }
  else { /* LR_adaptive && LR_AdaptiveDecided */
    if (LR_opt_freq == 0.0) {
      printf("Invalid Optimization frequency (%.2lf)\n", LR_opt_freq);
      exit(0);
    }
    if (Nearest(LR_opt_called, LR_opt_freq)) apply = 1;
    else 	apply = 0;

    if (LR_is_periodic) {
      i = LR_iter_cnt - LR_last_iter;

      if (i == 0) { /* access just changed */
        if (LR_apply_next_change) {
          apply = 1;
          LR_apply_next_change = 0;
        }
      }
      else if (apply == 1) {
        printf ("    reordering @%d - ", i);
        if (i >= (LR_last_lap>>1)) {
          apply = 0;
          LR_apply_next_change = 1;
          printf("postphoned\n");
        }
        else {
          printf("granted\n");
        }
      }
    }

    if (apply == 1) LR_count = 3;
    else {
      if (LR_count == 3 || LR_count == 6) LR_count = 5;
    } 
    LR_opt_called++;
  }

  if (apply) {

    if (__LR_choice__ != 0) {
      va_start(ap, num);
      n_nodes = va_arg(ap, int);
      n_edges = va_arg(ap, int);
      el = (int (*)[2]) va_arg(ap, int*);
      unit_sz = va_arg(ap, int);
      n = va_arg(ap, int);

      if (n == 0) repos = NULL;
      else {
        MALLOC2 (repos, long, n)
            for (i=0; i<n; i++) {
              repos[i][0] = (long) va_arg(ap, byte*);
              repos[i][1] = (long) va_arg(ap, int);
            }
      }
      va_end(ap);
      if (LR_param_override == 0 && LR_param_set == 0) 
        LR_set_parameters(n_nodes, n_edges, unit_sz);

      LR_apply_cnt++;
    }

    switch (__LR_choice__) {
      case 0:	/* ALL */
        printf("No reordering!\n");
        break;
      case 1:	/* RCB */
        rcbprocess(n_nodes, n_edges, el, n, repos);
        break;
      case 2:	/* METIS */
        metisprocess(n_nodes, n_edges, el, n, repos);
        break;
      case 3:	/* GPART */
        gpartprocess(n_nodes, n_edges, el, n, repos);
        break;
      case 4:	/* BFS */
        bfsprocess(n_nodes, n_edges, el, n, repos);
        break;
      case 5:	/* CPACK */
        cpackprocess(n_nodes, n_edges, el, n, repos);
        break;
      default:
        printf("Illegal Locality Reordering is set\n");
        exit(0);
        break;
    }
    if (__LR_choice__ != 0 && n != 0) FREE2 (repos, int, n)
  }

#ifdef TIME /* must define TIME to use this function */
  after_opt_time = get_timer();
#endif
  if (LR_adaptive) 
    LR_ovrhd[LR_count++] = after_opt_time - before_opt_time;

  LR_iter_cnt++;
}

void
LocalityReordering2 (int num, ...)
{
  va_list ap;
  int n_nodes, n_edges, *partners, *from, n, unit_sz, i, apply;
  long  (*repos)[2];
  static int apply_cnt;

#if defined(ADAPTIVE_ORG) || defined(ADAPTIVE_GP)
  LR_AdaptiveDecision();
#endif

#ifdef RCB
  __LR_choice__ = 1;
#elif METIS
  __LR_choice__ = 2;
#elif GPART
  __LR_choice__ = 3;
#elif BFS
  __LR_choice__ = 4;
#elif CPACK
  __LR_choice__ = 5;
#endif

  if (LR_adaptive == 0) apply = 1;
  else if (LR_AdaptiveDecided == 0) {
    apply = 1;
  }
  else { /* LR_adaptive && LR_AdaptiveDecided */
    if (LR_opt_freq == 0.0) {
      printf("Invalid Optimization frequency (%.2lf)\n", LR_opt_freq);
      exit(0);
    }
    if (Nearest(LR_opt_called, LR_opt_freq)) apply = 1;
    else    apply = 0;

    if (LR_is_periodic) {
      i = LR_iter_cnt - LR_last_iter;

      if (i == 0) { /* access just changed */
        if (LR_apply_next_change) {
          apply = 1;
          LR_apply_next_change = 0;
        }
      }
      else if (apply == 1) {
        printf ("    reordering @%d - ", i);
        if (i >= (LR_last_lap>>1)) {
          apply = 0;
          LR_apply_next_change = 1;
          printf("postphoned\n");
        }
        else {
          printf("granted\n");
        }
      }
    }

    if (apply == 1) LR_count = 3;
    else {
      if (LR_count == 3 || LR_count == 6) LR_count = 5;
    }
    LR_opt_called++;
  }

  if (apply) {

    if (__LR_choice__ != 0) {
      va_start(ap, num);
      n_nodes = va_arg(ap, int);
      n_edges = va_arg(ap, int);
      partners = va_arg(ap, int*);
      from = va_arg(ap, int*);
      unit_sz = va_arg(ap, int);
      n = va_arg(ap, int);

      if (n == 0) repos = NULL;
      else {
        MALLOC2 (repos, long, n)
            for (i=0; i<n; i++) {
              repos[i][0] = (long) va_arg(ap, byte*);
              repos[i][1] = (long) va_arg(ap, int);
            }
      }
      va_end(ap);
      if (LR_param_override == 0 && LR_param_set == 0) 
        LR_set_parameters(n_nodes, n_edges, unit_sz);

      LR_apply_cnt++;
    }

    switch (__LR_choice__) {
      case 0:	/* ALL */
        printf("No reordering!\n");
        break;
      case 1:	/* RCB */
        rcbprocess2(n_nodes, n_edges, partners, from, n, repos);
        break;
      case 2:	/* METIS */
        metisprocess2(n_nodes, n_edges, partners, from, n, repos);
        break;
      case 3:	/* GPART */
        gpartprocess2(n_nodes, n_edges, partners, from, n, repos);
        break;
      case 4:	/* BFS */
        bfsprocess2(n_nodes, n_edges, partners, from, n, repos);
        break;
      case 5:	/* CPACK */
        cpackprocess2(n_nodes, n_edges, partners, from, n, repos);
        break;
      default:
        printf("Illegal Locality Reordering is set\n");
        exit(0);
        break;
    }
    if (__LR_choice__ != 0 && n != 0) FREE2 (repos, int, n)
  }

#ifdef TIME /* must define TIME to use this function */
  after_opt_time = get_timer();
#endif
  if (LR_adaptive) 
    LR_ovrhd[LR_count++] = after_opt_time - before_opt_time;

  LR_iter_cnt++;
}

void
NA_LocalityReordering (int num, ...)
{
  va_list ap;
  int n_nodes, n_edges, (*el)[2], n, unit_sz, i, apply;
  long  (*repos)[2];

#ifdef RCB
  __LR_choice__ = 1;
#elif METIS
  __LR_choice__ = 2;
#elif GPART
  __LR_choice__ = 3;
#elif BFS
  __LR_choice__ = 4;
#elif CPACK
  __LR_choice__ = 5;
#endif

  if (__LR_choice__ != 0) {
    va_start(ap, num);
    n_nodes = va_arg(ap, int);
    n_edges = va_arg(ap, int);
    el = (int (*)[2]) va_arg(ap, int*);
    unit_sz = va_arg(ap, int);
    n = va_arg(ap, int);

    if (n == 0) repos = NULL;
    else {
      MALLOC2 (repos, long, n)
          for (i=0; i<n; i++) {
            repos[i][0] = (long) va_arg(ap, byte*);
            repos[i][1] = (long) va_arg(ap, int);
          }
    }
    va_end(ap);
    if (LR_param_override == 0 && LR_param_set == 0) 
      LR_set_parameters(n_nodes, n_edges, unit_sz);

  }

  switch (__LR_choice__) {
    case 0:	/* ALL */
      printf("No reordering!\n");
      break;
    case 1:	/* RCB */
      rcbprocess(n_nodes, n_edges, el, n, repos);
      break;
    case 2:	/* METIS */
      metisprocess(n_nodes, n_edges, el, n, repos);
      break;
    case 3:	/* GPART */
      gpartprocess(n_nodes, n_edges, el, n, repos);
      break;
    case 4:	/* BFS */
      bfsprocess(n_nodes, n_edges, el, n, repos);
      break;
    case 5:	/* CPACK */
      cpackprocess(n_nodes, n_edges, el, n, repos);
      break;
    default:
      printf("Illegal Locality Reordering is set\n");
      exit(0);
      break;
  }
  if (__LR_choice__ != 0 && n != 0) FREE2 (repos, int, n)
}

void
NA_LocalityReordering2 (int num, ...)
{
  va_list ap;
  int n_nodes, n_edges, *partners, *from, n, unit_sz, i, apply;
  long  (*repos)[2];
  static int apply_cnt;

#ifdef RCB
  __LR_choice__ = 1;
#elif METIS
  __LR_choice__ = 2;
#elif GPART
  __LR_choice__ = 3;
#elif BFS
  __LR_choice__ = 4;
#elif CPACK
  __LR_choice__ = 5;
#endif

  if (__LR_choice__ != 0) {
    va_start(ap, num);
    n_nodes = va_arg(ap, int);
    n_edges = va_arg(ap, int);
    partners = va_arg(ap, int*);
    from = va_arg(ap, int*);
    unit_sz = va_arg(ap, int);
    n = va_arg(ap, int);

    /* printf("(N,E)=(%d,%d) n=%d\n", n_nodes, n_edges,n); */
    if (n == 0) repos = NULL;
    else {
      MALLOC2 (repos, long, n)
          for (i=0; i<n; i++) {
            repos[i][0] = (long) va_arg(ap, byte*);
            repos[i][1] = (long) va_arg(ap, int);
          }
    }
    va_end(ap);
    if (LR_param_override == 0 && LR_param_set == 0) 
      LR_set_parameters(n_nodes, n_edges, unit_sz);

  }

  switch (__LR_choice__) {
    case 0:	/* ALL */
      printf("No reordering!\n");
      break;
    case 1:	/* RCB */
      rcbprocess2(n_nodes, n_edges, partners, from, n, repos);
      break;
    case 2:	/* METIS */
      metisprocess2(n_nodes, n_edges, partners, from, n, repos);
      break;
    case 3:	/* GPART */
      gpartprocess2(n_nodes, n_edges, partners, from, n, repos);
      break;
    case 4:	/* BFS */
      bfsprocess2(n_nodes, n_edges, partners, from, n, repos);
      break;
    case 5:	/* CPACK */
      cpackprocess2(n_nodes, n_edges, partners, from, n, repos);
      break;
    default:
      printf("Illegal Locality Reordering is set\n");
      exit(0);
      break;
  }
  if (__LR_choice__ != 0 && n != 0) FREE2 (repos, int, n)
}
