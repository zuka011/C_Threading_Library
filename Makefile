HEADER_DIR = ./Headers
SOURCE_DIR = ./Source
TESTS_DIR = ./Tests

CC = gcc
CFLAGS = -lpthread -g -Wall 

QUEUE_SRC = $(SOURCE_DIR)/Queue.c
QUEUE_DPS = $(QUEUE_SRC) $(HEADER_DIR)/Queue.h 

THREAD_POOL_SRC = $(QUEUE_SRC) $(SOURCE_DIR)/ThreadPool.c
THREAD_POOL_DEPS = $(QUEUE_DEPS) $(SOURCE_DIR)/ThreadPool.c $(HEADER_DIR)/ThreadPool.h

CHANNEL_SRC = $(THREAD_POOL_SRC) $(SOURCE_DIR)/Channel.c
CHANNEL_DEPS = $(THREAD_POOL_DEPS) $(SOURCE_DIR)/Channel.c $(HEADER_DIR)/Channel.h

FUTURE_SRC = $(CHANNEL_SRC) $(SOURCE_DIR)/Future.c
FUTURE_DEPS = $(CHANNEL_DEPS) $(SOURCE_DIR)/Future.c $(HEADER_DIR)/Future.h

QueueTest: $(QUEUE_DEPS) $(TESTS_DIR)/QueueTest.c
	$(CC) -o $@ $(QUEUE_SRC) $(TESTS_DIR)/$@.c $(CFLAGS)

ThreadPoolTest: $(THREAD_POOL_DEPS) $(TESTS_DIR)/ThreadPoolTest.c
	$(CC) -o $@ $(THREAD_POOL_SRC) $(TESTS_DIR)/$@.c $(CFLAGS)

ChannelTest: $(CHANNEL_DEPS) $(TESTS_DIR)/ChannelTest.c 
	$(CC) -o $@ $(CHANNEL_SRC) $(TESTS_DIR)/$@.c $(CFLAGS)

FutureTest: $(FUTURE_DEPS) $(TESTS_DIR)/FutureTest.c
	$(CC) -o $@ $(FUTURE_SRC) $(TESTS_DIR)/$@.c $(CFLAGS)

All: QueueTest ThreadPoolTest ChannelTest FutureTest

clean:
	rm *Test
	rm *.txt