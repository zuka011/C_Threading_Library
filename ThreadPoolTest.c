#include <stdio.h>
#include <stdlib.h>
#include "ThreadPool.h"
#include "pthread.h"

#define min(a, b) (a < b ? a : b)

#define N_ITEMS 900
#define N_WORKERS 30
#define GROUP_SIZE ((N_ITEMS - 1)/N_WORKERS + 1)
#define POOL_SIZE 5

unsigned long precalculatedSum = 0;
double precalculatedAverage = 0;

int verboseOutput = 0;

typedef struct {

	int items[N_ITEMS];

	unsigned long sum;
	pthread_mutex_t sumLock;

	double average;

	int currGroup;
	pthread_mutex_t groupLock;

} Data;

void fillData(Data *data) {

	data->sum = 0;

	for(int i = 0; i < N_ITEMS; i++) {

		int currItem = rand();
		precalculatedSum += currItem;

		data->items[i] = currItem;
	}

	if(verboseOutput) printf("Actual sum of items is: %lu\n", precalculatedSum);
	precalculatedAverage = precalculatedSum/(double) N_ITEMS;
}

void getSum(void *dataPtr) {

	Data *data = (Data *) dataPtr;

	pthread_mutex_lock(&data->groupLock);
	int currGroup = data->currGroup++;
	pthread_mutex_unlock(&data->groupLock);

	int startIndex = currGroup * GROUP_SIZE;
	int endIndex = min((currGroup + 1) * GROUP_SIZE, N_ITEMS);

	if(verboseOutput) printf("Worker N%d summing from %d to %d.\n", currGroup + 1, startIndex, endIndex);

	unsigned long sum = 0;

	for(int i = startIndex; i < endIndex; i++) sum += data->items[i];

	pthread_mutex_lock(&data->sumLock);
	data->sum += sum;
	pthread_mutex_unlock(&data->sumLock);

	if(verboseOutput) printf("Worker N%d done. sum: %lu\n", currGroup + 1, sum);
}

void getAverage(void *dataPtr) {

	Data *data = (Data *) dataPtr;

	data->average = data->sum/(double) N_ITEMS;
	if(verboseOutput) printf("Average calculated.\n");
}

void displayAverage(void *averagePtr) {

	if(verboseOutput) printf("Average of %d items is: %f.\n", N_ITEMS, *(double *) averagePtr);
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

	data.currGroup = 0;
	pthread_mutex_init(&data.sumLock, NULL);
	pthread_mutex_init(&data.groupLock, NULL);

	if(verboseOutput) printf("Scheduling threads...\n");
	for(int i = 0; i < N_WORKERS; i++) ThreadPoolSchedule(&pool, getSum, &data);
	if(verboseOutput) printf("Done.\n");

	ThreadPoolWait(&pool);

	ThreadPoolSchedule(&pool, getAverage, &data);

	ThreadPoolWait(&pool);

	ThreadPoolSchedule(&pool, displayAverage, &data.average);

	ThreadPoolShutdown(&pool);

	if(verboseOutput) printf("Calculated sum by the workers is: %lu\n", data.sum);
	pthread_mutex_destroy(&data.sumLock);
	pthread_mutex_destroy(&data.groupLock);

	if(precalculatedSum == data.sum) printf("Success. The sum was correctly calculated.\n");
	else printf("Failure. The sum was incorrectly calculated. Should be: %lu but instead was %lu.\n", precalculatedSum, data.sum);

	if(precalculatedAverage == data.average) printf("Success. The average was correctly calculated.\n");
	else printf("Failure. The average was incorrectly calculated. Should be: %f but instead was %f.\n", precalculatedAverage, data.average);

	return 0;
}