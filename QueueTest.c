#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "Queue.h"
#include "pthread.h"

#define N_TEST_THREADS 10
#define MIN_ITEMS 1
#define MAX_ITEMS 10

typedef struct {

	int threadID;
	Queue *testQueue;

} ThreadArgs;

int testSuccess = 1;

void intTest() {

	Queue queue;
	QueueInit(&queue, sizeof(int), NULL, 0);

	for(int i = 0; i < 99; i++) {

		QueueEnqueue(&queue, &i);
	}

	for(int i = 0; i < 90; i++) {

		int dequeuedInt;
		QueueDequeue(&queue, &dequeuedInt);

		if(i != dequeuedInt) {

			testSuccess = 0;
			printf("Fail at i = %d\n", i);	
			break;
		}
	}

	QueueDispose(&queue);
}

typedef struct {

	int someFieldOne;
	int someFieldTwo;

} TestStruct;

void structTest() {

	Queue queue;
	QueueInit(&queue, sizeof(TestStruct), NULL, 0);

	for(int i = 0; i < 99; i++) {

		TestStruct currStruct;
		currStruct.someFieldOne = i*i;
		currStruct.someFieldTwo = i << (i % 30);

		QueueEnqueue(&queue, &currStruct);
	}

	for(int i = 0; i < 90; i++) {

		TestStruct dequeuedStruct;
		QueueDequeue(&queue, &dequeuedStruct);

		if(i*i != dequeuedStruct.someFieldOne || i << (i % 30) != dequeuedStruct.someFieldTwo) {

			testSuccess = 0;
			printf("Fail at i = %d\n", i);	
			break;
		}
	}

	QueueDispose(&queue);
}

void stringFree(void *stringPtrPtr) {

	free(*(char **) stringPtrPtr);
}

void stringTest() {

	Queue queue;
	QueueInit(&queue, sizeof(char *), stringFree, 0);

	for(int i = 0; i < 99; i++) {

		char *newString = malloc(10 * sizeof(char));
		for(int j = 0; j < 9; j++) newString[j] = i + j;
		newString[9] = '\0';

		QueueEnqueue(&queue, &newString);
	}

	for(int i = 0; i < 90; i++) {

		char string[10];
		for(int j = 0; j < 9; j++) string[j] = i + j;
		string[9] = '\0';

		char *dequeuedStringAddr;
		QueueDequeue(&queue, &dequeuedStringAddr);
		
		if(strcmp(string, dequeuedStringAddr)) {

			testSuccess = 0;
			printf("Fail at i = %d\n", i);	
			stringFree(&dequeuedStringAddr);
			break;
		}

		stringFree(&dequeuedStringAddr);
	}

	QueueDispose(&queue);
}

int randomInt(int from, int to) {
	return from + rand() % (to - from + 1);
}

void randomSleep(int from, int to) {
	sleep(randomInt(from, to));
}

void *testThread(void *args) {

	randomSleep(0, 1);

	Queue *testQueue = ((ThreadArgs *) args)->testQueue;
	int id = ((ThreadArgs *) args)->threadID;

	int nItems = randomInt(MIN_ITEMS, MAX_ITEMS);
	printf("Thread #%d enqueueing %d items.\n", id, nItems);

	for(int i = 0; i < nItems; i++) {
	
		int currItem = rand();
		QueueEnqueue(testQueue, &currItem);
	}

	for(int i = 0; i < nItems; i++) {
	
		int currItemAddr; 
		QueueDequeue(testQueue, &currItemAddr);

		printf("Thread #%d dequeued %d.\n", id, currItemAddr);
		randomSleep(0, 1);
	}

	return NULL;
}

void testThreads() {

	Queue testQueue;
	pthread_t threadID[N_TEST_THREADS];
	ThreadArgs args[N_TEST_THREADS];

	QueueInit(&testQueue, sizeof(int), NULL, 1);

	// Start threads
	for(int i = 0; i < N_TEST_THREADS; i++) {
		
		args[i].threadID = i;
		args[i].testQueue = &testQueue;

		pthread_create(threadID + i, NULL, testThread, args + i);
	}

	// Wait for threads to finish
	for(int i = 0; i < N_TEST_THREADS; i++) pthread_join(threadID[i], NULL);

	if(!QueueIsEmpty(&testQueue)) {

		printf("Failure, queue is not empty after all threads have exited.\n");
		testSuccess = 0;
	}

	QueueDispose(&testQueue);
}

int main() {

	intTest();
	structTest();
	stringTest();
	testThreads();

	if(testSuccess) printf("Congrats, all tests passed.\n");

	return 0;
}