#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct object{
	void *p;
	int len;
} object_t;

typedef struct animal{
	char *pdog;
	int len;
} animal_t;

object_t * object_new(void *p, int len)
{
	object_t *po = (object_t *)malloc(sizeof(object_t));
	if(po == NULL){
		return NULL;
	}

	po->p = (unsigned char *)malloc(len);
	if(po->p == NULL){
		free(po);
		return NULL;
	}
	memcpy(po->p, p, len);

	return po;
}

animal_t *animal_new(char *name, int len)
{
	return (animal_t*)(object_new(name, len));
}

int main(int argc, char *argv[])
{
	animal_t *p = animal_new("dog", 4);
	printf("%s\n", p->pdog);
		
	return 0;
}
