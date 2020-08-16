#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#define SER_IP "39.96.93.59"
#define PORT  10000 

int main()
{
	char buf[10];
	

	int sockfd,ret;
	struct sockaddr_in servaddr;
	
	int keepalive = 1;
	int keep_intvl = 5;//间隔
	int keep_probes = 3;//次数
	int keep_idletime = 5;//空闲

	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0)
	{
		printf("socket open error\n");
		return -1;
	}

	setsockopt(sockfd,SOL_SOCKET,SO_KEEPALIVE,&keepalive,sizeof(keepalive));
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&keepalive,sizeof(keepalive));
	setsockopt(sockfd,SOL_TCP,TCP_KEEPCNT,&keep_probes,sizeof(int));
	setsockopt(sockfd,SOL_TCP,TCP_KEEPINTVL,&keep_intvl,sizeof(int));
	setsockopt(sockfd,SOL_TCP,TCP_KEEPIDLE,&keep_idletime,sizeof(int));

	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(SER_IP);
	servaddr.sin_port = htons(PORT);

	ret = connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	if(ret < 0)
	{
		printf("connect error\n");
		printf("%d\n",errno);
		perror("errno");
		close(sockfd);
		return 0;
	}

	fcntl(sockfd,F_SETFL,O_NONBLOCK|fcntl(sockfd,F_GETFL));


	memset(buf,0,10);
	strcpy(buf,"adfadf");

	while(1){
	ret = write(sockfd,buf,10);
	if(ret < 0)
	{
		if(errno == EINTR)
		{
			continue;
		}
		printf("write wrong...\n");

	}
	if(ret < 5)
		printf("ret = %d...\n",ret);
	else
		printf("write ret = %d...\n",ret);
		
		
		sleep(1);//修改睡眠时间，测试keepalive空闲x秒启动

	}
	return 0;
	
}
