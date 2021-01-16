#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include "../Headers/ThreadPool.h"
#include "../Headers/Channel.h"
#include "../Headers/Future.h"

#define MAX_SLEEP 3

int generatedNumber = 0;
sem_t numberAvailable;

int cancelSuccess = 1;

int verboseOutput = 0;

int randomInt(int from, int to) {
	return from + rand() % (to - from + 1);
}

void randomSleep(int from, int to) {
	sleep(randomInt(from, to));
}

void *sendNumber(void *channelPtr) {

	Channel *channel;
	if(channelPtr != NULL) channel = (Channel *) channelPtr;

	randomSleep(0, MAX_SLEEP);
	generatedNumber = rand();

	if(channelPtr != NULL) ChannelSend(channel, &generatedNumber);
	else sem_post(&numberAvailable);

	return NULL;
}

void *getNumber(void *channelPtr) {

	Channel *channel;
	if(channelPtr != NULL) channel = (Channel *) channelPtr;

	randomSleep(0, MAX_SLEEP);
	int *number = malloc(sizeof(int));

	if(channelPtr == NULL) {
	
		sem_wait(&numberAvailable);
		*number = generatedNumber;

	} else ChannelGet(channel, number);

	return number;
}

void *timeoutRoutine(void *unused) {

	int *waitTime = malloc(sizeof(int));
	*waitTime = randomInt(0, MAX_SLEEP);
	
	if(verboseOutput) printf("Timeout routine sleep for: %d.\n", *waitTime);
	sleep(*waitTime);

	return waitTime;
}

void *cancelRoutine(void *delayPtr) {

	if(verboseOutput) printf("This should be printed.\n");
	sleep(*(int *) delayPtr);
	
	cancelSuccess = 0;
	if(verboseOutput) printf("This shouldn't");

	return NULL;
}

void testFutureGet(int withChannel) {

	Channel numberChannel;
	ChannelInit(&numberChannel, sizeof(int), NULL, 0);

	Future futureGetNumber, futureSendNumber;
	int *numberPtr;
	
	FutureInit(&futureSendNumber, sendNumber, withChannel ? &numberChannel : NULL);
	FutureInit(&futureGetNumber, getNumber, withChannel ? &numberChannel : NULL);

	FutureGet(&futureGetNumber, (void **) &numberPtr, 0);
	
	if(*numberPtr == generatedNumber) printf("Success. Retreived number is correct.\n");
	else printf("Failure. Number was incorrectly retreived.\n");

	if(FutureIsDone(&futureGetNumber) && FutureIsDone(&futureSendNumber)) printf("Success. Both futures have executed.\n");
	else printf("Failure. Both futures should be done by now.\n");

	free(numberPtr);

	FutureDispose(&futureGetNumber);
	FutureDispose(&futureSendNumber);

	ChannelDispose(&numberChannel);
}

void testFutureGetTimeout() {

	Future timeout;
	FutureInit(&timeout, timeoutRoutine, NULL);

	int *waitTime;
	int nWaits = 0;

	while(!FutureIsDone(&timeout)) {

		if(FutureGet(&timeout, (void **) &waitTime, 1)) {
		
			if(verboseOutput) printf("Future says computation ain't ready.\n");
			nWaits++;
		}
		else {

			if(verboseOutput) printf("Waits performed: %d.\n", nWaits);
			if(verboseOutput) printf("Future says wait time was: %d.\n", *waitTime);

			if(*waitTime == nWaits + 1) printf("Success. Wait time seems to be correct.\n");
			else printf("Failure. wait time may be incorrect.\n");
		}
	}

	free(waitTime);
	FutureDispose(&timeout);
}

void testFutureCancel() {
	
	Future cancelTest;
	int executionTime = 2;

	FutureInit(&cancelTest, cancelRoutine, &executionTime);
	
	sleep(executionTime/2);

	if(!FutureIsDone(&cancelTest)) FutureCancel(&cancelTest);

	sleep(1);

	if(FutureIsCancelled(&cancelTest) && cancelSuccess) printf("Success. Future is canceled.\n");
	else printf("Failure. Future is not canceled.\n");

	FutureDispose(&cancelTest);
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

	sem_init(&numberAvailable, 0, 0);

	// Testing FutureGet()
	testFutureGet(0);
	testFutureGet(1);

	// Testing FutureGet() with timeout
	testFutureGetTimeout();

	// Testing FutureCancel()
	testFutureCancel();

	return 0;
}