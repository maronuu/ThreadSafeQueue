#include "safeQueue.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N_THREAD 1000
#define QUEUE_SIZE 10000
#define REPEAT_COUNT 10

int stopRequest;

void mSleep(int msec) {
    struct timespec ts;
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

double randDouble(double minValue, double maxValue) {
    return minValue + (double)rand() / ((double)RAND_MAX + 1) * (maxValue - minValue);
}

void *enqueue(void *arg) {
    SafeQueue *que = (SafeQueue *)arg;
    double x;
    int i;
    for(i = 0; i < REPEAT_COUNT; i++) {
        x = randDouble(0, 10000);
        if(!SafeQueuePush(que, x, x)) {
            printf("Failed to SafeQueueAdd\n");
        }
        mSleep(rand()%100);
    }
    return NULL;
}

void *dequeue(void *arg) {
    SafeQueue *que = (SafeQueue *)arg;
    double x, y;
    while(!stopRequest) {
        if(SafeQueueWait(que, 100)) {
            if(!SafeQueuePop(que, &x, &y)) {
                printf("Failed to SafeQueueGet\n");
                continue;
            }
            if(x != y) {
                printf("Oops! Invalid que data (%f != %f)\n", x, y);
            }
        }
    }
    return NULL;
}

int main() {
    SafeQueue *sque;
    pthread_t enqueueThreads[N_THREAD];
    pthread_t dequeueThread;

    printf("Start Testing...\n");
    
    if ((sque = SafeQueueCreate(QUEUE_SIZE)) == NULL) {
        printf("Failed to create queue\n");
        exit(1);
    }
    if (pthread_create(&dequeueThread, NULL, dequeue, (void *)sque) != 0) {
        printf("Failed to create dequeueThread\n");
        exit(1);
    }
    for (int i = 0; i < N_THREAD; ++i) {
        if (pthread_create(&enqueueThreads[i], NULL, enqueue, (void *)sque) != 0) {
            printf("Failed to create enqueuethreads[%d]\n", i);
            exit(1);
        }
    }

    for (int i = 0; i < N_THREAD; ++i) {
        if (pthread_join(enqueueThreads[i], NULL) != 0) {
            printf("Faield to joint enqueueThreads[%d]\n", i);
            exit(1);
        }
    }
    stopRequest = 1;
    if (pthread_join(dequeueThread, NULL) != 0) {
        printf("Failed to join dequeueThread.\n");
        exit(1);
    }
    SafeQueueDestroy(sque);

    printf("Success!\n");
    return 0;
}

