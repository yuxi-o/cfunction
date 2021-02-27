#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int total = 0;
void *thread_add(void *p)
{
	int i = 0;

	for(i = 0; i < 100000; i++){
		printf("%ld total=%d\n", pthread_self(), total++);
	}

	return NULL;
}

int main()
{

	pthread_t t1, t2;
	pthread_create(&t1, NULL, thread_add, NULL);
	pthread_create(&t2, NULL, thread_add, NULL);

//	sleep(10);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL); 
	printf("main total=%d\n", total);

	return 0;
}
