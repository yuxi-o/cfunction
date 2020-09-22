#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_SIZE 10

int main()
{
	int fd, nfds;
	int i, ret;
	fd_set rfds, wfds;
	char buff[MAX_SIZE];

	fd = open("text", O_RDWR);
	if (fd < 0){
		perror("open");
		exit(1);
	}

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_SET(fd, &rfds);
	FD_SET(fd, &wfds);
	FD_SET(STDIN_FILENO, &rfds);
    while (1) {
		nfds = select(fd+1, &rfds, &wfds, NULL, NULL);
		if (nfds < 0){
			if(errno == EINTR){
				continue;
			}
			perror("select");
			break;
		}
		else {
//            if (FD_ISSET(fd, &rfds)) {
            if (FD_ISSET(STDIN_FILENO, &rfds)) {
				memset(buff, 0, sizeof(buff));
//				ret = read(fd, buff, sizeof(buff));
				ret = read(STDIN_FILENO, buff, sizeof(buff));
				if (ret > 0){
					printf("stdin:[%d:%s]\n", ret, buff);
				} else {
					if (ret == 0){
						printf("client close\n");
					}
				}
            }
            if (FD_ISSET(fd, &rfds)) {
				memset(buff, 0, sizeof(buff));
				ret = read(fd, buff, sizeof(buff));
				if (ret > 0){
					printf("read:[%d:%s]\n", ret, buff);
				} else {
					if (ret == 0){
						printf("read 0\n");
						FD_CLR(fd, &rfds);
						FD_SET(fd, &wfds);
					}
					perror("select read");
//					close(fd);
				}
            }
            if (FD_ISSET(fd, &wfds)) {
				ret = write(fd, "HELLOWORLD", 11);
				if (ret > 0){
					printf("write[%d:%s]\n", ret, "HELLOWORLD");
					FD_CLR(fd, &wfds);
					FD_SET(fd, &rfds);
				} else {
					if (ret == 0){
						printf("write 0\n");
						continue;
					}
					perror("select write");
//					close(fd);
				}
            }
		}
    }

	close(fd);
}
