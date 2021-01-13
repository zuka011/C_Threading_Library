QUEUE_SRC = Queue.c
QUEUE_DPS = $(QUEUE_SRC) Queue.h 

THREAD_POOL_SRC = $(QUEUE_SRC) ThreadPool.c
THREAD_POOL_DEPS = $(QUEUE_DEPS) ThreadPool.c ThreadPool.h

CHANNEL_SRC = $(THREAD_POOL_SRC) Channel.c
CHANNEL_DEPS = $(THREAD_POOL_DEPS) Channel.c Channel.h

FUTURE_SRC = $(CHANNEL_SRC) Future.c
FUTURE_DEPS = $(CHANNEL_DEPS) Future.c Future.h

QueueTest: $(QUEUE_DEPS) QueueTest.c
	gcc -g -Wall -o QueueTest $(QUEUE_SRC) QueueTest.c -lpthread

ThreadPoolTest: $(THREAD_POOL_DEPS)  ThreadPoolTest.c
	gcc -g -Wall -o ThreadPoolTest $(THREAD_POOL_SRC) ThreadPoolTest.c -lpthread

ChannelTest: $(CHANNEL_DEPS) ChannelTest.c 
	gcc -g -Wall -o ChannelTest $(CHANNEL_SRC) ChannelTest.c -lpthread

FutureTest: $(FUTURE_DEPS) FutureTest.c
	gcc -g -Wall -o FutureTest $(FUTURE_SRC) FutureTest.c -lpthread

clean:
	rm *Test