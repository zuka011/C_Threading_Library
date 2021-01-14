CC = gcc
CFLAGS = -lpthread -g -Wall 

QUEUE_SRC = Queue.c
QUEUE_DPS = $(QUEUE_SRC) Queue.h 

THREAD_POOL_SRC = $(QUEUE_SRC) ThreadPool.c
THREAD_POOL_DEPS = $(QUEUE_DEPS) ThreadPool.c ThreadPool.h

CHANNEL_SRC = $(THREAD_POOL_SRC) Channel.c
CHANNEL_DEPS = $(THREAD_POOL_DEPS) Channel.c Channel.h

FUTURE_SRC = $(CHANNEL_SRC) Future.c
FUTURE_DEPS = $(CHANNEL_DEPS) Future.c Future.h

QueueTest: $(QUEUE_DEPS) QueueTest.c
	$(CC) -o $@ $(QUEUE_SRC) $@.c $(CFLAGS)

ThreadPoolTest: $(THREAD_POOL_DEPS)  ThreadPoolTest.c
	$(CC) -o $@ $(THREAD_POOL_SRC) $@.c $(CFLAGS)

ChannelTest: $(CHANNEL_DEPS) ChannelTest.c 
	$(CC) -o $@ $(CHANNEL_SRC) $@.c $(CFLAGS)

FutureTest: $(FUTURE_DEPS) FutureTest.c
	$(CC) -o $@ $(FUTURE_SRC) $@.c $(CFLAGS)

All: QueueTest ThreadPoolTest ChannelTest FutureTest

clean:
	rm *Test
	rm *.txt