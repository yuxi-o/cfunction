#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
	char *ptr = NULL;
	while(1){
		ptr = (char *)malloc(sizeof(char)*4 *1024*1024);
		//memset(ptr, 'a', sizeof(char) * 4 *1024 *1024);
		sleep(1);
	}

	return 0;
}


