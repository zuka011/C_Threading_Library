#include "Queue.h"

static void QueueWait(Queue *queue) {

	if(queue->threadSafe) {
	
		pthread_mutex_lock(&queue->QueueLock);
		while(QueueIsEmpty(queue)) pthread_cond_wait(&queue->QueueCond, &queue->QueueLock);
	} else assert(!QueueIsEmpty(queue));
}

static void QueueNodeInit(QueueNode *queueNode, void *elemAddr, int elemSize) {

	queueNode->next = NULL;
	queueNode->valueAddr = malloc(elemSize);
	memcpy(queueNode->valueAddr, elemAddr, elemSize);
} 

//----------------------------------------------------------//

void QueueInit(Queue *queue, int elemSize, FreeFn freeFn, int threadSafe) {

	assert(queue != NULL);
	assert(elemSize > 0);

	queue->threadSafe = threadSafe;
	queue->elemSize = elemSize;
	queue->freeFn = freeFn;
	queue->head = NULL;
	queue->tail = NULL;
	queue->nElems = 0;

	if(threadSafe) {
		
		pthread_mutex_init(&queue->QueueLock, NULL);
		pthread_cond_init(&queue->QueueCond, NULL);
	}
}

//----------------------------------------------------------//

void QueueEnqueue(Queue *queue, void *elemAddr) {

	assert(queue != NULL);
	assert(elemAddr != NULL);
	
	QueueNode *newNode = malloc(sizeof(QueueNode));
	QueueNodeInit(newNode, elemAddr, queue->elemSize);

	if(queue->threadSafe) pthread_mutex_lock(&queue->QueueLock);

	if(queue->head == NULL) {
		queue->head = newNode;
		queue->tail = newNode;
	} else {

		queue->head->next = newNode;
		queue->head = newNode;
	}

	queue->nElems++;

	if(queue->threadSafe) {
		pthread_cond_broadcast(&queue->QueueCond);
		pthread_mutex_unlock(&queue->QueueLock);
	}
}

//----------------------------------------------------------//

void QueueDequeue(Queue *queue, void *buffer){

	assert(queue != NULL);
	assert(buffer != NULL);

	QueueWait(queue);

	QueueNode *currNode = queue->tail;
	void *valueAddr = currNode->valueAddr;

	queue->tail = queue->tail->next;
	if(queue->tail == NULL) queue->head = NULL;

	queue->nElems--;

	if(queue->threadSafe) pthread_mutex_unlock(&queue->QueueLock);

	memcpy(buffer, valueAddr, queue->elemSize);
	free(currNode);
	free(valueAddr);
}

//----------------------------------------------------------//

void *QueuePeek(Queue *queue) {

	assert(queue != NULL);

	QueueWait(queue);

	return queue->tail->valueAddr;
}

//----------------------------------------------------------//

int QueueSize(Queue *queue) {
	return queue->nElems;
}

//----------------------------------------------------------//

int	QueueIsEmpty(Queue *queue) {

	assert(queue != NULL);
	if(queue->tail == NULL) assert(queue->head == NULL);

	return queue->tail == NULL;
}

//----------------------------------------------------------//

void QueueDispose(Queue *queue) {

	assert(queue != NULL);

	while(queue->tail != NULL) {

		QueueNode *next = queue->tail->next;

		if(queue->freeFn != NULL) queue->freeFn(queue->tail->valueAddr);
		free(queue->tail->valueAddr);
		free(queue->tail);

		queue->tail = next;
	}

	if(queue->threadSafe) {
		pthread_cond_destroy(&queue->QueueCond);
		pthread_mutex_destroy(&queue->QueueLock);
	}
}