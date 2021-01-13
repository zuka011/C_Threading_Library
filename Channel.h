#ifndef _CHANNEL_H_2021
#define _CHANNEL_H_2021

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "Queue.h"

/**
 * 	This is the interface of a generic undirected channel structure.
 * 
 * 	A channel allows threads to communicate information (items) with each other without
 * 	causing any concurrency issues. This is also a blocking channel.
 *
 * 	This implementation of a channel uses a blocking queue to provide all the 
 * 	functionality.
 */

typedef void (*FreeFn)(void *);

typedef struct {

	Queue itemQueue;
	int maxCapacity;

	pthread_mutex_t itemLock;

	pthread_cond_t 	sendCond;
	pthread_cond_t 	getCond;

	FreeFn freeFn;

} Channel;


/**
 * 		Method: ChannelInit();
 * //----------------------------------------------------------//
 * 	Initializes the passed channel to accomodate items of [elemSize] bytes. You
 * can also specify a maximum capacity [maxCapacity] for this channel (or 0 if you
 * do not wish for the channel to have a maximum capacity). 
 */
void ChannelInit(Channel *channel, int elemSize, FreeFn freeFn, int maxCapacity);

/**
 * 		Method: ChannelSend();
 * //----------------------------------------------------------//
 * 	Sends the data (item) pointed by [elemAddr] through the channel. 
 * 
 * The item is copied to a new location on the heap. If the maximum capacity of
 * a channel is reached and a thread is attempting to send something through, it 
 * will get blocked until another thread reads a previously sent item.
 */
void ChannelSend(Channel *channel, void *elemAddr);

/**
 * 		Method: ChannelGet();
 * //----------------------------------------------------------//
 * 	Reads an item from the channel.
 * 
 * If there are no items available, the thread will get blocked until an item
 * is sent into the channel. The returned address will be in the heap, so it is
 * the client's responsibility to free up the memory after use.
 */
void *ChannelGet(Channel *channel);

/**
 * 		Method: ChannelLength();
 * //----------------------------------------------------------//
 * 	Returns the number of items currently available in the channel.
 */
int	ChannelLength(Channel *channel);

/**
 * 		Method: ChannelResize();
 * //----------------------------------------------------------//
 * 	Sets the maximum capacity of the channel to [newCapacity].
 * 
 * If the new capacity is less than the current length of the channel, all
 * extra items are discarded.
 */
void ChannelResize(Channel *channel, int newCapacity);

/**
 * 		Method: ChannelDispose();
 * //----------------------------------------------------------//
 * 	Destructor for the channel. All remaining items in the channel are
 * discarded.
 */
void ChannelDispose(Channel *channel);

#endif