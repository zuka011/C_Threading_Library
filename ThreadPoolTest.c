#include <stdio.h>
#include <stdlib.h>
#include "ThreadPool.h"
#include "pthread.h"

#define min(a, b) (a < b ? a : b)

#define N_ITEMS 900
#define N_WORKERS 30
#define GROUP_SIZE ((N_ITEMS - 1)/N_WORKERS + 1)
#define POOL_SIZE 5

typedef struct {

	int items[N_ITEMS];

	unsigned long sum;
	pthread_mutex_t sumLock;

	double average;

	int currGroup;
	pthread_mutex_t groupLock;

} Data;

void fillData(Data *data) {

	unsigned long precalculatedSum = 0;
	data->sum = 0;

	for(int i = 0; i < N_ITEMS; i++) {

		int currItem = rand();
		precalculatedSum += currItem;

		data->items[i] = currItem;
	}

	printf("Actual sum of items is: %lu\n", precalculatedSum);
}

void getSum(void *dataPtr) {

	Data *data = (Data *) dataPtr;

	pthread_mutex_lock(&data->groupLock);
	int currGroup = data->currGroup++;
	pthread_mutex_unlock(&data->groupLock);

	int startIndex = currGroup * GROUP_SIZE;
	int endIndex = min((currGroup + 1) * GROUP_SIZE, N_ITEMS);

	printf("Worker N%d summing from %d to %d.\n", currGroup + 1, startIndex, endIndex);

	unsigned long sum = 0;

	for(int i = startIndex; i < endIndex; i++) sum += data->items[i];

	pthread_mutex_lock(&data->sumLock);
	data->sum += sum;
	pthread_mutex_unlock(&data->sumLock);

	printf("Worker N%d done. sum: %lu\n", currGroup + 1, sum);
}

void getAverage(void *dataPtr) {

	Data *data = (Data *) dataPtr;

	data->average = data->sum/(double) N_ITEMS;
	printf("Average calculated.\n");
}

void displayAverage(void *averagePtr) {

	printf("Average of %d items is: %f.\n", N_ITEMS, *(double *) averagePtr);
}

int main() {

	srand(time(NULL));

	ThreadPool pool;
	ThreadPoolInit(&pool, POOL_SIZE);

	Data data;
	fillData(&data);

	data.currGroup = 0;
	pthread_mutex_init(&data.sumLock, NULL);
	pthread_mutex_init(&data.groupLock, NULL);

	printf("Scheduling threads...\n");
	for(int i = 0; i < N_WORKERS; i++) ThreadPoolSchedule(&pool, getSum, &data);
	printf("Done.\n");

	ThreadPoolWait(&pool);

	ThreadPoolSchedule(&pool, getAverage, &data);

	ThreadPoolWait(&pool);

	ThreadPoolSchedule(&pool, displayAverage, &data.average);

	ThreadPoolShutdown(&pool);

	printf("Calculated sum by the workers is: %lu\n", data.sum);
	pthread_mutex_destroy(&data.sumLock);
	pthread_mutex_destroy(&data.groupLock);

	return 0;
}