#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

pthread_mutex_t mmutex = PTHREAD_MUTEX_INITIALIZER;

void *thread_run(void *p)
{
	printf("thread...\n");
	pthread_mutex_lock(&mmutex);
	int i;
	for(i = 0; i < 10; i++){
		sleep(1);
		printf("thread run [%d]!\n", i);
	}
	pthread_mutex_unlock(&mmutex);

	return NULL;
}

void signal_handler(int signo)
{
	printf("signal...\n");

	pthread_mutex_lock(&mmutex);
	int i;
	for(i = 0; i < 5; i++){
		sleep(1);
		printf("signal run [%d]!\n", i);
	}
	pthread_mutex_unlock(&mmutex);
}


int main()
{
	signal(SIGUSR1, signal_handler);
	pthread_t p;
//	pthread_create(&p, NULL, thread_run, NULL);
	sleep(2);
	raise(SIGUSR1);
	pthread_create(&p, NULL, thread_run, NULL);
	//pthread_kill(p, SIGUSR1);

	while(1){
		printf("main...\n");
		sleep(1);
	}

	pthread_join(p, NULL);
	pthread_mutex_destroy(&mmutex);

	return 0;
}

