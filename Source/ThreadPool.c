#include "../Headers/ThreadPool.h"

typedef struct {

	Process processFn;
	void *processArgs;
} Task;

static void *ThreadPoolWorker(void *poolAddr) {

	ThreadPool *pool = (ThreadPool *) poolAddr;

	while(1) {

		Task currTask;
		QueueDequeue(&pool->taskQueue, &currTask);

		if(currTask.processFn == NULL) break;

		currTask.processFn(currTask.processArgs);

		pthread_mutex_lock(&pool->taskLock);
		pool->tasksLeft--;
		if(pool->tasksLeft == 0) pthread_cond_broadcast(&pool->taskCond);
		pthread_mutex_unlock(&pool->taskLock);
	}

	return NULL;
}

static void ThreadPoolEnqueueTask(ThreadPool *pool, void (*fn)(void *), void *args) {

	Task newTask;
	newTask.processFn = fn;
	newTask.processArgs = args;

	QueueEnqueue(&pool->taskQueue, &newTask);
}

//----------------------------------------------------------//

void ThreadPoolInit(ThreadPool *pool, int poolSize) {

	assert(pool != NULL);
	assert(poolSize > 0);

	pool->poolSize = poolSize;
	pool->threadID = malloc(poolSize * sizeof(pthread_t));
	pool->tasksLeft = 0;

	pthread_mutex_init(&pool->taskLock, NULL);
	pthread_cond_init(&pool->taskCond, NULL);
	QueueInit(&pool->taskQueue, sizeof(Task), NULL, 1);

	for(int i = 0; i < poolSize; i++) pthread_create(pool->threadID + i, NULL, ThreadPoolWorker, pool);
}

//----------------------------------------------------------//

void ThreadPoolSchedule(ThreadPool *pool, void (*fn)(void *), void *args) {

	assert(pool != NULL);
	assert(fn != NULL);

	pthread_mutex_lock(&pool->taskLock);
	pool->tasksLeft++;
	pthread_mutex_unlock(&pool->taskLock);

	ThreadPoolEnqueueTask(pool, fn, args);
}

//----------------------------------------------------------//

int ThreadPoolIsRunning(ThreadPool *pool) {

	assert(pool != NULL);

	return pool->tasksLeft > 0;
}

//----------------------------------------------------------//

void ThreadPoolWait(ThreadPool *pool) {

	assert(pool != NULL);

	pthread_mutex_lock(&pool->taskLock);

	while(pool->tasksLeft > 0) pthread_cond_wait(&pool->taskCond, &pool->taskLock);
	pthread_mutex_unlock(&pool->taskLock);
}

//----------------------------------------------------------//

void ThreadPoolShutdown(ThreadPool *pool) {

	assert(pool != NULL);

	// Signal to finish executing tasks.
	for(int i = 0; i < pool->poolSize; i++) ThreadPoolEnqueueTask(pool, NULL, NULL);
	
	for(int i = 0; i < pool->poolSize; i++) pthread_join(pool->threadID[i], NULL);

	free(pool->threadID);
	pthread_mutex_destroy(&pool->taskLock);
}
