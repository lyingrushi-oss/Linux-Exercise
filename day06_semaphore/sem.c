#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

//定义数组大小
#define SAMPLE_COUNT 5

//传感器数据类型结构体
typedef struct{
    int seq;
    float temp;
}sensor_data_t;

//静态全局变量
static sensor_data_t buf[SAMPLE_COUNT];
static sem_t sem;//计数信号量

//生产者任务
static void* producer_task(void* arg)
{
    (void)arg;
    for(int i=0;i<SAMPLE_COUNT;i++)
    {
        buf[i].seq=i;
        buf[i].temp=25.0f+i*0.5f;
        printf("[producer] seq=%d temp=%.1f\n",buf[i].seq,buf[i].temp);
        //释放一个信号量
        if(sem_post(&sem)!=0)
        {
            //失败了
            fprintf(stderr,"[producer] post sem failed\n");
            perror("sem_post");
            return NULL;
        }
        printf("[producer] post success!\n");
    }
    printf("[producer] finished\n");
    return NULL;
}

static void* consumer_task(void* arg)
{
    (void)arg;
    for(int i=0;i<SAMPLE_COUNT;i++)
    {
        printf("[consumer] waiting for sem\n");
        if(sem_wait(&sem)!=0)
        {
            //等待失败
            perror("sem_wait");
            return NULL;
        }
        printf("[consumer] get sem\n");
        printf("[consumer] seq=%d temp=%.1f\n",buf[i].seq,buf[i].temp);
        //sleep(1);
    }
    printf("[consumer] finished\n");
    return NULL;
}

int main(void)
{
    pthread_t producer;
    pthread_t consumer;
    int ret;
    //先创建信号量资源
    if(sem_init(&sem,0,0)==-1)
    {
        perror("sem_init");
        return 1;
    }
    ret=pthread_create(&producer,NULL,producer_task,NULL);
    if(ret!=0)
    {
        //创建失败
        //perror("pthread_create");
        fprintf(stderr,"producer create failed:%s\n",strerror(ret));
        //销毁信号量资源
        sem_destroy(&sem);
        return 1;
    }
    //sleep(2);
    ret=pthread_create(&consumer,NULL,consumer_task,NULL);
    if(ret!=0)
    {
        //创建失败
        //perror("pthread_create");
        fprintf(stderr,"consumer create failed:%s\n",strerror(ret));
        //销毁信号量资源
        sem_destroy(&sem);
        return 1;
    }
    pthread_join(producer,NULL);
    pthread_join(consumer,NULL);

    //销毁资源
    sem_destroy(&sem);
    printf("[main] all threads joined\n");
    return 0;
}