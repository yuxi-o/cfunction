// cli.c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <signal.h>

#define SERV_IP "127.0.0.1"
#define SERV_PORT 10000
#define BSIZE 1024

int setnonblock(int fd)
{
	int opt, ret;

	opt = fcntl(fd, F_GETFL);
	assert(opt>0);

	ret = fcntl(fd, F_SETFL, opt | O_NONBLOCK);
	assert(ret==0);

	return 0;
}

int main(int argc, char *argv[])
{
    int fd; 
    int ret = 0;
    struct sockaddr_in saddr;
    
    fd = socket(AF_INET, SOCK_STREAM, 0); 
    if(-1 == fd){
        fprintf(stderr, "socket error\n");
        exit(EXIT_FAILURE);
    }   
    memset(&saddr, 0, sizeof(struct sockaddr_in));
    saddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERV_IP, &saddr.sin_addr.s_addr);
    saddr.sin_port = htons(SERV_PORT);
    ret = connect(fd, (struct sockaddr *)&saddr, sizeof(struct sockaddr));
    if( -1 == ret){
        fprintf(stderr, "connect error\n");
        exit(EXIT_FAILURE);
    }
	setnonblock(fd);

	unsigned char *p = malloc(10*90*1024*1024);
	assert(p);

	int i, j;
	int sum = 0, nleft = 10*90*1024*1024;
	for(j = 0; j < 9; j++){
		for(i = 0; i < 90; i++){
			memset(p+i*1024*1024, '!'+i, 1024*1024);
		}
	}

	signal(SIGPIPE, SIG_IGN);  // otherwise, write can't return -1, and errno=EPIPE
    while(nleft > 0){
//        ret = write(fd, p, 90*1024*1024);
        ret = write(fd, p, nleft);
        if( ret >= 0){
			sum += ret;
            fprintf(stdout, "----------------write %d bytes, sum %d bytes---------\n", ret, sum);
            fflush(stdout);
			nleft -= ret;
        } else {
			printf("fault: errno=%d\n", errno);
            if(errno == EINTR){
                continue;
            }
			if(errno == EAGAIN || errno == EWOULDBLOCK){
				fprintf(stdout, "----------------write sum %d bytes---------\n", sum);
				sleep(10);
			} else {
				break; // fault, close 
			}
        }

    }

    close(fd);

	fflush(stdout);
    return 0;
}
