#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define MAX_BUFFER_SIZE		5
#define MAX_EPOLL_EVENTS	20	
#define EPOLL_LT	0
#define EPOLL_ET	1
#define FD_BLOCK	0
#define FD_NONBLOCK	1

int set_nonblock(int fd){
	int old_flags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, old_flags | O_NONBLOCK);
	return old_flags;
}

// 注册文件描述符到epoll，并设置其事件为EPOLLIN
void addfd_to_epoll(int epfd, int fd, int epoll_type, int block_type){
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = EPOLLIN;

	if (epoll_type == EPOLL_ET){
		ev.events |= EPOLLET;
	}

	if (block_type == FD_NONBLOCK){
		set_nonblock(fd);
	}

	epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
}

void epoll_lt(int sockfd){
	char buffer[MAX_BUFFER_SIZE];
	int ret;

	printf("-------------------LT recv...\n");
	memset(buffer, 0, MAX_BUFFER_SIZE);
	ret = recv(sockfd, buffer, MAX_BUFFER_SIZE-1, 0);
	printf("recv bytes: %d\n", ret);
	if (ret > 0){
/*		printf("recv bytes:");
		for(i = 0; i < MAX_BUFFER_SIZE; i++){
			printf("%2x ", buffer[i]);
		}
		printf("\n");
*/
		printf("recv bytes: %s\n", buffer);

	} else {
		if (ret == 0){
			printf("client close\n");
		}
		close(sockfd);
	}
	printf("LT deal with over\n");
}

void epoll_et_loop(int sockfd){
	char buffer[MAX_BUFFER_SIZE];
	int ret ;

	printf("-------------------ET loop recv...\n");
	while(1){
		memset(buffer, 0, MAX_BUFFER_SIZE);
		ret = recv(sockfd, buffer, MAX_BUFFER_SIZE-1, 0);
		printf("ET loop recv bytes: %d\n", ret);
		if (ret > 0){
			printf("ET loop recv %d bytes: %s\n", ret, buffer);
		} else if (ret < 0){
			if ((errno == EAGAIN) || errno == EWOULDBLOCK){
				printf("ET loop recv all data...\n");
				break;
			}
			close(sockfd);
			break;
		} else { //if (ret == 0){
			printf("client close\n");
			close(sockfd);
			break;
		}
	}
	printf("ET loop deal with over\n");
}

void epoll_et_nonloop(int sockfd){
	char buffer[MAX_BUFFER_SIZE];
	int ret ;

	printf("--------------------ET nonloop recv...\n");
	memset(buffer, 0, MAX_BUFFER_SIZE);
	ret = recv(sockfd, buffer, MAX_BUFFER_SIZE-1, 0);

	printf("ET nonloop recv bytes: %d\n", ret);
	if (ret > 0){
		printf("ET loop recv %d bytes: %s\n", ret, buffer);
	} else if (ret < 0){
		close(sockfd);
	} else { //if (ret == 0){
		printf("client close\n");
		close(sockfd);
	}

	printf("ET nonloop deal with over\n");
}

void epoll_process(int epfd, struct epoll_event *events, int number, 
		int sockfd, int epoll_type, int block_type)
{
	struct sockaddr_in client_addr;
	socklen_t client_addrlen;
	int newfd, confd;
	int i;

	for (i = 0; i <number; i++){
		newfd = events[i].data.fd;
		if (newfd == sockfd){
			printf("---------------accept()-------------\n");
			printf("sleep 3s...\n");
			sleep(3);
			printf("sleep 3s over\n");

			client_addrlen = sizeof(client_addr);
			confd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addrlen);
			printf("confd= %d\n", confd);
			addfd_to_epoll(epfd, confd, epoll_type, block_type);
			printf("accept() over!!!\n");
		} else if (events[i].events & EPOLLIN){
			if (epoll_type == EPOLL_LT){
				epoll_lt(newfd);
			} else if (epoll_type == EPOLL_ET){
				epoll_et_loop(newfd);
				//epoll_et_nonloop(newfd);
			}
		} else {
			printf("other events...\n");
		}
	}
}

void err_exit(char *msg){
	perror(msg);
	exit(1);
}

int create_socket(const char *ip, const int portnumber){
	struct sockaddr_in server_addr;
	int sockfd , reuse = 1;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(portnumber);

	if(inet_pton(PF_INET, ip, &server_addr.sin_addr)== -1){
		err_exit("inet_pton() error");
	}

	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1){
		err_exit("socket() error");
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1){
		err_exit("setsockopt() error");
	}

	if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
		err_exit("bind() error");
	}

	if (listen(sockfd, 5) == -1){
		err_exit("listen() error");
	}

	return sockfd;
}

int main(int argc, char *argv[])
{
	if (argc < 3){
		fprintf(stderr, "usage: %s ip_address port_number\n", argv[0]);
		exit(1);
	}

	int sockfd, epfd, number;

	sockfd = create_socket(argv[1], atoi(argv[2]));
	struct epoll_event events[MAX_EPOLL_EVENTS];

	if ((epfd = epoll_create1(0)) == -1){
		err_exit("epoll_create() error");
	}
	
	// 以下设置针对监听的sockfd，当epoll_wait返回时，必定有事件发生
	// 所以这里忽略罕见的情况外，设置阻塞IO没有意义，我们设置非阻塞IO
	//
	// 水平触发 非阻塞
	addfd_to_epoll(epfd, sockfd, EPOLL_LT, FD_NONBLOCK);

	// 边缘触发 非阻塞
	//addfd_to_epoll(epfd, sockfd, EPOLL_ET, FD_NONBLOCK);
	
	while(1){
		number = epoll_wait(epfd, events, MAX_EPOLL_EVENTS, -1);
		if (number == -1){
			err_exit("epoll_wait() error");
		} else{
			// 水平触发 阻塞
			//epoll_process(epfd, events, number, sockfd, EPOLL_LT, FD_BLOCK);
			// 水平触发 非阻塞
			//epoll_process(epfd, events, number, sockfd, EPOLL_LT, FD_NONBLOCK);
			// 边缘触发 阻塞
			//epoll_process(epfd, events, number, sockfd, EPOLL_ET, FD_BLOCK);
			// 边缘触发 非阻塞
			epoll_process(epfd, events, number, sockfd, EPOLL_ET, FD_NONBLOCK);
		}

	}

	close(sockfd);
	return 0;
}
