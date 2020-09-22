#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_SIZE 10

int main()
{
    int epfd, nfds, fd, ret;
    struct epoll_event event, events[5];
	char buff[MAX_SIZE];

    epfd = epoll_create(1);

	fd = open("text", O_RDWR);
	if (fd < 0){
		perror("open");
		exit(1);
	}

    event.data.fd = fd;
//    event.data.fd = STDIN_FILENO;
    event.events = EPOLLIN ;
//	event.events = EPOLLOUT; // not report event
//    epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &event);
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
    while (1) {
        nfds = epoll_wait(epfd, events, 5, -1);
        int i;
        for (i = 0; i < nfds; ++i) {
            if (events[i].data.fd == fd) {
				printf("monitor common fd event\n");
//            if (events[i].data.fd == STDIN_FILENO) {
				memset(buff, 0, sizeof(buff));
				ret = read(fd, buff, sizeof(buff));
//				ret = read(STDIN_FILENO, buff, sizeof(buff));
				if (ret > 0){
					printf("%d:%s\n", ret, buff);
				} else {
					if (ret == 0){
						printf("client close\n");
					}
					close(fd);
					break;
				}
            }
        }
    }

	close(fd);
	close(epfd);
}
