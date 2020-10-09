#include <stdio.h>
#include <assert.h>

int main()
{
	int flag = -5;
	while(flag < 10){
		printf("flag == %d\n", flag);
		// assert后整个程序退出
		assert(flag);
		flag++;
	}

	printf("Program over\n");
	return 0;
}

// a.out: assert.c:10: main: Assertion `flag' failed.
