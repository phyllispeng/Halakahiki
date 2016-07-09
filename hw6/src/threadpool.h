#ifndef threadpool_h
#define threadpool_h

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <errno.h>
#include <signal.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <time.h>
#include <stdbool.h>
#include "fileManager.h"
#include "wrap.h"

#define _GNU_SOURCE

typedef struct job job_t;

struct job {
    job_t *job_next; /* linked list of jobs */

    void *(*job_func)(void *); /* function to call */
    void *job_arg; /* its argument */
};

typedef struct active active_t;

struct active {
   active_t *active_next; /* linked list of threads */
   pthread_t active_tid; /* active thread id */
};

typedef struct th_pool pool_t;

struct th_pool {


    pool_t *pool_forw; /* circular linked list */
    pool_t *pool_back; /* of all thread pools */


    pthread_mutex_t pool_mutex; /* protects the pool data */

    pthread_cond_t pool_busycv;
    pthread_cond_t pool_workcv;
    pthread_cond_t pool_waitcv;

    
    pool_t *forward; /* circular linked list */
    pool_t *backward; /* of all thread pools */
    
    
    pthread_mutex_t mutex; /* protects the pool data */
    
    pthread_cond_t busy;
    pthread_cond_t work;
    pthread_cond_t wait;
    

    active_t *active; /* list of threads performing work */

    job_t *pool_head; /* head of FIFO job queue */
    job_t *pool_tail; /* tail of FIFO job queue */

    pthread_attr_t pool_attr; /* attributes of the workers */

    int pool_flags; /* see below */


    uint16_t pool_linger; /* seconds before idle workers exit */
    uint16_t pool_minimum; /* min number of worker threads */
    uint16_t pool_maximum; /* max number of worker threads */
    uint16_t pool_nthreads; /* current number of worker threads */
    uint16_t pool_idle; /* number of idle workers */
};



/* pool_flags */
#define POOL_WAIT     0x01 /* waiting in thr_pool_wait() */
#define POOL_DESTROY  0x02 /* pool is being destroyed */


/*
 * Creates a thread pool. More than one pool can be created.
 * @param min Minimum number of threads in the pool.
 * @param max Maximum number of threads in the pool.
 * @param linger Number of seconds that idle threads can linger before
 * exiting, when no tasks come in. The idle threads can only exit if
 * they are extra threads, above the number of minimum threads.
 * @param attr Attributes of all worker threads. This can be NULL.
 * @return Returns a pool_t if the operation was successful. On error,
 * NULL is returned with errno set to the error code.
 */

pool_t* pool_create(uint16_t min, uint16_t max, uint16_t linger, pthread_attr_t* attr);


/*
* Enqueue a work request to the thread pool job queue.
* If there are idle worker threads, awaken one to perform the job.
* Else if the maximum number of workers has not been reached,
* create a new worker thread to perform the job.
* Else just return after adding the job to the queue;
* an existing worker thread will perform the job when
* it finishes the job it is currently performing.
* @param pool A thread pool identifier returned from pool_create().
* @param func The task function to be called.
* @param arg The only argument passed to the task function.
* @return Returns 0 on success, otherwise -1 with errno set to the error
code.
*/
int pool_queue(pool_t* pool, void* (*func)(void *), void* arg);


/*
* Wait for all queued jobs to complete in the thread pool.
* @param pool A thread pool identifier returned from pool_create(). **/
void pool_wait(pool_t *pool);

/*
 * Cancel all queued jobs and destroy the pool. Worker threads that are
 * actively processing tasks are cancelled.
 * @param pool A thread pool identifier returned from pool_create(). */
void pool_destroy(pool_t *pool);

uint16_t* readConfigFile(char* filename);

void copyThreadAttr(pthread_attr_t *copy, pthread_attr_t *original);

bool createThread(pool_t *pool);

void threadCleanup(pool_t *pool);

void *thread(void *arg);

void update(pool_t *pool);

void jobCleanup(pool_t *pool);

#endif
