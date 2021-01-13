#include "Future.h"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
 
typedef enum {IN_PROGRESS, DONE, CANCELED	} State;
typedef enum {CANCEL, GET	} Request;

static void *Executor(void *futurePtr) {

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	Future *future = (Future *) futurePtr;

	future->result = future->fn(future->args);
	future->state = DONE;

	sem_post(&future->done);

	return NULL;
}

static void *Manager(void *futurePtr) {
	
	Future *future = (Future *) futurePtr;

	while(1) {

		sem_wait(&future->requested);
		if(future->state == DONE) break;

		if(future->requestType == GET && future->timeout != 0) sem_post(&future->timerSet);
		else if(future->requestType == CANCEL) {
			
			pthread_cancel(future->executorID);
			break;
		}
	}

	void *result;
	pthread_join(future->executorID, &result);

	if(future->state != DONE && result != PTHREAD_CANCELED) perror("Failed to cancel Future execution.\n"); 
	if(future->state != DONE) future->state == CANCELED;

	sem_post(&future->timerSet);

	pthread_join(future->timerID, NULL);

	return NULL;
}

static void *Timer(void *futurePtr) {

	Future *future = (Future *) futurePtr;

	while(1) {

		sem_wait(&future->timerSet);
		if(future->state == DONE && future->state == CANCELED) break;

		sleep(future->timeout);

		sem_post(&future->done);
	}
	return NULL;
}

//----------------------------------------------------------//

void FutureInit(Future *future, FutureFunction fn, void *args) {

	future->fn = fn;
	future->args = args;

	future->state = IN_PROGRESS;

	future->result = NULL;

	sem_init(&future->requested, 0, 0);
	sem_init(&future->timerSet, 0, 0);	
	sem_init(&future->done, 0, 0);

	pthread_create(&future->executorID, NULL, Executor, future);
	pthread_create(&future->managerID, NULL, Manager, future);
	pthread_create(&future->timerID, NULL, Timer, future);
}

//----------------------------------------------------------//

int FutureGet(Future *future, void **buffer, unsigned long timeout) {

	future->timeout = timeout;
	future->requestType = GET;

	sem_post(&future->requested);
	sem_wait(&future->done);
	
	*buffer = future->result;
	
	if(future->state == DONE) return EXIT_SUCCESS;
	return EXIT_FAILURE;
}


//----------------------------------------------------------//

int FutureCancel(Future *future) {

	if(future->state == DONE || future->state == CANCELED) return EXIT_FAILURE;

	future->requestType = CANCEL;
	sem_post(&future->requested);

	return EXIT_SUCCESS;
}

//----------------------------------------------------------//

int FutureIsDone(Future *future) {
	return future->state == DONE;
}

//----------------------------------------------------------//

int FutureIsCancelled(Future *future) {
	return future->state == CANCELED;
}

//----------------------------------------------------------//

void FutureDispose(Future *future) {

	if(future->state != DONE && future->state != CANCELED) FutureCancel(future);
	
	pthread_join(future->managerID, NULL);
	
	sem_destroy(&future->requested);
	sem_destroy(&future->timerSet);	
	sem_destroy(&future->done);
}