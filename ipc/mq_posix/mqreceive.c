// refer to: https://www.cnblogs.com/Anker/archive/2013/01/04/2843832.html
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <mqueue.h>

int main(int argc, char *argv[])
{
	mqd_t mqd;
	char cbuf[8192];
	unsigned int priority;
	int bytes;
	struct mq_attr attr;
	int ret = 0;

	if(argc < 2){
		printf("Usage: %s mq_name\n", argv[0]);
		return -1;
	}

	mqd = mq_open(argv[1], O_RDONLY | O_CREAT, 0666, NULL);
	if(mqd < 0){
		printf("mq_open error: %s\n", strerror(errno));
		return -1;
	}

	ret = mq_getattr(mqd, &attr);
	if(ret == 0){
		printf("mq_attr: flags[0x%lx], maxmsg[%ld], msgsize[%ld], curmsgs[%ld]\n", attr.mq_flags, attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs);
	}

	while(1){
		bytes = mq_receive(mqd, cbuf, sizeof(cbuf), &priority);
		if(bytes < 0){
			printf("mq_receive error: %s\n", strerror(errno));
		}
		cbuf[bytes] = 0;
		printf("mq receive %d bytes(%d):[%s]\n", bytes, priority, cbuf);

		ret = mq_getattr(mqd, &attr);
		if(ret == 0){
			printf("mq_attr: flags[0x%lx], maxmsg[%ld], msgsize[%ld], curmsgs[%ld]\n", attr.mq_flags, attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs);
		}
		sleep(1);
	}

	mq_close(mqd);
	return 0;
}

