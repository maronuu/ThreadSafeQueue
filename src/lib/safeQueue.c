#include "safeQueue.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>




/**
 * @brief type of an item in a queue
 * 
 */
typedef struct {
    double x;
    double y;
} Point;

/**
 * @brief queue struct
 * 
 */
struct SafeQueue_ {
    Point *data; /*data array*/
    size_t size;  /*size of data array*/
    size_t wp;    /*write index (push)*/
    size_t rp;    /*read index (read)*/
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

/*
 * sleeps for x msec
 */
static void mSleep(int msec) {
    struct timespec ts;
    ts.tv_sec = msec/1000;
    ts.tv_nsec = (msec%1000)*1000000;
    nanosleep(&ts, NULL);
}

/**
 * @brief create safe queue object
 * 
 * @param size : max number of elements in queue
 * @return SafeQueue* : a pointer to created queue. if fails, return NULL.
 */
SafeQueue *SafeQueueCreate(size_t size) {
    if (size == 0)
        return NULL;
    SafeQueue *sque = (SafeQueue *)malloc(sizeof(SafeQueue));
    if (sque == NULL)
        return NULL;
    
    sque->size = size + 1;
    sque->data = (Point *)malloc(sque->size * sizeof(Point));
    if (sque->data == NULL) {
        free(sque);
        return NULL;
    }
    sque->wp = sque->rp = 0;
    pthread_mutex_init(&sque->mutex, NULL);
    pthread_cond_init(&sque->cond, NULL);

    return sque;
}

/**
 * @brief destroy safe queue object
 * 
 * @param sque 
 */
void SafeQueueDestroy(SafeQueue *sque) {
    if (sque == NULL)
        return;
    pthread_cond_destroy(&sque->cond);
    pthread_mutex_destroy(&sque->mutex);
    free(sque->data);
    free(sque);
}

/**
 * @brief get size of queue
 * 
 * @param sque 
 * @return size_t 
 */
size_t SafeQueueGetSize(SafeQueue *sque) {
    if (sque == NULL)
        return 0;
    return sque->size - 1;
}

/**
 * @brief get current number of elements in a queue
 * 
 * @param sque 
 * @return size_t 
 */
size_t SafeQueueGetCount(SafeQueue *sque) {
    if (sque == NULL)
        return 0;
    int res;
    pthread_mutex_lock(&sque->mutex);
    if (sque->wp < sque->rp)
        res = sque->size + sque->wp - sque->rp;
    else
        res = sque->wp - sque->rp;
    pthread_mutex_unlock(&sque->mutex);
    return res;
}

/**
 * @brief get current free space in a queue
 * 
 * @param sque 
 * @return size_t 
 */
size_t SafeQueueGetFreeCount(SafeQueue *sque) {
    if (sque == NULL)
        return 0;
    return sque->size - 1 - SafeQueueGetCount(sque);
}

/**
 * @brief pushes an element to a queue
 * 
 * @param que 
 * @param x 
 * @param y 
 * @return int : status (SUCCESS = 1, FAILED = 0)
 */
int SafeQueuePush(SafeQueue *sque, double x, double y) {
    if (sque == NULL)
        return 0;
    pthread_mutex_lock(&sque->mutex);
    size_t next_wp = sque->wp + 1;
    if (next_wp >= sque->size) {
        next_wp -= sque->size;
    }
    if (next_wp == sque->rp) {
        pthread_mutex_unlock(&sque->mutex);
        return 0;
    }

    sque->data[sque->wp].x = x;
    sque->data[sque->wp].y = y;

    sque->wp = next_wp;
    pthread_mutex_unlock(&sque->mutex);

    return 1;
}

/**
 * @brief pops an element in a queue and stores them into arguments.
 * 
 * @param que 
 * @param x 
 * @param y 
 * @return int : status (SUCCESS = 1, FAILED = 0)
 */
int SafeQueuePop(SafeQueue *sque, double *x, double *y) {
    if (sque == NULL)
        return 0;
    pthread_mutex_lock(&sque->mutex);
    if (sque->rp == sque->wp) {
        pthread_mutex_unlock(&sque->mutex);
        return 0;
    }
    if (x != NULL)
        *x = sque->data[sque->rp].x;
    if (y != NULL)
        *y = sque->data[sque->rp].y;

    if (++(sque->rp) >= sque->size)
        sque->rp -= sque->size;
    pthread_mutex_unlock(&sque->mutex);

    return 1;
}

/**
 * @brief waits until msec at most
 *        does not wait if  an element exists
 * 
 * @param sque 
 * @param msec 
 * @return int : statis (1 = added element, 0 = TIMEDOUT)
 */
int SafeQueueWait(SafeQueue *sque, long msec) {
    if (sque == NULL)
        return 0;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += msec / 1000;
    ts.tv_nsec += (msec % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }
    pthread_mutex_lock(&sque->mutex);
    while(sque->wp == sque->rp) {
        int err = pthread_cond_timedwait(&sque->cond, &sque->mutex, &ts);
        if (err == ETIMEDOUT) {
            break;
        } else if (err != 0) {
            fprintf(stderr, "Fatal error\n");
            exit(1);
        }
    }
    int res = (sque->wp != sque->rp);
    pthread_mutex_unlock(&sque->mutex);
    return res;
}