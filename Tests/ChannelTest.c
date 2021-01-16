#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../Headers/ThreadPool.h"
#include "../Headers/Channel.h"

#define min(a, b) (a < b ? a : b)

#define N_ITEMS 900
#define N_WORKERS 30
#define GROUP_SIZE ((N_ITEMS - 1)/N_WORKERS + 1)
#define POOL_SIZE 5
#define CHANNEL_TEST_LIMIT 5

unsigned long precalculatedSum = 0;
double precalculatedAverage = 0;

int verboseOutput = 0;

typedef struct {

	int items[N_ITEMS];

	Channel groupNumberChannel;
	Channel partialSumChannel;
	Channel finalSumChannel;
	Channel finalAverageChannel;

} Data;

void fillData(Data *data) {

	precalculatedSum = 0;

	for(int i = 0; i < N_ITEMS; i++) {

		int currItem = rand();
		precalculatedSum += currItem;

		data->items[i] = currItem;
	}

	if(verboseOutput) printf("Actual sum of items is: %lu\n", precalculatedSum);
	precalculatedAverage = precalculatedSum/(double) N_ITEMS;
}

void divideItems(void *channelPtr) {

	Channel *channel = (Channel *) channelPtr;
	for(int i = 0; i < N_WORKERS; i++) ChannelSend(channel, &i);
}

void getGroupSum(void *dataPtr) {

	Data *data = (Data *) dataPtr;

	int currGroup;
	ChannelGet(&data->groupNumberChannel, &currGroup);

	int startIndex = currGroup * GROUP_SIZE;
	int endIndex = min((currGroup + 1) * GROUP_SIZE, N_ITEMS);

	if(verboseOutput) printf("Worker N%d summing from %d to %d.\n", currGroup + 1, startIndex, endIndex);

	unsigned long sum = 0;

	for(int i = startIndex; i < endIndex; i++) sum += data->items[i];

	ChannelSend(&data->partialSumChannel, &sum);

	if(verboseOutput) printf("Worker N%d done. sum: %lu\n", currGroup + 1, sum);
}

void getSum(void *dataPtr) {

	Data *data = (Data *) dataPtr;	
	unsigned long sum = 0;

	for(int i = 0; i < N_WORKERS; i++) {
	
		unsigned long currSum;
		ChannelGet(&data->partialSumChannel, &currSum); 
		
		sum += currSum;
	}

	if(verboseOutput) printf("Calculated sum by the workers is: %lu\n", sum);
	ChannelSend(&data->finalSumChannel, &sum);

	if(precalculatedSum == sum) printf("Success. The sum was correctly calculated.\n");
	else printf("Failure. The sum was incorrectly calculated. Should be: %lu but instead was %lu.\n", precalculatedSum, sum);
}

void getAverage(void *dataPtr) {

	Data *data = (Data *) dataPtr;

	unsigned long sum;
	ChannelGet(&data->finalSumChannel, &sum);

	double average = sum /(double) N_ITEMS;

	ChannelSend(&data->finalAverageChannel, &average);
	if(verboseOutput) printf("Average calculated.\n");
}

void displayAverage(void *channelPtr) {

	Channel *channel = (Channel *) channelPtr;

	double average;
	ChannelGet(channel, &average);
	if(verboseOutput) printf("Average of %d items is: %f.\n", N_ITEMS, average);

	if(precalculatedAverage == average) printf("Success. The average was correctly calculated.\n");
	else printf("Failure. The average was incorrectly calculated. Should be: %f but instead was %f.\n", precalculatedAverage, average);
}

int checkCommandLineArgs(int nArgs, char *args[]) {

	if(nArgs > 2) {

		printf("Too many command line arguments. You can request verbose output by adding -v.\n");
		return 1;
	}

	if(nArgs == 2) {

		if(strcmp(args[1], "-v") == 0) verboseOutput = 1;
		else {
			printf("Not a valid argument. You can request verbose output by adding -v.\n");
			return 1;
		} 
	}
	return 0;
}

int main(int nArgs, char *args[]) {

	if(checkCommandLineArgs(nArgs, args)) return 1;

	srand(time(NULL));

	ThreadPool pool;
	ThreadPoolInit(&pool, POOL_SIZE);

	Data data;
	fillData(&data);

	ChannelInit(&data.groupNumberChannel, sizeof(int), NULL, CHANNEL_TEST_LIMIT);
	ChannelInit(&data.partialSumChannel, sizeof(unsigned long), NULL, 0);
	ChannelInit(&data.finalSumChannel, sizeof(unsigned long), NULL, 0);
	ChannelInit(&data.finalAverageChannel, sizeof(double), NULL, 0);

	if(verboseOutput) printf("Dividing workload...\n");
	ThreadPoolSchedule(&pool, divideItems, &data.groupNumberChannel);
	if(verboseOutput) printf("Done (workload).\n");

	if(verboseOutput) printf("Scheduling workers...\n");
	for(int i = 0; i < N_WORKERS; i++) ThreadPoolSchedule(&pool, getGroupSum, &data);
	if(verboseOutput) printf("Done (scheduling).\n");

	ThreadPoolSchedule(&pool, getSum, &data);	

	ThreadPoolSchedule(&pool, getAverage, &data);

	ThreadPoolSchedule(&pool, displayAverage, &data.finalAverageChannel);

	ThreadPoolShutdown(&pool);

	if(verboseOutput) printf("Thread pool has shut down.\n");

	//Testing ChannelResize() method:
	Channel testChannel;

	const int N_RESIZE_TEST_ELEMS = 5;

	ChannelInit(&testChannel, sizeof(int), NULL, N_RESIZE_TEST_ELEMS);
	for(int i = 0; i < N_RESIZE_TEST_ELEMS; i++) ChannelSend(&testChannel, &i);

	ChannelResize(&testChannel, 1);

	int lastElem;
	
	ChannelGet(&testChannel, &lastElem);
	if(lastElem == N_RESIZE_TEST_ELEMS - 1) printf("Success. Last elem was correctly received.\n");
	else printf("Fail. Last element in the channel was %d, it should have been %d.\n", lastElem, N_RESIZE_TEST_ELEMS - 1);

	if(ChannelLength(&testChannel) > 0) printf("Ooops, the channel still isn't empty.\n");
	else printf("Success. Channel is empty.\n");

	return 0;
}