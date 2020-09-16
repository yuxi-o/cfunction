#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "lqueue.h" 

#define LOOP_COUNT 10

struct {
	char *pdata;
	int length;
} array[] = {
	{"LOVEA", 6},
	{"LOVEB", 6},
	{"LOVEC", 6},
	{"LOVED", 6},
	{"LOVEE", 6},
	{"LOVEF", 6},
	{"LOVEG", 6},
	{"LOVEH", 6},
	{"LOVEI", 6},
	{"LOVEJ", 6},
	{"LOVEK", 6},
	{"LOVEL", 6},
	{"LOVEM", 6},
	{"LOVEN", 6},
};

int main(int argc, char *argv[])
{
	lqueue_t lq;
	int index = 0;
	int ARRAY_SIZE = sizeof(array) /sizeof(array[0]);
	int loop = 0;
	char *pstr = NULL;
	int length = 0;
	
	printf("Array total is %d\n", ARRAY_SIZE);

	lqueue_init(&lq, 10);
	while(loop++ < LOOP_COUNT)
	{
		printf("---------------------loop %d------------------------\n", loop);
		while(!lqueue_is_full(&lq))
		{
			lqueue_enqueue_front_ext(&lq, array[index].pdata, array[index].length);
			index=(index + 1) % ARRAY_SIZE;
		}

		lqueue_info(&lq);

		while(!lqueue_is_empty(&lq))
		{
			lqueue_dequeue_back(&lq, (void **)&pstr, &length);
			printf("[%d]:%s\n", length, (char *)pstr);
			lqueue_data_destroy_ext(pstr);
		}

		lqueue_info(&lq);
		sleep(1);
	}

	lqueue_destroy(&lq, lqueue_data_destroy_ext);
	return 0;
}
