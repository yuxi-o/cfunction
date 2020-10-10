#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_rwlock_t rwlock;

long int love;

void *pthread_write(void *arg)
{
	//int i = (int)arg;
	int i = *((int*)&arg);
	while(love <= 520)
	{
		pthread_rwlock_wrlock(&rwlock);
		printf("write -----love = %ld------[%d]\n", love+=40, i+1);
		pthread_rwlock_unlock(&rwlock);
		sleep(1);
	}

	return NULL;
}

void *pthread_read(void *arg)
{
	//int i = (int)arg;
	int i = *((int*)&arg);
	while(love <= 520)
	{
		pthread_rwlock_rdlock(&rwlock);
		printf("read -----love = %ld------[%d]\n", love, i+1);
		pthread_rwlock_unlock(&rwlock);
		sleep(1);
	}
	return NULL;
}

int main(void)
{
	pthread_t pthread[10];
	int i;

	pthread_rwlock_init(&rwlock, NULL);
	for (i = 0; i != 5; i++)
	{
		pthread_create(&pthread[i], NULL, pthread_write, (void *)(long)i);
	}
	for (i = 0; i != 5; i++)
	{
		pthread_create(&pthread[i+5], NULL, pthread_read, (void *)(long)i);
	}

	while(1)
	{
		if(love >=520)
		{
			for(int j=0; j!=10; j++)
			{
				pthread_cancel(pthread[j]);
				pthread_join(pthread[j], NULL);
			}
			break;
		}
	}
	
	return 0;
}
