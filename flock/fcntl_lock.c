#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/file.h>

int lockfile(int fd)
{
#if 0
	struct flock fl;

	fl.l_type = F_WRLCK; // write lock
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;  // lock the whole file

	return(fcntl(fd, F_SETLK, &fl));
#else
	return(flock(fd, LOCK_EX | LOCK_NB));
#endif
}

// single process check
int process_already_running(const char *filename)
{
	int fd;
	char buf[16];

	fd = open(filename, O_RDWR | O_CREAT, 0644);
	if(fd < 0){
		printf("can't open %s \n", filename);
		return -1;
	}

	if(lockfile(fd) < 0){
		//if(errno == EACCES || errno == EAGAIN){
		if(errno == EWOULDBLOCK){
			printf("file: %s already locked\n", filename);
			close(fd);
			return -1;
		}
		printf("can't lock %s\n", filename);
		return -1;
	}
	
	sprintf(buf, "%ld\n", (long)getpid());
	write(fd, buf, strlen(buf)+1);

	return 0;
}

int main(int argc, char *argv[])
{
	int ret = 0;

	if(argc != 2){
		printf("Usage: %s lock_filename\n", argv[0]);
		exit(-1);
	}

	ret = process_already_running(argv[1]);
	if(ret < 0){
		return -1;
	}
	while(1){
		sleep(3);
		printf("lock file %s\n", argv[1]);
	}

	return 0;
}

