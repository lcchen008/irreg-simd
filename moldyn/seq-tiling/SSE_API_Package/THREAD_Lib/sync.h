#ifndef SYNC_H
#define SYNC_H

#include <sys/time.h>
#include <pthread.h>
#include <cstdlib>
#include <cstdio>

class Sync
{
private:
    pthread_mutex_t mutex_var;
    pthread_cond_t cond_var;
    int count;
    int thread_num;
    double t0, t1;
    
    double rtclock()
    {
	    struct timezone Tzp;
	    struct timeval Tp;
	    int stat;
	    stat = gettimeofday (&Tp, &Tzp);
	    if (stat != 0) printf("Error return from gettimeofday: %d",stat);
	    return(Tp.tv_sec + Tp.tv_usec*1.0e-6);
    };

public:
    Sync(int _count)
        :thread_num(_count), count(_count), mutex_var(PTHREAD_MUTEX_INITIALIZER),
        cond_var(PTHREAD_COND_INITIALIZER), t0(0), t1(0)
    {}

    void lock()
    {
        pthread_mutex_lock(&mutex_var);
    }

    void unlock()
    {
        pthread_mutex_unlock(&mutex_var);
    }
    
    void setSyncTime()
    {
        pthread_mutex_lock(&mutex_var);
        if(--count == 0)
        {
            count = thread_num;
            pthread_cond_broadcast(&cond_var);
            t0 = rtclock();
        }
        else
        {
            pthread_cond_wait(&cond_var, &mutex_var);
        }
        pthread_mutex_unlock(&mutex_var);
    }
    
    void threadSync()
    {
        pthread_mutex_lock(&mutex_var);
        if(--count == 0)
        {
            count = thread_num;
            pthread_cond_broadcast(&cond_var);
        }
        else
        {
            pthread_cond_wait(&cond_var, &mutex_var);
        }
        pthread_mutex_unlock(&mutex_var);
    }


    double setStartTime()
    {
        return t0=rtclock();
    }

    double setEndTime()
    {
        return t1=rtclock();
    }

    double getTime()
    {
        return (t1-t0);
    }
};

#endif
