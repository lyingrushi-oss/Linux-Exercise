#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

/* 两个线程共享的数据 */
static int data = 0;
static int data_ready = 0;

/* 保护 data 和 data_ready */
static pthread_mutex_t mutex =
    PTHREAD_MUTEX_INITIALIZER;

/* 通知“数据状态发生变化” */
static pthread_cond_t cond =
    PTHREAD_COND_INITIALIZER;

static void* consumer(void* arg)
{
    (void)arg;
    //拿锁
    pthread_mutex_lock(&mutex);
    //必须用while而不用if:虚假唤醒或者多个消费者等待产生竟态
    while(!data_ready)
    {
        printf("[consumer] waiting for data sleeping\n");
        pthread_cond_wait(&cond,&mutex);
    }
    //成功唤醒
    printf("[consumer] recv data=%d\n",data);
    //还锁
    pthread_mutex_unlock(&mutex);
    return NULL;
}

static void* producer(void* arg)
{
    (void)arg;
    sleep(1);
    //拿锁
    pthread_mutex_lock(&mutex);
    data=42;
    data_ready=1;
    printf("[producer] data ready data=%d\n",data);
    //更新状态条件变量，唤醒消费者线程
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    return NULL;

}

int main(void)
{
    //创建生产者和消费者线程
    pthread_t pthread_producer;
    pthread_t pthread_consumer;

    pthread_create(&pthread_consumer,NULL,consumer,NULL);
    pthread_create(&pthread_producer,NULL,producer,NULL);

    //等待线程返回
    pthread_join(pthread_producer,NULL);
    pthread_join(pthread_consumer,NULL);

    //销毁锁和条件变量
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
    printf("😭🔍 main finished\n");
    return 0;
}