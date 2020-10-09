// cli.c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>

#define SERV_IP "127.0.0.1"
#define SERV_PORT1	10000
#define SERV_PORT2	9999 
#define BSIZE 1024

void *monitor(void *arg)
{
	int fd, ret, n;
	unsigned short port = *(unsigned short*)arg;
	struct sockaddr_in addr;
	fd_set rfds, orfds;
    char buf[BSIZE];

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd < 0){
		printf("socket error\n");
		exit(-1);
	}
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, SERV_IP, &addr.sin_addr.s_addr);
	addr.sin_port = htons(port);
	ret = connect(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr));
	if(ret < 0){
		printf("connect error\n");
		exit(-1);
	}
	
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	orfds = rfds;
	while(1){
		rfds = orfds;
		n = select(fd+1, &rfds, NULL, NULL, 0);
		if(n < 0){
			if(errno == EINTR){
				continue;
			}
			printf("select error\n");
			exit(-1);
		} else {
			if(FD_ISSET(fd, &rfds)){
				ret = read(fd, buf, sizeof(buf));
				if( ret > 0){
					if(buf[ret-1] != '\n'){
						buf[ret] = '\n';
					}
					fprintf(stdout, "[port:%d] %d:%s", port, ret, buf);
					fflush(stdout);
				} else if(ret == 0){ // network disconneted
					fprintf(stderr, "disconnect \n");
					break;
				} else {
					if(errno == EINTR){
						continue;
					}
					fprintf(stderr, "disconnect \n");
					break;
				}
			}
		}
	}
	close(fd);
	return NULL;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	pthread_t tid[2];
	unsigned short port[2] = {SERV_PORT1, SERV_PORT2};
	int i;

	for(i=0; i<2; i++){
		ret = pthread_create(&tid[i], NULL, monitor, &port[i]);
		if (ret != 0){
			printf("pthread_create %dth failed\n", i);
			exit(-1);
		}
	}

	for(i=0; i<2; i++){
		ret = pthread_join(tid[i], NULL);
		if(ret != 0){
			printf("pthread_join %dth failed\n", i);
			exit(-1);
		}
	}
	return 0;
}
