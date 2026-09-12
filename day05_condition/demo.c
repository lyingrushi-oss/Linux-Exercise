#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define SAMPLE_COUNT 5

typedef struct {
    int sequence;
    float temperature;
} sensor_data_t;

static sensor_data_t buffer;
static int data_ready = 0;
static int producer_finished = 0;

static pthread_mutex_t buffer_mutex =
    PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t buffer_condition =
    PTHREAD_COND_INITIALIZER;

static void *producer(void *arg)
{
    (void)arg;

    for (int i = 0; i < SAMPLE_COUNT; i++) {
        pthread_mutex_lock(&buffer_mutex);

        /*
         * 单槽缓冲区仍有旧数据时，等待消费者取走。
         */
        while (data_ready) {
            pthread_cond_wait(
                &buffer_condition,
                &buffer_mutex
            );
        }

        buffer.sequence = i + 1;
        buffer.temperature = 25.0f + i * 0.5f;
        data_ready = 1;

        printf("[producer] produced: seq=%d, temp=%.1f\n",
               buffer.sequence,
               buffer.temperature);

        /*
         * 通知消费者：共享状态发生了变化。
         */
        pthread_cond_signal(&buffer_condition);
        pthread_mutex_unlock(&buffer_mutex);

        sleep(1);
    }

    pthread_mutex_lock(&buffer_mutex);

    /*
     * 确保最后一条数据已经被消费者取走。
     */
    while (data_ready) {
        pthread_cond_wait(
            &buffer_condition,
            &buffer_mutex
        );
    }

    producer_finished = 1;
    pthread_cond_signal(&buffer_condition);
    pthread_mutex_unlock(&buffer_mutex);

    printf("[producer] finished\n");
    return NULL;
}

static void *consumer(void *arg)
{
    (void)arg;

    while (1) {
        pthread_mutex_lock(&buffer_mutex);

        /*
         * 没有数据且生产者未结束时，释放锁并休眠。
         */
        while (!data_ready && !producer_finished) {
            printf("[consumer] waiting...\n");

            pthread_cond_wait(
                &buffer_condition,
                &buffer_mutex
            );
        }

        /*
         * 没有剩余数据，并且生产者已经结束。
         */
        if (!data_ready && producer_finished) {
            pthread_mutex_unlock(&buffer_mutex);
            break;
        }

        sensor_data_t local_data = buffer;
        data_ready = 0;

        /*
         * 通知生产者：单槽缓冲区已经空了。
         */
        pthread_cond_signal(&buffer_condition);
        pthread_mutex_unlock(&buffer_mutex);

        printf("[consumer] consumed: seq=%d, temp=%.1f\n",
               local_data.sequence,
               local_data.temperature);
    }

    printf("[consumer] finished\n");
    return NULL;
}

int main(void)
{
    pthread_t producer_thread;
    pthread_t consumer_thread;

    int ret = pthread_create(
        &consumer_thread,
        NULL,
        consumer,
        NULL
    );

    if (ret != 0) {
        fprintf(stderr,
                "create consumer: %s\n",
                strerror(ret));
        return 1;
    }

    ret = pthread_create(
        &producer_thread,
        NULL,
        producer,
        NULL
    );

    if (ret != 0) {
        fprintf(stderr,
                "create producer: %s\n",
                strerror(ret));
        return 1;
    }

    ret = pthread_join(producer_thread, NULL);

    if (ret != 0) {
        fprintf(stderr,
                "join producer: %s\n",
                strerror(ret));
        return 1;
    }

    ret = pthread_join(consumer_thread, NULL);

    if (ret != 0) {
        fprintf(stderr,
                "join consumer: %s\n",
                strerror(ret));
        return 1;
    }

    pthread_cond_destroy(&buffer_condition);
    pthread_mutex_destroy(&buffer_mutex);

    return 0;
}