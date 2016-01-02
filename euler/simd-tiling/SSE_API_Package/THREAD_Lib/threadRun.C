#include <iostream>
#include <malloc.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <sys/time.h>
#include <pthread.h>
#include "atomic.h"
#include "header.h"
#include "../../ThreadClass/ThreadClass.h"
#include "sync.h"
#include <immintrin.h>
#include "sse_api.h"
#include "transform.h"

#define DIM 3

using namespace std;

const int vec_width = 16;

//float rtclock();
double total_E_copy;
double total_E_computing;
double rtclock()
{
	struct timezone Tzp;
	struct timeval Tp;
	int stat;
	stat = gettimeofday (&Tp, &Tzp);
	if (stat != 0) printf("Error return from gettimeofday: %d",stat);
	return(Tp.tv_sec + Tp.tv_usec*1.0e-6);
};

class Kmeans_Kernel : public ThreadClass
{
private:
    int tid;
    int thread_num;
    int k;
    int n;
    float *data;
    float *update;
    float *cluster;
    Sync *sync;
    int start;
    int block;
    int end;
    
public:
    Kmeans_Kernel(int _tid, int _thread_num, int _k, int _n, float *_data, float *_update, float *_cluster, Sync *_sync)
    :tid(_tid), thread_num(_thread_num), k(_k), n(_n), data(_data), update(_update),cluster(_cluster), sync(_sync)
    {
        block = n/thread_num;
        start = tid*block;
        end =(tid==thread_num-1?n:start+block);
    }
    
    void threadEntry()
    {
        sync->setSyncTime();
        kernel();
    }
    
    void kernel()
    {
        printf("tid%d: start = %d, end = %d\n",tid, start, end);
        for(int i = start; i < end; i += vec_width)
        {
            sfloat *x = (sfloat*)&data[i];
            sfloat *y = (sfloat*)&data[i+n];
            sfloat *z = (sfloat*)&data[i+2*n];
            sint min_index = 0;
            sfloat sse_min = (float)65535*65;
           
            sfloat dis;
            sfloat cluster_cord;
            sfloat dis_cord;

            for(int j = 0; j < k; ++j){
                dis = 0.0;
                cluster_cord = cluster[j*DIM];
                dis_cord = *x - cluster_cord;
                dis = dis + (dis_cord * dis_cord);
                
                cluster_cord = cluster[j*DIM+1];
                dis_cord = *y - cluster_cord;
                dis = dis + (dis_cord * dis_cord);
                
                cluster_cord = cluster[j*DIM+2];
                dis_cord = *z - cluster_cord;
                dis = dis + (dis_cord * dis_cord);
                
                dis = sqrt(dis);
                
                //set mask
                __MASK m = dis < sse_min;
                smask.set_mask(m, sse_min);
                sse_min = dis;
                smask.set_mask(m, min_index);
                min_index = sint(j);
                smask.clear();
            }
            
            reduction(update, 5, 0, min_index, *x);
            reduction(update, 5, 1, min_index, *y);
            reduction(update, 5, 2, min_index, *z);
            reduction(update, 5, 3, min_index, 1.0);
            reduction(update, 5, 4, min_index, sse_min);
        }
    }
};

void kmeans_func(int k,int n,float* data,float* update,float* cluster)
{   
    double t0, t1;
    double transferTime;
    int thread_num = kmeans_THREADS;

    /************************Initialization**********************/
    init();
    float *d_update = (float *)_mm_malloc(5*k*thread_num*sizeof(float), 64);
    float *d_data = (float *)_mm_malloc(DIM*n*sizeof(float), 64);
    memset(update, 0, sizeof(float)*5*k);
    memset(d_update, 0, sizeof(float)*5*k*thread_num);

    //Transform Data layout
    t0 = rtclock();
    transformTo(data, d_data, DIM, n);
    t1 = rtclock();
    transferTime = t1 - t0;
    
    Sync sync(thread_num);
    /*********************************************************/
    
    Kmeans_Kernel **kmeansThreads = new Kmeans_Kernel*[thread_num];
    for(int i = 0; i < thread_num; ++i){
        kmeansThreads[i] = new Kmeans_Kernel(i, thread_num, k, n, d_data, d_update+5*k*i, cluster, &sync);
    }

    for(int i = 0; i < thread_num; i++)
    {
        kmeansThreads[i]->run();
    }

    for(int i = 0; i < thread_num; i++)
    {
        kmeansThreads[i]->join();
    }
    sync.setEndTime();

    cout<<"kernel time = "<<sync.getTime()<<endl;
    cout<<"Data Transfermation time = "<<transferTime<<endl;
    
    //combination
    for(int i = 0; i < thread_num; i++)
    {
        for(int j = 0; j < k; j++)
        {
            update[j*5] += d_update[i*k*5+j*5];
            update[j*5+1] += d_update[i*k*5+j*5+1];
            update[j*5+2] += d_update[i*k*5+j*5+2];
            update[j*5+3] += d_update[i*k*5+j*5+3];
            update[j*5+4] += d_update[i*k*5+j*5+4];
        }
    }

    _mm_free(d_update);
    _mm_free(d_data);
}
void Free_func()
{

}
