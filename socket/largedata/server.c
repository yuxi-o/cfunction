// ser.c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#define SERV_PORT 10000
#define MAX_CONN 2
#define STR_SND "Time: "

int main(int argc, char *argv[])
{
    int fd, sfd;
    int ret = 0;
    struct sockaddr_in saddr, cliaddr;
    socklen_t slen;
    
    fd = socket(AF_INET, SOCK_STREAM, 0); 
    if(-1 == fd){
        fprintf(stderr, "socket error\n");
        exit(EXIT_FAILURE);
    }   
	int opt = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&saddr, 0, sizeof(struct sockaddr_in));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(SERV_PORT);
    ret = bind(fd, (struct sockaddr *)&saddr, sizeof(struct sockaddr));
    if(-1 == ret){
        fprintf(stderr, "bind error\n");
        exit(EXIT_FAILURE);
    }

    ret = listen(fd, MAX_CONN);
    if( -1 == ret){
        fprintf(stderr, "listen error\n");
        exit(EXIT_FAILURE);
    }

	signal(SIGCHLD, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);

    while(1){
		sleep(10);
        slen = sizeof(struct sockaddr);
        sfd = accept(fd, (struct sockaddr*)&cliaddr, &slen);
        if(sfd < 0){
            // fprintf(stderr, "accept error\n");
			perror("accept()");
            if((errno == ECONNABORTED) || (errno == EINTR)){
				continue;
			} else {
				exit(EXIT_FAILURE);
			}
        }
        ret = fork();
        if(ret > 0){ // parent 
			printf("[%d:%d] listen...\n", getppid(), getpid());
			continue;
		} else if (ret == 0){ //child
            printf("connet:%s %d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
			sleep(10);
			int nbytes = 0, sum = 0;;
			char buf[1024];
            while(1){
				nbytes = read(sfd, buf, sizeof(buf)-1);
				if(nbytes > 0){
					buf[nbytes-1] = '\0';
					sum += nbytes;
					printf("---------------------recevied %d bytes, total: %d bytes----------------\n", nbytes, sum);
					printf("%s\n", buf);
				} else if(nbytes == 0){
					printf("---------------------read 0 byte, close fd----------\n");
					break;
				} else {
					if(errno == EINTR)
						continue;
					printf("---------------------fault, close fd----------\n");
					break;
				}
			}
			close(sfd);
        } else if(ret < 0) {
            perror("fork");
            fprintf(stderr, "fork error\n");
		}
    }

    close(fd);
    return 0;
}
