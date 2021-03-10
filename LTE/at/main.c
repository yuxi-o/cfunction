#include <stdio.h>
#include <unistd.h>

#include "mat.h"

int main(int argc, char *argv[])
{
#if 0
	int ret = 0;
	mat_uart_t mat;

	ret = mat_uart_init(&mat, "/dev/ttyUSB1");
	if(ret < 0){
		printf("init /dev/ttyUSB1 error\n");
		return -1;
	}

	while(1){
		ret = mat_uart_get_signal(&mat);
		mat_uart_show_sim(&mat);

		sleep(2);
	}

	mat_uart_deinit(&mat);
#endif

	int ret = 0,  signal = 0;
	char buf[64];

	while(1){
		ret = mat_uart_get_sim("/dev/ttyUSB1", &signal, buf, sizeof(buf));
		printf("signal: %d, operator: %s\n", signal, buf);
		sleep(2);
	}

	return 0;
}

