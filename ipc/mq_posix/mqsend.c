// refer to: https://www.cnblogs.com/Anker/archive/2013/01/04/2843832.html
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <errno.h>
#include <time.h>

int main(int argc, char *argv[])
{
	mqd_t mqd;
	int ret = 0;
	char cbuf[128] = {0};
	struct timespec ts = {1, 0};
	struct mq_attr attr;
	
	if(argc != 2){
		printf("Usage: %s mq_name\n", argv[0]);
		return -1;
	}

	mqd = mq_open(argv[1], O_WRONLY | O_CREAT, 0666, NULL);
	if(mqd < 0){
		printf("mq_open error: %s\n", strerror(errno));
		return -1;
	}

	ret = mq_getattr(mqd, &attr);
	if(ret == 0){
		printf("mq_attr: flags[0x%lx], maxmsg[%ld], msgsize[%ld], curmsgs[%ld]\n", attr.mq_flags, attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs);
	}

	while(1){
		sprintf(cbuf, "ts:%ld", time(NULL));
		ret = mq_timedsend(mqd, cbuf, strlen(cbuf)+1, 0, &ts);
		if(ret < 0){
			printf("mq_timedsend error: %s\n", strerror(errno));
		}
		printf("%s\n", cbuf);
		sleep(1);
	}

	mq_close(mqd);
	return 0;
}

