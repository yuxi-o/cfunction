#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

pthread_mutex_t mmutex = PTHREAD_MUTEX_INITIALIZER;

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
	int i;
	signal(SIGUSR1, signal_handler);

	pthread_mutex_lock(&mmutex);
	for(i = 0; i < 5; i++){
		sleep(1);
		printf("main run [%d]!\n", i);
		if(i == 2){
			raise(SIGUSR1);
		}
	}
	pthread_mutex_unlock(&mmutex);

	pthread_mutex_destroy(&mmutex);

	printf("main exit [%d]!\n", i);
	return 0;
}

