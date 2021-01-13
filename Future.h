#ifndef _FUTURE_H_2021
#define _FUTURE_H_2021

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

/**
 * 	This is the interface for a generic Future structure.
 * 
 * 	A Future can be used to perform a computation asynchronously and receive the result
 * 	when it is ready. The client can also request to cancel a computation midway.
 */

typedef void *(*FutureFunction)(void *args);

typedef struct {

	pthread_t executorID;
	pthread_t managerID;
	pthread_t timerID;

	FutureFunction fn;
	void *args;

	int state;
	int requestType;
	unsigned long timeout;

	void *result;

	sem_t requested;
	sem_t timerSet;
	sem_t done;

} Future;


/**
 * 		Method: FutureInit();
 * //----------------------------------------------------------//
 * 	Initializes the Future structure and begins executing the given function
 * [fn] in a new thread. The passed function must accept and return a void pointer.
 * [args] is the argument passed to the function.
 */
void FutureInit(Future *future, FutureFunction fn, void *args);

/**
 * 		Method: FutureGet();
 * //----------------------------------------------------------//
 * 	Waits for the computation to finish for at most [timeout] seconds and retrieves 
 * the result if ready. The result of the computation is written into the address 
 * specified by [buffer]. The method will return a 0 if the result was retrieved,
 * else it will return a non-zero value. set [timeout] to 0 if you want to wait 
 * for the result indefinitely.
 */
int FutureGet(Future *future, void **buffer, unsigned long timeout);

/**
 * 		Method: FutureCancel();
 * //----------------------------------------------------------//
 * 	Requests the computation to be canceled. will return 0 if the request
 * was succesfully issued, else it will return a non-zero value.
 */
int FutureCancel(Future *future);

/**
 * 		Method: FutureIsDone();
 * //----------------------------------------------------------//
 * 	Returns a non-zero value if the computation is done, else 0.
 */
int FutureIsDone(Future *future);

/**
 * 		Method: FutureIsCancelled();
 * //----------------------------------------------------------//
 * 	Returns a non-zero value if the computation was canceled, else 0. This 
 * method may still return 0 despite having already called FutureCancel(). A computation
 * may not cancel immediately upon request.
 */
int FutureIsCancelled(Future *future);

/**
 * 		Method: FutureDispose();
 * //----------------------------------------------------------//
 * 	The destructor for the Future structure.
 */
void FutureDispose(Future *future);

#endif