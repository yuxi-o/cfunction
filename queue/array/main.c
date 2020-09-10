#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

int main(int argc, char *argv[])
{
	squeue_t sq;
	squeue_data_t data, data1;

	squeue_init(&sq, 10);
	squeue_enqueue_ext(&sq, "LOVEM", 6);
	squeue_enqueue_ext(&sq, "LOVEN", 6);
	
	data.length = 10;  
	data.pdata = (char *)malloc(5);
	if(data.pdata == NULL)
	{
		printf("malloc error");
		return -1;
	}
	memcpy(data.pdata, "LOVEA", 5);
	squeue_enqueue(&sq, data);

	memcpy(data.pdata, "LOVEB", 5);
	squeue_enqueue(&sq, data);

	memcpy(data.pdata, "LOVEC", 5);
	squeue_enqueue(&sq, data);

	memcpy(data.pdata, "LOVED", 5);
	squeue_enqueue(&sq, data);

	squeue_enqueue_ext(&sq, "LOVEE", 6);
	squeue_info(&sq);

	while(squeue_dequeue(&sq, &data1)== 0)
	{
		printf("[%d]:%s\n", data1.length, (char *)data1.pdata);
	}

//	squeue_destroy(&sq, squeue_data_destroy);
	return 0;
}

