#ifndef MISC_H
#define MISC_H

#include <stdlib.h>
#include <strings.h>
#include <stdarg.h>

extern int LEVEL_arg;
extern int DIMENSION;
extern int npart_arg;
extern int nPass, nShift;
extern int LR_param_override;
extern int LR_apply_cnt;

#define min(x,y) ((x)<(y)?(x):(y))
#define max(x,y) ((x)>(y)?(x):(y))
#define max3(x,y,z) (max(max(x,y), z))

#ifdef SIMULATE

#define MALLOC(p, typ, sz)  { if (ma_cur_ptr+(sz)*sizeof(typ) <= ma_end_ptr) { \
		p = (typ *)ma_cur_ptr; ma_cur_ptr += (sz)*sizeof(typ); \
		     } else  { printf("No more space!!\n"); exit(1); }}
#define MALLOC2(p, typ, sz) { if (ma_cur_ptr+(sz)*sizeof(typ)*2 <= ma_end_ptr) { \
		p = (typ(*)[2])ma_cur_ptr; ma_cur_ptr += (sz)*sizeof(typ)*2; \
		     } else  { printf("No more space!!\n"); exit(1); }}
#define MALLOC3(p, typ, sz) { if (ma_cur_ptr+(sz)*sizeof(typ)*3 <= ma_end_ptr) { \
		p = (typ(*)[3])ma_cur_ptr; ma_cur_ptr += (sz)*sizeof(typ)*3; \
		     } else  { printf("No more space!!\n"); exit(1); }}
#define FREE(x, typ, sz)  { if (x != NULL && ma_postponed == NULL) \
				ma_cur_ptr -= (sz)*sizeof(typ); }
#define FREE2(x, typ, sz) { if (x != NULL && ma_postponed == NULL) \
				ma_cur_ptr -= (sz)*sizeof(typ)*2; }
#define FREE3(x, typ, sz) { if (x != NULL && ma_postponed == NULL) \
				ma_cur_ptr -= (sz)*sizeof(typ)*3; }
#define POSTPONE_FREE()  ma_postponed = ma_cur_ptr; 
#define FREE_NOW()   	 ma_cur_ptr = ma_postponed; ma_postponed = NULL;

#else

#define MALLOC(p, typ, sz)  { if (!(p = (typ *)malloc((sz) * sizeof(typ)))) { \
                                printf("No more space!!\n"); exit(1); } }
#define MALLOC2(p, typ, sz) { if (!(p = (typ(*)[2])malloc((sz) * sizeof(typ) * 2))) { \
                                printf("No more space!!\n"); exit(1); } }
#define MALLOC3(p, typ, sz) { if (!(p = (typ(*)[3])malloc((sz) * sizeof(typ) * 3))) { \
                                printf("No more space!!\n"); exit(1); } }

#define FREE(x, typ, sz)  free(x);
#define FREE2(x, typ, sz) free(x);
#define FREE3(x, typ, sz) free(x);

#endif /* SIMULATE */


#define REPOSITION(tmp, x, n_nodes, u_sz, ind) { \
    int i, j;					\
    bcopy (x, tmp, n_nodes*u_sz);		\
    if (u_sz % sizeof(double) == 0) {		\
      double *dtmp=(double*)tmp, *dx=(double*)x;\
      int du_sz = u_sz/sizeof(double); 		\
      if (du_sz == 1) {				\
        for (i=0; i<n_nodes; i++)		\
	  dx[i] = dtmp[ind[i]];			\
      } else {					\
        for (i=0; i<n_nodes; i++)		\
        for (j=0; j<du_sz; j++)			\
	  dx[i*du_sz+j] = dtmp[ind[i]*du_sz+j];	\
      }						\
    }						\
    else if (u_sz % sizeof(int) == 0) {		\
      int *itmp = (int*)tmp, *ix = (int*)x; 	\
      int iu_sz = u_sz/sizeof(int); 		\
      if (iu_sz == 1) {				\
        for (i=0; i<n_nodes; i++)		\
	  ix[i] = itmp[ind[i]];			\
      } else {					\
        for (i=0; i<n_nodes; i++)		\
        for (j=0; j<iu_sz; j++)			\
	  ix[i*iu_sz+j] = itmp[ind[i]*iu_sz+j];	\
      }						\
    }						\
    else {					\
      if (u_sz == 1) {				\
        for (i=0; i<n_nodes; i++)		\
	  x[i] = tmp[ind[i]];			\
      } else {					\
        for (i=0; i<n_nodes; i++)		\
        for (j=0; j<u_sz; j++)			\
	  x[i*u_sz+j] = tmp[ind[i]*u_sz+j];	\
      }						\
    }						\
}
#define REPOSITION2(tmp, x, n_nodes, u_sz, ind2) { \
    int i, j;					\
    bcopy (x, tmp, n_nodes*u_sz);		\
    if (u_sz % sizeof(double) == 0) {		\
      double *dtmp=(double*)tmp, *dx=(double*)x;\
      int du_sz = u_sz/sizeof(double); 		\
      if (du_sz == 1) {				\
        for (i=0; i<n_nodes; i++)		\
	  dx[ind2[i]] = dtmp[i];		\
      } else {					\
        for (i=0; i<n_nodes; i++)		\
        for (j=0; j<du_sz; j++)			\
	  dx[ind2[i]*du_sz+j] = dtmp[i*du_sz+j];\
      }						\
    }						\
    else if (u_sz % sizeof(int) == 0) {		\
      int *itmp = (int*)tmp, *ix = (int*)x; 	\
      int iu_sz = u_sz/sizeof(int); 		\
      if (iu_sz == 1) {				\
        for (i=0; i<n_nodes; i++)		\
	  ix[ind2[i]] = itmp[i];		\
      } else {					\
        for (i=0; i<n_nodes; i++)		\
        for (j=0; j<iu_sz; j++)			\
	  ix[ind2[i]*iu_sz+j] = itmp[i*iu_sz+j];\
      }						\
    }						\
    else {					\
      if (u_sz == 1) {				\
        for (i=0; i<n_nodes; i++)		\
	  x[ind2[i]] = tmp[i];   		\
      } else {					\
        for (i=0; i<n_nodes; i++)		\
        for (j=0; j<u_sz; j++)			\
	  x[ind2[i]*u_sz+j] = tmp[i*u_sz+j];	\
      }						\
    }						\
}

#define N_REPOSITION()  { \
    if (repos != NULL) {                        \
      for (i = 0; i < n; i++) {			\
        ptr = (byte *)(repos[i][0]);		\
        siz = (int) (repos[i][1]);     		\
        if (siz > siz0) {			\
            FREE(tx, byte, n_nodes*siz0)	\
            MALLOC (tx, byte, n_nodes*siz)	\
        }					\
        REPOSITION(tx, ptr, n_nodes, siz, ind)	\
        siz0 = siz;				\
      }						\
      FREE (tx, byte, n_nodes*siz0)		\
    }                                           \
}


#define CACHE_LINE(a)  ((((long)(a))>>5)%512)

/*===================== function declarations ============================*/

/*========================================================================*
 *									  *
 *  Read meshes and coordinates from files 				  *
 *									  *
 *  	ReadInputGraph    - read meshes for EDGE LIST			  *
 *  	ReadInputGraph2   - read meshes for PARTNER LIST			  *
 *      ReadCoord    - if coordinates are given for RCB                   *
 *      ReadEdgeList - if edge list is given directly                     * 
 *									  *
 *========================================================================*/

void ReadInputGraph(char *filename, int *n_nodes, int *n_edges, int el[][2]);
void ReadInputGraph2(char *filename, int *n_nodes, int *n_edges, int *from, int *partners);
void ReadCoord(char *filename, int n_nodes, float xyz[][3]);
void ReadEdgeList(char *filename, int *n_nodes, int *n_edges, int el[][2]);


void LocalityReordering(int, ...);
void LocalityReordering2(int, ...);
void NA_LocalityReordering(int, ...);
void NA_LocalityReordering2(int, ...);
void LR_set_coordinate(double (*xyz)[3]);
void LR_set_choice(int optimization);
void LR_set_adaptive(int n_iter, int adapt_freq);
void LR_set_override_adaptive(int opt_freq);
void LR_access_changed();
void LR_access_change_begin();
void LR_reorder_map(int **);

#ifdef INSPECTOR
/*========================================================================*
 *								   	  *
 * Preprocess to apply LocalWrite (Classify accesses in 3 categories)	  *  
 *                                 Both, Left(or I), Right(or J)          *
 *									  *
 * 	inspector  - for EDGE LIST					  *
 * 	inspector2 - for PARTNER LIST					  *
 *									  *
 *========================================================================*/
void inspector(int (*el)[2], int ne, int nn, int my_id, int nproc,
          int (*both)[2],  int *bcnt,
          int (*left)[2],  int *lcnt,
          int (*right)[2], int *rcnt);
void inspector2(int *ptnr, int *from, int nn, int my_id, int nproc,
          int *bothptnr, int *bfrom,
          int *iptnr,    int *ifrom,
          int *jptnr,    int *jfrom);

#endif

void adapt_graph(int, ...);
void adapt_graph2(int, ...);

#ifdef BLOCK_ADAPTIVE
int *NodeBlocking (double (*xyz)[3], int n_nodes, double side, int *n_xyz);
#endif

#ifdef STATS
double reuse_distance(int n_nodes, int *partners, int *from);
#endif
#endif /* MISC_H */
