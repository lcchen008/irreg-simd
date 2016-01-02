//
//  api_design.h
//  SSE Template
//
//  Created by MARS on 12/13/13.
//
//

enum APPType {GeneralRd, Stencil, IrregularRed};

//MIMD APIs
struct Configuration
{
	vector<int> size;
    vector<int> offset;
    vector<int> stride;
};

//specific parameters for different applications
//typedef tuple<parameter list> Parameters;
//For example, in Kmeans
//typedef tuple<float*, float*, float*, int, int> Parameters;

template<typename ParaType>
class Task
{
public:
	Configuration config;
	ParaType param;
    APPType appType;
    
	Task(const Configuration &_config, const ParaType &_param, APPType type)
	:config(_config), param(_param), appType(type)
    {}
	
	/* The computation logic for a single input data */
    virtual void kernel(vector<int> &index) = 0;
};

/*
//The kernel for kmeans
void Task::kernel(vector<int> &index){
    float min = FLT_MAX;
    int min_index = 0;
    //Iterate k clusters to compute the distance to each cluster
    for(int j = 0; j < get<4>(param); ++j){
        float dis = 0.0;
        for(int m = 0; m < 3; ++m){
            dis += (get<0>(param)[index[0]*3+m]-get<1>(param)[j*3+m])*
            (get<0>(param)[index[0]*3+m]-get<1>(param)[j*3+m]);
        }
        dis = sqrt(dis);
        if(dis < min){
            min = dis;
            min_index = j;
        }else{
            min = min;
            min_index = min_index;
        }
    }
    
    get<3>(param)[5*min_index]   += get<0>(param)[index[0]*3];
    get<3>(param)[5*min_index+1] += get<3>(param)[index[0]*3+1];
    get<3>(param)[5*min_index+2] += get<3>(param)[index[0]*3+2];
    get<3>(param)[5*min_index+3] += 1.0;
    get<3>(param)[5*min_index+4] += min;
}
*/

struct SysParam
{
    vector<thread*> thread_pool;
    int thread_num;
    SysParameter(const vector<thread*> &_thread_pool)
    :thread_pool(_thread_pool), thread_num(_thread_pool.size())
    {}
}

class Partition_func
{
public:
    vector<int> start;
    vector<int> size;

	void partition()(const Configuration &config, const SysParam &sysParam){
		size = vector<int>(sysParam.thread_num, config.size[0]/sysParam.thread_num);
        start = vector<int>(sysParam.thread_num, 0);
        start[0] = 0;
        for(int i = 1; i < sysParam.thread_num; ++i){
            start[i] = start[i-1] + size[i-1];
        }
	}
};

template<typename ParaType>
class Framework
{
private:
	Task<ParaType> *task;
    SysParam sysParam;
	Partition_func part_func;
    
    
    void registerTask(const Task *_task){
        task = _task;
	}
    
    void runtimeOptimization(){
        //data reorganization
    }
    
    void partition(){
        part_func.partition(task->config, sysParam);
    }
    
    void add_one(vector<int> &index, vector<int> &dim, int k){
        if(k < 0){
            ++index[0];
            return;
        }
        
        if(index[k] < dim[k]-1){
            ++index[k];
        }
        else{
            index[k] == 0;
            add_one(index, dim, k-1);
        }
    }
    
    void kernel(int start, int size)
    {
        vector<int> dim = task->config.size;
        vector<int> index(dim.size(), 0);
        
        index[0] = start;
        
        while(index[0] < start+size){
            task->kernel(index);
            add_one(index, dim, dim.size()-1);
        }
    }
    
    void scheduling(){
		for(int i = 0; i < sysParam.thread_num; ++i){
            sysParam.thread_pool[i] = new thread(kernel, part_func.start[i], part_func.size[i]);
        }
        
        for(int i = 0; i < sysParam.thread_num; ++i){
            sysParam.thread_pool[i]->join();
        }
	}
    
public:
    Framework(const Task<ParaType> *_task)
    :task(_task)
    {}
    
    void run(const Task<ParaType> *_task){
		registerTask(_task);
        runtimeOptimization();
		partition();
		scheduling();
	}
    
    void run(){
        runtimeOptimization();
		partition();
		scheduling();
    }
};
