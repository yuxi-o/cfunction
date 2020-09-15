#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#define FILE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define NBUFF	10
#define SEM_MUTEX	"mutex"
#define SEM_NEMPTY	"nempty"
#define SEM_NSTORED	"nstored"

int nitems;

struct 
{
	int buff[NBUFF];
	sem_t *mutex, *nempty, *nstored;
} shared;

char *px_ipc_name(const char *name);
void *produce(void *arg);
void *consume(void *arg);

int main(int argc, char *argv[])
{
	pthread_t tid_produce, tid_consume;
	if (argc != 2){
		printf("usage: products <#items>");
		exit(0);
	}
	nitems = atoi(argv[1]);

	if ((shared.mutex = sem_open(SEM_MUTEX, O_CREAT, FILE_MODE, 1))== SEM_FAILED){
		perror("sem_open() error");
		exit(-1);
	}
	if ((shared.nempty = sem_open(SEM_NEMPTY, O_CREAT, FILE_MODE, NBUFF))== SEM_FAILED){
		perror("sem_open() error");
		exit(-1);
	}
	if ((shared.nstored = sem_open(SEM_NSTORED, O_CREAT, FILE_MODE, 0))== SEM_FAILED){
		perror("sem_open() error");
		exit(-1);
	}
	pthread_setconcurrency(2);
	pthread_create(&tid_produce, NULL, produce, NULL);
	pthread_create(&tid_consume, NULL, consume, NULL);
	pthread_join(tid_produce, NULL);
	pthread_join(tid_consume, NULL);
	sem_unlink(SEM_MUTEX);
	sem_unlink(SEM_NEMPTY);
	sem_unlink(SEM_NSTORED);
	exit(0);
}

void *produce(void *arg)
{
	int i;
	printf("produce is called\n");
	for (i = 0; i <nitems; i++){
		sem_wait(shared.nempty);
		sem_wait(shared.mutex); // 互斥锁
		printf("produced a new item.[%d]\n", i);
		shared.buff[i%NBUFF] = i;
		sem_post(shared.mutex);
		sem_post(shared.nstored);
	}
	return NULL;
}

void *consume(void *arg)
{
	int i;
	printf("consume is called\n");
	for (i = 0; i <nitems; i++){
		sem_wait(shared.nstored);
		sem_wait(shared.mutex); // 互斥锁
		if (shared.buff[i % NBUFF] != i)
			printf("buff[%d] = %d\n", i, shared.buff[i%NBUFF]);
		printf("consume a item.[%d]\n", i);
		sem_post(shared.mutex);
		sem_post(shared.nempty);
	}
	return NULL;

}
