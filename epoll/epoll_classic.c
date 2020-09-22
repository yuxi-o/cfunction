#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAXLINE 512
#define OPEN_MAX 100
#define LISTENQ 20
#define SERV_PORT 5000
#define INFTIM 1000

void setnonblocking(int sock)
{
    int opts;
    opts=fcntl(sock,F_GETFL);
    if(opts<0)
    {
        perror("fcntl(sock,GETFL)");
        exit(1);
    }
    opts = opts|O_NONBLOCK;
    if(fcntl(sock,F_SETFL,opts)<0)
    {
        perror("fcntl(sock,SETFL,opts)");
        exit(1);
    }
}

int main(int argc, char* argv[])
{
    int i, listenfd, connfd, sockfd,epfd,nfds, portnumber;
    ssize_t n, nbytes;
    char line[MAXLINE];
    socklen_t clilen;
	int ret = 0;


    if ( 2 == argc )
    {
        if( (portnumber = atoi(argv[1])) < 0 )
        {
            fprintf(stderr,"Usage:%s portnumber\n",argv[0]);
            return 1;
        }
    }
    else
    {
        fprintf(stderr,"Usage:%s portnumber\n",argv[0]);
        return 1;
    }

    //声明epoll_event结构体的变量,ev用于注册事件,数组用于回传要处理的事件
    struct epoll_event ev,events[20];

    //生成用于处理accept的epoll专用的文件描述符
    epfd=epoll_create(1);
    struct sockaddr_in clientaddr;
    struct sockaddr_in serveraddr;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    //把socket设置为非阻塞方式
    setnonblocking(listenfd);

    //设置与要处理的事件相关的文件描述符
    ev.data.fd=listenfd;
    //设置要处理的事件类型  水平触发非阻塞
    ev.events=EPOLLIN;

    //注册epoll事件
    epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);

    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    char *local_addr="127.0.0.1";
    inet_aton(local_addr,&(serveraddr.sin_addr));
    serveraddr.sin_port=htons(portnumber);

	int on = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
    bind(listenfd,(struct sockaddr *)&serveraddr, sizeof(serveraddr));
    listen(listenfd, LISTENQ);
    for ( ; ; ) {
        //等待epoll事件的发生
        nfds=epoll_wait(epfd, events, 20, 500);

        //处理所发生的所有事件
        for(i=0;i<nfds;++i)
        {
            if(events[i].data.fd==listenfd)//如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。
            {
				clilen = sizeof(clientaddr);
                connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clilen);
                if(connfd<0){
                    perror("connfd<0");
                    exit(1);
                }

                setnonblocking(connfd);
                char *str = inet_ntoa(clientaddr.sin_addr);
                printf("accapt a connection from %s\n", str);
                //设置用于读操作的文件描述符
                ev.data.fd=connfd;
                //设置用于注测的读操作事件, 边缘触发非阻塞
                ev.events=EPOLLIN|EPOLLET;

                //注册ev
                epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);
            }
            else if(events[i].events&EPOLLIN)//如果是已经连接的用户，并且收到数据，那么进行读入。
            {
                printf("EPOLLIN\n");
                if ( (sockfd = events[i].data.fd) < 0)
                    continue;

				nbytes = 0;
				memset(line, 0, MAXLINE);
				while(1){
					n = read(sockfd, line+nbytes, MAXLINE-1-nbytes);
					if (n > 0){
						printf("read(%ld): %s\n", n, line);
						nbytes += n;
					} else if( n < 0){
						if ((errno == EAGAIN) || errno == EWOULDBLOCK){
							printf("read over\n");

							//设置用于写操作的文件描述符
							ev.data.fd=sockfd;
							//设置用于注测的写操作事件
							ev.events=EPOLLOUT|EPOLLET;
							//修改sockfd上要处理的事件为EPOLLOUT
							epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
							break;
						}
						//} else if (errno == ECONNRESET) {
						close(sockfd);
						events[i].data.fd = -1;
						printf("readline error\n");
						break;
					} else { //if (n == 0) {
						close(sockfd);
						events[i].data.fd = -1;
						break;
					}
				}
            } 
			else if(events[i].events&EPOLLOUT) // 如果有数据发送
            {
				printf("EPOLLOUT\n");
                sockfd = events[i].data.fd; // 应该用struct管理fd和数据line
                ret = write(sockfd, line, nbytes);
				if(ret < 0){
					if(errno == EPIPE){
						close(sockfd);
						events[i].data.fd = -1;
						break;
					}
					perror("EPOLLOUT write");
				}
                //设置用于读操作的文件描述符
                ev.data.fd=sockfd;
                //设置用于注册读操作事件
                ev.events=EPOLLIN|EPOLLET;
                //修改sockfd上要处理的事件为EPOLIN
                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
            }
        }
    }

    return 0;
}
