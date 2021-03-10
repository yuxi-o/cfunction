#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>

#include "muart.h"

int speed_arr[] = {B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300};
int name_arr[] = {115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300};

static int set_speed(int fd, int speed)
{
	int   i;
	int   status;
	struct termios   Opt;
	tcgetattr(fd, &Opt);
	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) {
		if  (speed == name_arr[i]) {
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, speed_arr[i]);
			cfsetospeed(&Opt, speed_arr[i]);
			status = tcsetattr(fd, TCSANOW, &Opt);
			if  (status != 0) {
				printf("UART: tcsetattr fd error: %s", strerror(errno));
				return -1;
			}
			tcflush(fd,TCIOFLUSH);
		}
	}

	return 0;
}

static int set_Parity(int fd,int databits,int stopbits,int parity)
{
	struct termios options;
	if  (tcgetattr(fd, &options)  !=  0) {
		perror("SetupSerial 1");
		printf("UART: tcgetattr fd error: %s", strerror(errno));
		return -1;
	}
	options.c_cflag &= ~CSIZE;
	switch (databits)
	{
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			printf("Unsupported data size"); 
			return -1;
	}
	switch (parity)
	{
		case 'n':
		case 'N':
			options.c_cflag &= ~PARENB;   /* Clear parity enable */
			options.c_iflag &= ~INPCK;     /* Enable parity checking */
			break;
		case 'o':
		case 'O':
			options.c_cflag |= (PARODD | PARENB);
			options.c_iflag |= INPCK;             /* Disnable parity checking */
			break;
		case 'e':
		case 'E':
			options.c_cflag |= PARENB;     /* Enable parity */
			options.c_cflag &= ~PARODD;
			options.c_iflag |= INPCK;       /* Disnable parity checking */
			break;
		case 'S':
		case 's':  /*as no parity*/
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;break;
		default:
			printf("Unsupported parity");
			return -1;
	}
	switch (stopbits)
	{
		case 1:
			options.c_cflag &= ~CSTOPB;
			break;
		case 2:
			options.c_cflag |= CSTOPB;
			break;
		default:
			printf("Unsupported stop bits");
			return -1;
	}
	/* Set input parity option */
	if (parity != 'n')
		options.c_iflag |= INPCK;
	tcflush(fd,TCIFLUSH);
	options.c_cc[VTIME] = 0;
	options.c_cc[VMIN] = 1; /* Update the options and do it NOW */
	if (tcsetattr(fd,TCSANOW,&options) != 0)
	{
		printf("UART: tcsetattr fd error: %s", strerror(errno));
		return -1;
	}

	return 0;
}

static int tty_raw(int fd, struct termios *conf)
{
	int err;
	struct termios buf;

	if(tcgetattr(fd, &buf) < 0){
		printf("UART: tcgetattr fd error: %s", strerror(errno));
		return -1;
	}
	*conf = buf;

	buf.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

	buf.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

	buf.c_cflag &= ~(CSIZE | PARENB);

	buf.c_cflag |= CS8;

	buf.c_oflag &= ~(OPOST);
	buf.c_cc[VMIN] = 1;
	buf.c_cc[VTIME] = 0;
	if(tcsetattr(fd, TCSAFLUSH, &buf)<0){
		printf("UART: tcsetattr fd error: %s", strerror(errno));
		return -1;
	}

	if(tcgetattr(fd, &buf)<0){
		err = errno;
		tcsetattr(fd, TCSAFLUSH, conf);
		printf("UART: tcgetattr fd error: %s", strerror(errno));
		errno = err;
		return -1;
	}

	if((buf.c_lflag & (ECHO | ICANON | IEXTEN | ISIG)) || (buf.c_iflag & (BRKINT | ICRNL | INPCK | ISTRIP | IXON )) || (buf.c_cflag & ( CSIZE | PARENB | CS8)) != CS8 || (buf.c_oflag & OPOST) || buf.c_cc[VMIN]!=1 || buf.c_cc[VTIME]!=0){
		tcsetattr(fd, TCSAFLUSH, conf);
		printf("UART: tcgetattr fd error: %s", strerror(errno));
		errno = EINVAL;
		return -1;
	}

	return 0;
}

int muart_open(muart_t *m, const char *dev)
{
	if(NULL == m){
		return -1;
	}

	int fd = 0;

	fd = open(dev, O_RDWR | O_NONBLOCK);
	if(fd < 0){
		printf("UART: open %s error: %s", dev, strerror(errno));
		return -1;
	}

	if(tty_raw(fd, &m->old_conf) < 0){
		close(fd);
		return -1;
	}

	m->fd = fd;
	strncpy(m->dev, dev, sizeof(m->dev));
	m->write_max_bytes_once = WRITE_MAX_BYTES;

	return 0;
}

/**
 * @brief init uart 
 *
 * @param speed: baundrate, 300-115200
 * @param databits: data bits, 7,8
 * @param stopbits: stop bits, 1,2 
 * @param parity: check bit, o,e 
 */
int muart_setup(muart_t *m, int speed, char databits, char stopbits, char parity)
{
	int ret = 0;

	if(NULL == m){
		return -1;
	}

	ret = set_speed(m->fd, speed);
	if(ret < 0){
		return -1;
	}

	ret = set_Parity(m->fd, databits, stopbits, parity);
	if(ret < 0){
		return -1;
	}

	m->baudrate = speed;
	m->databits = databits;
	m->stopbits = stopbits;
	m->parity = parity;

	return 0;
}

ssize_t muart_read(muart_t *m, void *buf, size_t nbytes)
{
	return read(m->fd, buf, nbytes);
}

ssize_t muart_read_timeout(muart_t *m, void *buf, size_t nbytes, unsigned int timeout)
{
	int fd = m->fd;
	int nfds;
	fd_set readfds;
	struct timeval tv;

	if(timeout){
		tv.tv_sec = timeout;
		tv.tv_usec = 0;
		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);
		nfds = select(fd+1, &readfds, NULL, NULL, &tv);
		if(nfds <= 0){
			if(nfds == 0)
				errno = ETIME;
			return (-1);
		}
	}

	usleep(1000);

	return (read(fd, buf, nbytes));
}

/**
 * @brief read nbytes bytes as far as possible 
 * @description return when readed nbytes bytes or when there is no byte to be read in timeout seconds
 *
 * @param buf: the buffer to read to 
 * @param nbytes: the bytes number to read 
 * @param timeout: timeout second 
 */
ssize_t muart_read_nbytes(muart_t *m, void *buf, size_t nbytes, unsigned int timeout)
{
	size_t nleft;
	ssize_t nread=0;

	nleft = nbytes;
	while(nleft > 0){
		if((nread = muart_read_timeout(m, buf, nleft, timeout)) < 0){
			if (nleft == nbytes)
				return -1; 
			else
				break; /* error, return bytes to read so far*/
		} else if (nread == 0){
			break;  /* EOF */
		}
		nleft -= nread;
		buf += nread;
	}

	return (nbytes - nleft);
}

ssize_t muart_write(muart_t *m, void *buf, size_t nbytes)
{
	ssize_t nleft = nbytes, nwrite = 0, npos = 0, ret = 0;
	int max_write_once = m->write_max_bytes_once;

	do{
		if(nleft > max_write_once){
			nwrite = max_write_once; 
		} else {
			nwrite = nleft;
		}

		ret = write(m->fd, buf + npos, nwrite);
		if(ret < 0){
			if(nleft == nbytes){
				return -1;
			} else {
				break;
			}
		}

		nleft -= ret;
		npos += ret;
	} while(nleft > 0);

	return npos;
}

int muart_close(muart_t *m)
{
	if(!m){
		return -1;
	}

	tcflush(m->fd, TCIOFLUSH);
	tcsetattr(m->fd, TCSANOW, &m->old_conf);

	close(m->fd);
	return 0;
}
