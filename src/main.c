#include "safeQueue.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>


#define WIDTH 78   /* width of the display */
#define HEIGHT 23  /* height of the display */
#define MAX_FLY 1  /* number of flies */

#define QUEUE_SIZE 10 /* size of queue */

static int stopRequest;  /* thread flag */
static int drawRequest;  /* draw flag */
static pthread_mutex_t drawMutex; /* mutex for drawing */
static pthread_cond_t  drawCond;  /* condition for drawing */

static void requestDraw(void);

/*
 * sleeps for x msec
 */
void mSleep(int msec) {
    struct timespec ts;
    ts.tv_sec = msec/1000;
    ts.tv_nsec = (msec%1000)*1000000;
    nanosleep(&ts, NULL);
}

/*
 * condwait for msec
 */
int pthread_cond_timedwait_msec(pthread_cond_t *cond, pthread_mutex_t *mutex, long msec) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += msec/1000;
    ts.tv_nsec += (msec%1000)*1000000;
    if(ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }
    return pthread_cond_timedwait(cond, mutex, &ts);
}

/*
 * reflesh the display
 */
void clearScreen() {
    fputs("\033[2J", stdout);
}

/*
 * move the cursor
 */
void moveCursor(int x, int y) {
    printf("\033[%d;%dH", y, x);
}

/*
 * save the position of cursor
 */
void saveCursor() {
    printf("\0337");
}

/*
 * restore the position of cursor
 */
void restoreCursor() {
    printf("\0338");
}

/*
 * fly
 */
typedef struct {
    char mark;    
    double x, y;  
    double angle; 
    double speed; 
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    SafeQueue *destQue; 
} Fly;

Fly flyList[MAX_FLY];

/**
 * @brief initialize fly
 * 
 * @param fly 
 * @param mark_ 
 */
void FlyInitCenter(Fly *fly, char mark_) {
    fly->mark = mark_;
    pthread_mutex_init(&fly->mutex, NULL);
    pthread_cond_init(&fly->cond, NULL);
    fly->x = (double)WIDTH/2.0;
    fly->y = (double)HEIGHT/2.0;
    fly->angle = 0;
    fly->speed = 2;
    fly->destQue = SafeQueueCreate(QUEUE_SIZE);
}

/**
 * @brief destroy a fly object
 * 
 * @param fly 
 */
void FlyDestroy(Fly *fly) {
    pthread_mutex_destroy(&fly->mutex);
    pthread_cond_destroy(&fly->cond);
    SafeQueueDestroy(fly->destQue);
}

/**
 * @brief move fly
 * 
 * @param fly 
 */
void FlyMove(Fly *fly) {
    pthread_mutex_lock(&fly->mutex);
    fly->x += cos(fly->angle);
    fly->y += sin(fly->angle);
    pthread_mutex_unlock(&fly->mutex);
    requestDraw();
}

int FlyIsAt(Fly *fly, int x, int y) {
    pthread_mutex_lock(&fly->mutex);
    int res = ((int)fly->x == x) && ((int)fly->y == y);
    pthread_mutex_unlock(&fly->mutex);
    return res;
}

void FlySetDirection(Fly *fly, double destX, double destY) {
    pthread_mutex_lock(&fly->mutex);
    double dx = destX - fly->x;
    double dy = destY - fly->y;
    fly->angle = atan2(dy, dx);
    fly->speed = sqrt(dx*dx+dy*dy)/2.0;
    if (fly->speed < 2.0) {
        fly->speed = 2.0;
    }
    pthread_mutex_unlock(&fly->mutex);
}

double FlyDistance(Fly *fly, double x, double y) {
    pthread_mutex_lock(&fly->mutex);
    double dx = x - fly->x;
    double dy = y - fly->y;
    double res = sqrt(dx*dx + dy*dy);
    pthread_mutex_unlock(&fly->mutex);
    return res;
}

int FlySetDestination(Fly *fly, double x, double y) {
    return SafeQueuePush(fly->destQue, x, y);
}

int FlyWaitForSetDestination(Fly *fly, double *destX, double *destY, long msec) {
    if(!SafeQueueWait(fly->destQue, msec))
        return 0;
    if(!SafeQueuePop(fly->destQue, destX, destY))
        return 0;
    return 1;
}

static void *doMove(void *arg) {
    Fly *fly = (Fly *)arg;
    double destX, destY;
    while(!stopRequest) {
        if (!FlyWaitForSetDestination(fly, &destX, &destY, 100))
            continue;
        FlySetDirection(fly, destX, destY);
        while((FlyDistance(fly, destX, destY) >= 1.0) && !stopRequest) {
            FlyMove(fly);
            mSleep((int)(1000.0 / fly->speed));
        }
    }
    return NULL;
}

static void requestDraw() {
    pthread_mutex_lock(&drawMutex);
    drawRequest = 1;
    pthread_cond_signal(&drawCond);
    pthread_mutex_unlock(&drawMutex);
}

static void drawScreen() {
    int x,y;
    char ch;
    int i;

    saveCursor();
    moveCursor(0, 0);
    for(y = 0; y < HEIGHT; y++) {
        for(x = 0; x < WIDTH; x++) {
            ch = 0;
            for(i = 0; i < MAX_FLY; i++) {
                if(FlyIsAt(&flyList[i], x, y)) {
                    ch = flyList[i].mark;
                    break;
                }
            }
            if(ch != 0) {
                putchar(ch);
            } else if((y == 0) || (y == HEIGHT-1)) {
                putchar('-');
            } else if((x == 0) || (x == WIDTH-1)) {
                putchar('|');
            } else {
                putchar(' ');
            }
        }
        putchar('\n');
    }
    restoreCursor();
    fflush(stdout);
}

void *doDraw(void *arg) {
    int err;
    pthread_mutex_lock(&drawMutex);
    while(!stopRequest) {
        err = pthread_cond_timedwait_msec(&drawCond, &drawMutex, 100);
        if((err != 0) && (err != ETIMEDOUT)) {
            printf("Fatal error\n");
            exit(1);
        }
        while (drawRequest && !stopRequest) {
            drawRequest = 0;
            pthread_mutex_unlock(&drawMutex);
            drawScreen();
            pthread_mutex_lock(&drawMutex);
        }
    }
    pthread_mutex_unlock(&drawMutex);
    return NULL;
}

int main() {
    pthread_t drawThread;
    pthread_t moveThread;
    pthread_mutex_init(&drawMutex, NULL);
    pthread_cond_init(&drawCond, NULL);
    clearScreen();
    FlyInitCenter(&flyList[0], '@');

    pthread_create(&moveThread, NULL, doMove, (void *)&flyList[0]);
    pthread_create(&drawThread, NULL, doDraw, NULL);
    requestDraw();

    char buf[40], *cp;
    double destX, destY;
    while(1) {
        printf("Destination? ");
        fflush(stdout);
        fgets(buf, sizeof(buf), stdin);
        if(strncmp(buf, "stop", 4) == 0)
            break;
        destX = strtod(buf, &cp);
        destY = strtod(cp, &cp);
        if (!FlySetDestination(&flyList[0], destX, destY)) {
            printf("The flyu is busy now\n");
        }
    }
    stopRequest = 1;

    pthread_join(drawThread, NULL);
    pthread_join(moveThread, NULL);
    FlyDestroy(&flyList[0]);
    pthread_mutex_destroy(&drawMutex);
    pthread_cond_destroy(&drawCond);

    return 0;
}