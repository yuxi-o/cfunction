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

	int len = strlen(STR_SND);
	char buf[64], buftime[64];
	strcpy(buf, STR_SND);
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
            while(1){
                //ret = write(sfd, STR_SND, sizeof(STR_SND));
                //ret = write(sfd, STR_SND, 100);
				sprintf(buftime, "%ld", time(NULL));
				strcpy(buf+len, buftime);
                ret = write(sfd, buf, strlen(buf));
                if(ret == -1){
                    if(errno == EPIPE){
                        close(sfd);
                        printf("[%d:%d] client [%s:%d] disconnect\n", getppid(), getpid(), inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
                        //exit(EXIT_SUCCESS);
						return -1;
                    }
                }
				printf("[%d:%d] send client [%s:%d](%ld): %s\n", getppid(), getpid(), inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), strlen(buf), buf);
                sleep(3);
            }
        } else if(ret < 0) {
            perror("fork");
            fprintf(stderr, "fork error\n");
		}
    }

    close(fd);
    return 0;
}
