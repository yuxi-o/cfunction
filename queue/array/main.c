#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

#define LOOP_COUNT 100


squeue_data_t array[] = {
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
	squeue_t sq;
	squeue_data_t data1;
	int index = 0;
	int ARRAY_SIZE = sizeof(array) /sizeof(array[0]);
	int loop = 0;

	
	printf("Array total is %d\n", ARRAY_SIZE);
	squeue_init(&sq, 10);
	while(loop++ < LOOP_COUNT)
	{
		printf("---------------------loop %d------------------------\n", loop);
		while(!squeue_is_full(&sq))
		{
			squeue_enqueue_ext(&sq, array[index].pdata, array[index].length);
			index=(index + 1) % ARRAY_SIZE;
		}

		squeue_info(&sq);

		while(!squeue_is_empty(&sq))
		{
			squeue_dequeue(&sq, &data1);
			printf("[%d]:%s\n", data1.length, (char *)data1.pdata);
			squeue_data_destroy(&data1);
		}

		squeue_info(&sq);
		sleep(1);
	}

	squeue_destroy(&sq, squeue_data_destroy);
	return 0;
}

