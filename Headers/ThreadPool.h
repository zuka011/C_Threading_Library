#ifndef _THREAD_POOL_H_2021
#define _THREAD_POOL_H_2021

#include <stdio.h>
#include <pthread.h>
#include <assert.h>

#include "../Headers/Queue.h"

/**
 * This is the interface for a thread pool structure.
 * 
 * A thread pool of desired size can be created in advance and then run
 * on various tasks without the extra overhead of additional thread creation/destruction.
 */

typedef void (*Process)(void *args);

typedef struct {

	int poolSize;
	pthread_t *threadID;
	int tasksLeft;

	pthread_mutex_t taskLock;
	pthread_cond_t taskCond;

	Queue taskQueue;
} ThreadPool;

/**
 * 		Method: TheadPoolInit();
 * //----------------------------------------------------------//
 * 	Initializes a thread pool [pool], with [poolSize] threads, which can
 * later be run to execute a specific procedure. 
 */
void ThreadPoolInit(ThreadPool *pool, int poolSize);

/**
 * 		Method: ThreadPoolSchedule();
 * //----------------------------------------------------------//
 * 	Schedules the passed function [fn] to be executed by a thread as soon
 * as there's a worker thread available. [fn] must accept a void*. [args] is
 * passed to the thread while executing the given function [fn]. This function
 * takes no responsibility over memory management.
 */
void ThreadPoolSchedule(ThreadPool *pool, Process fn, void *args);

/**
 * 		Method: ThreadPoolTasks();
 * //----------------------------------------------------------//
 * Returns the number of unfinished tasks.	
 */
int ThreadPoolTasks(ThreadPool *pool);

/**
 * 		Method: ThreadPoolWait();
 * //----------------------------------------------------------//
 * Waits for all tasks in the pool to finish.
 */
void ThreadPoolWait(ThreadPool *pool);

/**
 * 		Method: ThreadPoolShutdown();
 * //----------------------------------------------------------//
 * Destructor for ThreadPool. This function will automatically wait for
 * the worker threads to finish all of the scheduled tasks.
 */
void ThreadPoolShutdown(ThreadPool *pool);

#endif