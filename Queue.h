#ifndef _QUEUE_H_2021
#define _QUEUE_H_2021

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "pthread.h"

/**
 * This is the interface for a generic queue data structure.
 * 
 * The queue is implemented using a linked list and all methods work in 
 * O(1) time (except for the destructor). It can also work as a thread safe
 * blocking queue if the option is chosen during initialization.
 */

typedef void (*FreeFn)(void *);

typedef struct QueueNode {
	
	void *valueAddr;
	struct QueueNode *next;	
} QueueNode;

typedef struct {

	pthread_mutex_t QueueLock;
	pthread_cond_t QueueCond;

	QueueNode *head;
	QueueNode *tail;
	int elemSize;
	int nElems;
	int threadSafe;

	FreeFn freeFn;

} Queue;

/**
 * 		Method: QueueInit();
 * //----------------------------------------------------------//
 * Constructor for the queue. 
 * 
 * Parameters:
 * 	[elemSize]		- number of bytes needed to store one element
 * 	[freeFn]		- free function called in the destructor on every element
 * 	[threadSafe]	- queue is thread safe if this value is nonzero, otherwise it is a regular queue. 
 */
void QueueInit(Queue *queue, int elemSize, FreeFn freeFn, int threadSafe);

/**
 * 		Method: QueueEnqueue();
 * //----------------------------------------------------------//
 * Enqueues the element at address [elemAddr] to the queue. The element is copied to a new
 * location in the heap.
 */
void QueueEnqueue(Queue *queue, void *elemAddr);

/**
 * 		Method: QueueDequeue();
 * //----------------------------------------------------------//
 * Dequeues an element from the queue. A pointer to the element is returned. 
 * It is the user's responsibility to free the memory occupied by this element.
 */
void *QueueDequeue(Queue *queue);

/**
 * 		Method: QueuePeek();
 * //----------------------------------------------------------//
 * Returns a pointer to the first element in the queue. The element still belongs 
 * to the queue so it should not be freed.
 */
void *QueuePeek(Queue *queue);

/**
 * 		Method: QueueSize();
 * //----------------------------------------------------------//
 * Returns the number of elements in the queue. 
 */
int QueueSize(Queue *queue);

/**
 * 		Method: QueueIsEmpty();
 * //----------------------------------------------------------//
 * Returns a non-zero value if queue is empty, 0 otherwise.
 */
int	QueueIsEmpty(Queue *queue);

/**
 * 		Method: QueueDispose();
 * //----------------------------------------------------------//
 * Destructor for the queue.
 * This (like the constructor) should only be called once (even if you're using
 * multiple threads).
 */
void QueueDispose(Queue *queue);

#endif