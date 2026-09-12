#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>

#define THREAD_COUNT 2
#define INTERATIONS 100000

/*是否使用互斥锁*/
#ifndef USE_MUTEX
#define USE_MUTEX 1
#endif

/*计数变量*/
static long long counter=0;

/*定义并初始化互斥锁*/
static pthread_mutex_t counter_mutex=PTHREAD_MUTEX_INITIALIZER;



/*线程函数*/
static void* worker(void* param)
{
    int thread_id=*(int*)param;
    printf("[thread %d] started\n",thread_id);
    for(int i=0;i<INTERATIONS;i++)
    {
        #if USE_MUTEX
        pthread_mutex_lock(&counter_mutex);
        #endif

        long long temp=counter;//读
        if(i%1000==0)
        {
            sched_yield();
        }
        temp++;//改
        counter=temp;//写

        #if USE_MUTEX
        pthread_mutex_unlock(&counter_mutex);
        #endif

    }
    printf("[thread %d] finished\n",thread_id);
    return NULL;

}

int main(void)
{
    /*线程数组*/
    pthread_t threads[THREAD_COUNT];
    int thread_id[THREAD_COUNT]={1,2};

    /*创建线程*/
    for(int i=0;i<THREAD_COUNT;i++)
    {
        int ret = pthread_create(&threads[i],NULL,worker,&thread_id[i]);
        if(ret!=0)
        {
            //创建线程失败，打印错误原因
            fprintf(stderr,"[thread %d] create:%s\n",thread_id[i],strerror(ret));
            return 1;

        }
    }

    /*等待线程返回*/
    for(int i=0;i<THREAD_COUNT;i++)
    {
        int ret = pthread_join(threads[i],NULL);
        if(ret!=0)
        {
            fprintf(stderr,"pthread join:%s\n",strerror(ret));
            return 1;
        }
    }

    /*等待成功*/
    printf("expexted counter:%d\n",INTERATIONS*THREAD_COUNT);
    printf("actual counter:%lld\n",counter);
    //释放互斥锁资源
    pthread_mutex_destroy(&counter_mutex);
    return 0;
}
