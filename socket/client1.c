// cli.c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERV_IP "39.96.93.59"
#define SERV_PORT 10000
#define BSIZE 1024

int main(int argc, char *argv[])
{
    int fd; 
    int ret = 0;
    struct sockaddr_in saddr;
    char buf[BSIZE];
    
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

    while(1){
        memset(buf, 0, sizeof(buf));
        ret = read(fd, buf, sizeof(buf));
        if( ret > 0){
            if(buf[ret-1] != '\n'){
                buf[ret] = '\n';
            }
            fprintf(stdout, "%d:%s", ret, buf);
            fflush(stdout);
        } else if(ret == 0){ // network disconneted
            fprintf(stderr, "disconnect \n");
            break;
        } else {
            if(errno == EINTR){
                continue;
            }
        }

    }

    close(fd);
    return 0;
}
