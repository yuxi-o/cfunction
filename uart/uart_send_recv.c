#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <poll.h>
#include <sys/select.h>
#include <stdarg.h>
#include <pthread.h>

#define DAPPNAME	"TESTUART"
#define DDUMMY		0
#define DRELEASE	1
#define DDEBUG		2

int gLOG_LEVEL = DDEBUG;	

enum error_num
{
	E_DUMMY = -1,
	E_NOFILE,
	E_WRITE,
	E_READ,
	E_TIMEOUT,
	E_FRAGMENT,
};

char *error_msg[] = 
{
	"open file or directory error", 

	"write error",
	"read error",

	"time out",
	"packet fragment",
};

// serial
#define BUF_SIZE	256	

static struct termios save_termios;
static int  ttysavefd = -1;
static enum {RESET , RAW , CBREAK} ttystate = RESET;

int speed_arr[] = { B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300};
int name_arr[] = {115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300};
unsigned char crc8(unsigned char arr[], int len)
{
	int i = 0; 
	unsigned char crc = 0;

	for(i = 0; i < len; i++){
		crc ^= arr[i];
	}

	return crc;
}

void set_speed(int fd, int speed)
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
                                perror("tcsetattr fd");
                                return;
                        }
                        tcflush(fd,TCIOFLUSH);
                }
        }

        return ;
}

int set_Parity(int fd,int databits,int stopbits,int parity)
{
	struct termios options;
	if  ( tcgetattr( fd,&options)  !=  0) {
		perror("SetupSerial 1");
		return 1;
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
			fprintf(stderr,"Unsupported data size\n"); return 1;
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
			fprintf(stderr,"Unsupported parity\n");
			return 1;
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
			fprintf(stderr,"Unsupported stop bits\n");
			return 1;
	}
	/* Set input parity option */
	if (parity != 'n')
		options.c_iflag |= INPCK;
	tcflush(fd,TCIFLUSH);
	options.c_cc[VTIME] = 0;
	options.c_cc[VMIN] = 1; /* Update the options and do it NOW */
	if (tcsetattr(fd,TCSANOW,&options) != 0)
	{
		perror("SetupSerial 3");
		return 1;
	}
	return 0;
}

int tty_raw(int fd)
{
        int err;
        struct termios buf;

        if(ttystate != RESET){
                errno = EINVAL;
                return -1;
        }
        if(tcgetattr(fd, &buf)<0)
                return -1;
        save_termios = buf;

        buf.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

        buf.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

        buf.c_cflag &= ~(CSIZE | PARENB);

        buf.c_cflag |= CS8;

        buf.c_oflag &= ~(OPOST);
        buf.c_cc[VMIN] = 1;
        buf.c_cc[VTIME] = 0;
        if(tcsetattr(fd, TCSAFLUSH, &buf)<0)
                return -1;

        if(tcgetattr(fd, &buf)<0){
                err = errno;
                tcsetattr(fd, TCSAFLUSH, &save_termios);
                errno = err;
                return -1;
        }

        if((buf.c_lflag & (ECHO | ICANON | IEXTEN | ISIG)) || (buf.c_iflag & (BRKINT | ICRNL | INPCK | ISTRIP | IXON )) || (buf.c_cflag & ( CSIZE | PARENB | CS8)) != CS8 || (buf.c_oflag & OPOST) || buf.c_cc[VMIN]!=1 || buf.c_cc[VTIME]!=0){
                tcsetattr(fd, TCSAFLUSH, &save_termios);
                errno = EINVAL;
                return -1;

        }

        ttystate = RAW;
        ttysavefd = fd;
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
int init_uart(const char *uart, int speed, int databits,int stopbits,int parity)
{
	int fd;
	
	if (NULL == uart){
		printf("FUNC:%s, uart file NULL!\n", __func__);
		return -1;
	}

	fd = open(uart, O_RDWR | O_NONBLOCK);
	if(-1 == fd){
		printf("FUNC:%s, open %s failed !\n", __func__, uart);
		return -1;
	}

	tty_raw(fd);
	close(fd);

	fd = open(uart, O_RDWR | O_NONBLOCK);
	if(-1 == fd){
		printf("FUNC:%s, open %s failed !\n", __func__, uart);
		return -1;
	}

	tty_raw(fd);

	set_speed(fd, speed);
	set_Parity(fd, databits, stopbits, parity);

	return fd;
}

/**
 * @brief printf the message
 *
 * @param err_num: wrong number
 */
void mprintf(int level, int err_num, const char *format, ...)
{
	va_list ap;

	if (level <= gLOG_LEVEL)
	{
		if (level == DDUMMY)
		{
			return;			
		}

		va_start(ap, format);
		printf("\n##[%s]: ", DAPPNAME);
		vprintf(format, ap);
		if (err_num >= 0)
		{
			printf(": %s\n", error_msg[err_num]);
		}
		va_end(ap);
	}
}

ssize_t dread(int fd, void *buf, size_t nbytes, unsigned int timeout)
{
	int nfds;
	fd_set readfds;
	struct timeval tv;

	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);
	nfds = select(fd+1, &readfds, NULL, NULL, &tv);
	if(nfds <= 0)
	{
		if(nfds == 0)
			errno = ETIME;
		return (-1);
	}

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
ssize_t dreadn(int fd, void *buf, size_t nbytes, unsigned int timeout)
{
	size_t nleft;
	ssize_t nread=0;

	nleft = nbytes;
	while(nleft > 0)
	{
		if((nread = dread(fd, buf, nleft, timeout)) < 0)
		{
			if (nleft == nbytes)
				return -1; 
			else
				break; /* error, return bytes to read so far*/
		} 
		else if (nread == 0)
		{
			break;  /* EOF */
		}
		nleft -= nread;
		buf += nread;
	}

	return (nbytes - nleft);
}

int uart_fd = 0;
char tx_buf[BUF_SIZE*2], rx_buf[BUF_SIZE*2];

void* send_routine(void *arg)
{
	int len = 11;
	strcpy(tx_buf, "HelloWorld");

	while(1){
		tcflush(uart_fd, TCOFLUSH);
		write(uart_fd, tx_buf, len);
		usleep(100000);
	}
	return NULL;
}
void* recv_routine(void *arg)
{
	int len = 0;
	while(1){
//		tcflush(uart_fd, TCIFLUSH);
		len = dreadn(uart_fd, rx_buf, 22, 1);
		printf("recv [%d] bytes: [%s]\n", len, rx_buf);
	}
	return NULL;
}

// socat -d -d pty,raw,echo=0 pty,raw,echo=0
// 生成两个虚拟串口，双向收发测试
int main(int argc, char *argv[])
{
	pthread_t tid_send, tid_recv;

//	uart_fd = init_uart("/dev/pts/3", 115200, 8, 1, 'n');
	uart_fd = init_uart(argv[1], 115200, 8, 1, 'n');
	if (uart_fd < 0){
		printf("FUNC:%s, init uart %s failed !\n", __func__, argv[1]);
		exit(-1);
	}
	if ((0 != pthread_create(&tid_send, NULL, send_routine, NULL))
			|| (0 != pthread_create(&tid_recv, NULL, recv_routine, NULL))){
		printf("pthread_create fail!\n");
		exit(-1);
	}

	pthread_join(tid_send, NULL);
	pthread_join(tid_recv, NULL);

	close(uart_fd);
	return 0;
}

