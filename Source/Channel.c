#include "../Headers/Channel.h"


//----------------------------------------------------------//

void ChannelInit(Channel *channel, int elemSize, FreeFn freeFn, int maxCapacity) {
	
	assert(channel != NULL);
	assert(elemSize > 0);
	assert(maxCapacity >= 0);

	QueueInit(&channel->itemQueue, elemSize, freeFn, 0);

	channel->elemSize = elemSize;
	channel->maxCapacity = maxCapacity;
	channel->freeFn = freeFn;

	pthread_mutex_init(&channel->itemLock, NULL);

	pthread_cond_init(&channel->sendCond, NULL);
	pthread_cond_init(&channel->getCond, NULL);
}

//----------------------------------------------------------//

void ChannelSend(Channel *channel, const void *elemAddr) {

	assert(channel != NULL);
	assert(elemAddr != NULL);

	pthread_mutex_lock(&channel->itemLock);
	while(channel->maxCapacity > 0 && QueueSize(&channel->itemQueue) >= channel->maxCapacity) {
		pthread_cond_wait(&channel->sendCond, &channel->itemLock);
	}

	QueueEnqueue(&channel->itemQueue, elemAddr);

	pthread_cond_broadcast(&channel->getCond);
	pthread_mutex_unlock(&channel->itemLock);
}

//----------------------------------------------------------//

void ChannelGet(Channel *channel, void *buffer) {

	assert(channel != NULL);
	assert(buffer != NULL);

	pthread_mutex_lock(&channel->itemLock);
	while(QueueIsEmpty(&channel->itemQueue)) {
		pthread_cond_wait(&channel->getCond, &channel->itemLock);
	}

	QueueDequeue(&channel->itemQueue, buffer);
	
	pthread_cond_broadcast(&channel->sendCond);
	pthread_mutex_unlock(&channel->itemLock);
}

//----------------------------------------------------------//

int	ChannelLength(Channel *channel) {

	assert(channel != NULL);

	return QueueSize(&channel->itemQueue);
}

//----------------------------------------------------------//

void ChannelResize(Channel *channel, int newCapacity) {

	assert(channel != NULL);
	assert(newCapacity >= 0);

	pthread_mutex_lock(&channel->itemLock);
	while(QueueSize(&channel->itemQueue) > newCapacity) {

		void *buffer = malloc(channel->elemSize);
		QueueDequeue(&channel->itemQueue, buffer);
		
		if(channel->freeFn != NULL) channel->freeFn(buffer);
		free(buffer);
	}

	channel->maxCapacity = newCapacity;

	pthread_mutex_unlock(&channel->itemLock);
}

//----------------------------------------------------------//

void ChannelDispose(Channel *channel) {

	assert(channel != NULL);
	
	QueueDispose(&channel->itemQueue);

	pthread_mutex_destroy(&channel->itemLock);
	pthread_cond_destroy(&channel->sendCond);
	pthread_cond_destroy(&channel->getCond);
}
