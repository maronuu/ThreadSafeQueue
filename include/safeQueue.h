#ifndef _SAFE_QUEUE_H_
#define _SAFE_QUEUE_H_

#include <stdlib.h>

/**
 * @brief Queue type
 * 
 */
typedef struct SafeQueue_ SafeQueue;

/**
 * @brief create safe queue object
 * 
 * @param size : max number of elements in queue
 * @return SafeQueue* : a pointer to created queue. if fails, return NULL.
 */
extern SafeQueue *SafeQueueCreate(size_t size);

/**
 * @brief destroy safe queue object
 * 
 * @param sque 
 */
extern void SafeQueueDestroy(SafeQueue *sque);

/**
 * @brief get size of queue
 * 
 * @param sque 
 * @return size_t 
 */
extern size_t SafeQueueGetSize(SafeQueue *sque);

/**
 * @brief get current number of elements in a queue
 * 
 * @param sque 
 * @return size_t 
 */
extern size_t SafeQueueGetCount(SafeQueue *sque);

/**
 * @brief get current free space in a queue
 * 
 * @param sque 
 * @return size_t 
 */
extern size_t SafeQueueGetFreeCount(SafeQueue *sque);

/**
 * @brief pushes an element to a queue
 * 
 * @param que 
 * @param x 
 * @param y 
 * @return int : status (SUCCESS = 1, FAILED = 0)
 */
extern int SafeQueuePush(SafeQueue *sque, double x, double y);

/**
 * @brief pops an element in a queue and stores them into arguments.
 * 
 * @param que 
 * @param x 
 * @param y 
 * @return int : status (SUCCESS = 1, FAILED = 0)
 */
extern int SafeQueuePop(SafeQueue *sque, double *x, double *y);

/**
 * @brief waits until msec at most
 *        does not wait if  an element exists
 * 
 * @param sque 
 * @param msec 
 * @return int : statis (1 = added element, 0 = TIMEDOUT)
 */
extern int SafeQueueWait(SafeQueue *sque, long msec);

#endif