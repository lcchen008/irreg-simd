#ifndef _ThreadClass_h
#define _ThreadClass_h

#include <pthread.h>
#include "sync.h"

class ThreadClass
{
private:
    pthread_t tid;
    static void *threadEntryFunc(void *context)
    {
        ((ThreadClass *)context)->threadEntry();
        return NULL;
    }
    
public:
    ThreadClass(){}
    virtual ~ThreadClass(){}
    
    bool run()
    {
        return pthread_create(&tid, NULL, threadEntryFunc, this) == 0;
    }
    
    bool join()
    {
        return pthread_join(tid, NULL) == 0;
    }
    
protected:
    virtual void threadEntry() = 0;
};

struct ThreadInfo
{
    int tid;
    int thread_num;
    Sync *sync;
    int start;
    int end;
    int block;
    
};

#endif
