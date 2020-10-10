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
	options.c_cc[VTIME] = 10;
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

int main(int argc, char *argv[])
{
	int fd = 0;
	char tx_buf[BUF_SIZE*2], rx_buf[BUF_SIZE*2];
	unsigned char tx_buf_tmp[BUF_SIZE];
	unsigned char *txp = NULL;
	int speed = 0, len = 0, retlen = 0;
	char pserial[3]  = {0}, flag_data = 0, flag_read = 0;
	int i = 0, j = 0;
	
	printf("Usage: %s /dev/ttyUSBx -s[115200|57600|9600|4800|2400] -p[8n1|7o2|8e1] -f[1|2]\n", argv[0]);
	printf("-s: specify the bps, 115200, 57600, 9600, 4800, 2400...\n");
	printf("-p: specify the databits, parity and stopbits.\
			\n\tdatabits: 8 or 7, parity: o, e, n, stopbits: 1 or 2\n");
	printf("-f: specify the following data format. string: 1, hex: 2.\n");

	if(5 != argc){
		printf("CMD error! Please repeat!\n");	
		exit(-1);
	}
	
	if(strncmp("-s", argv[2], 2) == 0){
		speed = atoi(&argv[2][2]);
	} else {
		exit(-1);
	}

	if(strncmp("-p", argv[3], 2) == 0){
		strncpy(pserial, &argv[3][2], 3);
	} else {
		exit(-1);
	}

	if(strncmp("-f", argv[4], 2) == 0){
		flag_data = argv[4][2] - '0';
	} else {
		exit(-1);
	}

	fd = open(argv[1], O_RDWR);
	if(fd == -1){
		perror("open ttyUSBx");
		exit(1);
	}
#if 1
	tty_raw(fd);

	set_speed(fd, speed);
	set_Parity(fd, pserial[0]-'0', pserial[2]-'0', pserial[1]);
#endif

	printf("====\nInput format [%d] data(1: string, 2: hex): \n", flag_data);
	while(fgets(tx_buf, sizeof(tx_buf), stdin)){
		len = strlen(tx_buf);

		txp = (unsigned char *)tx_buf;
		if(flag_data == 2){ // deal with hex
			i = 0, j = 0;
			while(isspace(tx_buf[i]) != 0) i++; // eat space
			for(; i < len; i++){
				while(isspace(tx_buf[i]) == 0) i++;
				if((isxdigit(tx_buf[i-2]) != 0) && (isxdigit(tx_buf[i-1]) != 0)){
//					tx_buf_tmp[j++] = (tx_buf[i-2] - '0')*16 + (tx_buf[i-1] - '0');
					tx_buf_tmp[j++] = (unsigned char)(strtol(tx_buf+(i-2), NULL, 16) & 0xFF);
				} else {
					break;
				}	
				while(isspace(tx_buf[++i]) != 0); // eat space
			}
			if((i < len) || (j == 0)){
				printf("Input error, please repeat:\n");
				continue;
			}
			txp = tx_buf_tmp;
			len = j;

			printf("Will send %d [hex] data:", len);
			for(j = 0; j < len; j++){
				if(j % 10 == 0)printf("\n");
				printf("%.2X ", tx_buf_tmp[j]&0xFF);
			}
			printf("\n");
		} else {
			if(tx_buf[len-1] == '\n')
				len--;
		}

		write(fd, txp, len);
		sync();
		len = 0;
		flag_read = 0;
		// first, recv at least one bytes, and continue receiving remaining bytes
		// exit util read no more bytes
		// options.c_cc[VTIME] = 10; // every 1s to read
		// options.c_cc[VMIN] = 1; // at least one bytes 
		do {
			retlen = read(fd, rx_buf+len, sizeof(rx_buf));
			if(retlen < 0){
				printf("recevice error, continue...\n");
				exit(-1);
			} else if(retlen > 0) {
				len += retlen;
				flag_read = 1;
			} else if ((flag_read >0) && (retlen == 0)){
				break;
			}	
		} while(1);

		if(flag_data == 2){ //deal with hex
			printf("receive %d [hex] data:", len);
			for(i = 0; i < len; i++){
				if(i % 10 == 0)printf("\n");
				printf("%.2X ", rx_buf[i]&0xFF);
			}
			printf("\n");
		} else {
			printf("receive %d [string] data:[%s]\n", len, rx_buf);
		}
		printf("------\n");
	}

	close(fd);
	return 0;
}

