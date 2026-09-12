#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

//样本量
#define SAMPLE_COUNT 5

//传感器数据
typedef struct{
    int seq;    //序列号
    float temp; //温度
}sensor_data_t;

/*全局变量*/
static sensor_data_t buf;
static int data_ready=0;
static int pro_finish=0;

static pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

static void* producer_task(void* arg)
{
    (void)arg;
    for(int i=0;i<SAMPLE_COUNT;i++)
    {
        //拿锁
        pthread_mutex_lock(&mutex);
        while(data_ready)
        {
            pthread_cond_wait(&cond,&mutex);//释放等待条件变量成立再拿锁
        }
        //修改缓冲区
        buf.seq=i+1;
        buf.temp=25.0f+0.5f*i;
        data_ready=1;
        printf("[producer]:data ready,seq=%d,temp=%.1f\n",buf.seq,buf.temp);

        //通知消费者共享状态发生变化
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);

        sleep(1);
    }
    pthread_mutex_lock(&mutex);
    /*确保最后一条被消费者取走*/
    while(data_ready)
    {
        pthread_cond_wait(&cond,&mutex);

    }
    pro_finish=1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    printf("[producer] finished\n");
    return NULL;

}

static void* consumer_task(void* arg)
{
    (void)arg;
    while(1)
    {
        pthread_mutex_lock(&mutex);
        while(!data_ready&&!pro_finish)
        {
            //数据没有就绪且生产者任务未结束
            printf("[consumer] waiting...\n");
            pthread_cond_wait(&cond,&mutex);
        }

        /*没有数据且生产者已结束*/
        if(!data_ready&&pro_finish)
        {
            //释放锁
            printf("[consumer] producer finished, exiting\n");
            pthread_mutex_unlock(&mutex);
            break;
        }

        /*生产者完成*/

        /*有数据产生*/
        if(data_ready)
        {
            sensor_data_t local = buf;
            //修改共享变量
            data_ready=0;
            //还锁
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
            printf("[consumer] seq=%d,temp=%.1f\n",local.seq,local.temp);
        }
       


    }
    return NULL;
}

int main(void)
{

    //创建生产者和消费者线程
    pthread_t pthread_producer;
    pthread_t pthread_consumer;

    pthread_create(&pthread_consumer,NULL,consumer_task,NULL);
    pthread_create(&pthread_producer,NULL,producer_task,NULL);

    //等待线程返回
    pthread_join(pthread_producer,NULL);
    pthread_join(pthread_consumer,NULL);
    printf("[main] all threads joined\n");

    //销毁锁和条件变量
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);

    return 0;
}

