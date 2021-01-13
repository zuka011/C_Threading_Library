#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "ThreadPool.h"
#include "Channel.h"

#define min(a, b) (a < b ? a : b)

#define N_ITEMS 900
#define N_WORKERS 30
#define GROUP_SIZE ((N_ITEMS - 1)/N_WORKERS + 1)
#define POOL_SIZE 5

typedef struct {

	int items[N_ITEMS];

	Channel groupNumberChannel;
	Channel partialSumChannel;
	Channel finalSumChannel;
	Channel finalAverageChannel;

} Data;

void fillData(Data *data) {

	unsigned long precalculatedSum = 0;

	for(int i = 0; i < N_ITEMS; i++) {

		int currItem = rand();
		precalculatedSum += currItem;

		data->items[i] = currItem;
	}

	printf("Actual sum of items is: %lu\n", precalculatedSum);
}

void divideItems(void *channelPtr) {

	Channel *channel = (Channel *) channelPtr;
	for(int i = 0; i < N_WORKERS; i++) ChannelSend(channel, &i);
}

void getGroupSum(void *dataPtr) {

	Data *data = (Data *) dataPtr;

	int *currGroupPtr = (int *) ChannelGet(&data->groupNumberChannel);
	int currGroup = *currGroupPtr;

	free(currGroupPtr);

	int startIndex = currGroup * GROUP_SIZE;
	int endIndex = min((currGroup + 1) * GROUP_SIZE, N_ITEMS);

	printf("Worker N%d summing from %d to %d.\n", currGroup + 1, startIndex, endIndex);

	unsigned long sum = 0;

	for(int i = startIndex; i < endIndex; i++) sum += data->items[i];

	ChannelSend(&data->partialSumChannel, &sum);

	printf("Worker N%d done. sum: %lu\n", currGroup + 1, sum);
}

void getSum(void *dataPtr) {

	Data *data = (Data *) dataPtr;	
	unsigned long sum = 0;

	for(int i = 0; i < N_WORKERS; i++) {
	
		unsigned long *currSum = (unsigned long *) ChannelGet(&data->partialSumChannel); 
		sum += *currSum;

		free(currSum);
	}

	printf("Calculated sum by the workers is: %lu\n", sum);
	ChannelSend(&data->finalSumChannel, &sum);
}

void getAverage(void *dataPtr) {

	Data *data = (Data *) dataPtr;

	unsigned long *sum = (unsigned long *) ChannelGet(&data->finalSumChannel);
	double average = *sum /(double) N_ITEMS;

	ChannelSend(&data->finalAverageChannel, &average);
	printf("Average calculated.\n");

	free(sum);
}

void displayAverage(void *channelPtr) {

	Channel *channel = (Channel *) channelPtr;

	double *average = (double *) ChannelGet(channel);
	printf("Average of %d items is: %f.\n", N_ITEMS, *average);

	free(average);
}

int main() {

	srand(time(NULL));

	ThreadPool pool;
	ThreadPoolInit(&pool, POOL_SIZE);

	Data data;
	fillData(&data);

	ChannelInit(&data.groupNumberChannel, sizeof(int), NULL, 0);
	ChannelInit(&data.partialSumChannel, sizeof(unsigned long), NULL, 0);
	ChannelInit(&data.finalSumChannel, sizeof(unsigned long), NULL, 0);
	ChannelInit(&data.finalAverageChannel, sizeof(double), NULL, 0);

	printf("Dividing workload...\n");
	ThreadPoolSchedule(&pool, divideItems, &data.groupNumberChannel);
	printf("Done (workload).\n");

	printf("Scheduling workers...\n");
	for(int i = 0; i < N_WORKERS; i++) ThreadPoolSchedule(&pool, getGroupSum, &data);
	printf("Done (scheduling).\n");

	ThreadPoolSchedule(&pool, getSum, &data);	

	ThreadPoolSchedule(&pool, getAverage, &data);

	ThreadPoolSchedule(&pool, displayAverage, &data.finalAverageChannel);

	ThreadPoolShutdown(&pool);

	printf("Thread pool has shut down.\n");

	return 0;
}