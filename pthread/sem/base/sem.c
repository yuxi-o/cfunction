#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#define NUM 5

int queue[NUM];

pthread_t pid, cid;
sem_t blank_number, product_number;

void *producer(void *arg)
{
	int i = 0;
	int ret = 0;
	while(1){
		// sem_wait/sem_post 不用判断返回值，因为sem_wait()会重启被信号打断后，在ubuntu下测试
		/*
		ret = sem_wait(&blank_number);
		if ((ret < 0) && (errno == EINTR)){
			printf("[blank]:sem_wait is interrupted by SIGINT");
		}
		*/
		sem_wait(&blank_number);
		queue[i] = rand() % 1000 + 1;
		printf("-----produce----%d----\n", queue[i]);
		sem_post(&product_number);
		
		i = (i + 1) % NUM;
		sleep(rand()%3);
	}
}

void *consumer(void *arg)
{
	int i = 0;
	int ret = 0;
	while(1){
		// sem_wait/sem_post 不用判断返回值，因为sem_wait()会重启被信号打断后，在ubuntu下测试
		/*
		ret = sem_wait(&product_number);
		if ((ret < 0) && (errno == EINTR)){
			printf("[product]:sem_wait is interrupted by SIGINT");
		}
		*/
		sem_wait(&product_number);
		printf("-----consumer----%d----\n", queue[i]);
		queue[i] = 0;
		sem_post(&blank_number);

		i = (i + 1) % NUM;
		sleep(rand()%3);
	}
}

void signal_int(int signo)
{
	printf("receive signo: %d\n", signo);	
	pthread_cancel(pid);
	pthread_cancel(cid);
}

int main(int argc, char *argv[])
{
	int ret = 0;
	struct sigaction sc, osc;
	sc.sa_handler = signal_int;
	sigemptyset(&sc.sa_mask);
	sc.sa_flags = 0;
	ret = sigaction(SIGINT, &sc, &osc);
	if (ret < 0){
		perror("sigaction error");
		exit(EXIT_FAILURE);
	}
	// SA_RESTART is set by system default. And sem_wait cat be intterrupted by SIGINT
	printf("Default sigaction.sa_flags is 0x%x, but SA_RESTART[0x%x] is set forever.\n", osc.sa_flags, SA_RESTART); // 0  0x10000000
	printf("Now sigaction.sa_flags is 0x%x.\n", sc.sa_flags); //0
	//signal(SIGINT, signal_int);
	
	sem_init(&blank_number, 0, NUM);
	sem_init(&product_number, 0, 0);

	pthread_create(&pid, NULL, producer, NULL);
	pthread_create(&cid, NULL, consumer, NULL);

	pthread_join(pid, NULL);
	pthread_join(cid, NULL);

	sem_destroy(&blank_number);
	sem_destroy(&product_number);
	return 0;
}
