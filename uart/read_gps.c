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
#include <time.h>
#include <sys/time.h>
#include <signal.h>

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

//////////////////////////////////////////////////////////////////////////////
#define BUF_SIZE	2048 // gt the maximum length of one whole packet
#define MIN_SIZE	64   // according to the maximum length of the info line 

typedef struct gps_info{
	double latitude;
	double longitude;
	int latitude_degree;
	int latitude_minute;
	int latitude_second;
	int longitude_degree;
	int longitude_minute;
	int longitude_second;
	float speed;
	float direction;
	float height;
	int satellite;
	char position_sn;
	char position_ew;
	struct tm date_tm;
	char date_day[16];
	char date_year[16];
} gps_info_t;
gps_info_t mgps;

char flag_send = 0;

void print_gps(gps_info_t *gps)
{
	printf("=============GPS info==================\n");
	printf("\tLatitude: %02d°%02d'%02d\"%c(%f), Longitude: %02d°%02d'%02d\"%c(%f)\n", 
			gps->latitude_degree, gps->latitude_minute, gps->latitude_second,
			gps->position_sn, gps->latitude, 
			gps->longitude_degree, gps->longitude_minute, gps->longitude_second,
			gps->position_ew, gps->longitude);
	printf("\tSpeed: %.2f km/h, Direction: %.2f°\n", gps->speed, gps->direction);
	printf("\tHeight: %.3fm, Satellite: %d\n", gps->height, gps->satellite);
	printf("\tDate: %04d-%02d-%02d %02d:%02d:%02d (%s-%s, ddmmyy-hhmmss.ms)\n", 
			gps->date_tm.tm_year + 1900, gps->date_tm.tm_mon+1, gps->date_tm.tm_mday,
			gps->date_tm.tm_hour, gps->date_tm.tm_min, gps->date_tm.tm_sec,
			gps->date_year, gps->date_day);
}

char date_mday[]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
void utc2btc(struct tm *mtm)
{
	int year;

	mtm->tm_hour += 8;
	if(mtm->tm_hour >= 24){
		mtm->tm_hour -=24;
		mtm->tm_mday +=1;
		if(mtm->tm_mon == 1){
			year = mtm->tm_year + 1900;
			if((year%400 == 0) || ((year%4 == 0) && (year%100 != 0))){
				date_mday[1] = 29;
			}
		}
		if(mtm->tm_mday > date_mday[mtm->tm_mon]){
			mtm->tm_mday = 1;
			mtm->tm_mon += 1;
			if(mtm->tm_mon > 11){
				mtm->tm_mon = 0;
				mtm->tm_year += 1;
			}
		}
	}
}

void fix_gps_info(gps_info_t *gps)
{
	double dd; 
	char *pbuf = NULL;

	gps->latitude_degree = ((int)gps->latitude) / 100;
	dd = gps->latitude - gps->latitude_degree * 100;
	gps->latitude_minute = (int)dd;
	gps->latitude_second = (int)((dd - gps->latitude_minute)*60);

	gps->longitude_degree = ((int)gps->longitude) / 100;
	dd = gps->longitude - gps->longitude_degree * 100;
	gps->longitude_minute = (int)dd;
	gps->longitude_second = (int)((dd - gps->longitude_minute)*60);

	if(strlen(gps->date_day) && strlen(gps->date_year)){
		pbuf = gps->date_day;
		gps->date_tm.tm_hour = (pbuf[0] - '0')*10 + (pbuf[1] - '0');
		gps->date_tm.tm_min = (pbuf[2] - '0')*10 + (pbuf[3] - '0');
		gps->date_tm.tm_sec = (pbuf[4] - '0')*10 + (pbuf[5] - '0');

		pbuf = gps->date_year;
		gps->date_tm.tm_mday = (pbuf[0] - '0')*10 + (pbuf[1] - '0');
		gps->date_tm.tm_mon = (pbuf[2] - '0')*10 + (pbuf[3] - '0') - 1;
		gps->date_tm.tm_year = (pbuf[4] - '0')*10 + (pbuf[5] - '0') + 100;

		utc2btc(&gps->date_tm);
	}
}

// $GPRMC,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>*hh
//        <1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>
void parse_gnrmc(gps_info_t *gps, char *buf)
{
	printf("##################GNRMC: [%s]\n", buf);

	char *strx;
	strx = strstr(buf, "A,"); // A: valid data
	if(!strx){
		printf("invalid GNRMC\n");
		return;
	}
	
	int i = 0;

	// item <1>
	if(buf[i] != ','){
		memset(gps->date_day, 0, sizeof(gps->date_day));
		strncpy(gps->date_day, buf, (int)(strx-buf)-1);
#if 0
		gps->date_tm.tm_hour = (buf[i] - '0')*10 + (buf[i+1] - '0');
		i = i+2;
		gps->date_tm.tm_min = (buf[i] - '0')*10 + (buf[i+1] - '0');
		i = i+2;
		gps->date_tm.tm_sec = (buf[i] - '0')*10 + (buf[i+1] - '0');
#endif
	}
//	while(buf[i++] != ',');

	// item <3>
	i = (int)(strx-buf) + 2;
	if(buf[i] != ','){
		gps->latitude = atof(buf+i);
	}

	// item <4>
	while(buf[i++] != ',');
	if(buf[i] != ','){
		gps->position_sn = buf[i]; 
	}

	// item <5>
	while(buf[i++] != ',');
	if(buf[i] != ','){
		gps->longitude = atof(buf+i);
	}

	// item <6>
	while(buf[i++] != ',');
	if(buf[i] != ','){
		gps->position_ew = buf[i]; 
	}

	// item <7>
	while(buf[i++] != ',');
	if(buf[i] != ','){
		gps->speed = atof(buf+i) * 1.85; 
	}


	// item <8>
	while(buf[i++] != ',');
	if(buf[i] != ','){
		gps->direction = atof(buf+i); 
	}

	// item <9>
	while(buf[i++] != ',');
	if(buf[i] != ','){
		memset(gps->date_year, 0, sizeof(gps->date_year));
		strncpy(gps->date_year, buf+i, 6);
#if 0
		gps->date_tm.tm_mday = (buf[i] - '0')*10 + (buf[i+1] - '0');
		i = i+2;
		gps->date_tm.tm_mon = (buf[i] - '0')*10 + (buf[i+1] - '0') - 1;
		i = i+2;
		gps->date_tm.tm_year = (buf[i] - '0')*10 + (buf[i+1] - '0') + 100;
		i = i+2;
#endif
	}

	// item <10>
	//while(buf[i++] != ',');

	// item <11>
	//while(buf[i++] != ',');

	// item <12>
}

// $GPGGA,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,M,<10>,M,<11>,<12>*hh
//        <1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,M,<10>,M,<11>,<12>
void parse_gngga(gps_info_t *gps, char *buf)
{
	printf("##################GNGGA: [%s]\n", buf);
	int i = 0, item = 0;

	// find item <6>
	while(item < 5){
		if(buf[i++] == ','){
			item++;
		}
	}

	if(buf[i] == '0'){
		printf("invalid GNGGA\n");
		return;
	}

	// item <7>
	while(buf[i++] != ',');
	if(buf[i] != ','){
		gps->satellite = atoi(buf+i);
	}

	// item <8>
	while(buf[i++] != ',');

	// item <9>
	while(buf[i++] != ',');
	if(buf[i] != ','){
		gps->height = atof(buf+i);
	}
}

char hex_str[]={"0123456789ABCDEF"};
char bcc(char *buf, int len)
{
	int i;
	char crc = 0;

	for(i = 0; i < len; i++){
		crc ^= buf[i];
	}

	return crc;
}

void parse_packet(char *buf)
{
	printf("##################parse one packet: [%s]\n", buf);
	int len;
	char crc, crc_str[2];

	len = strlen(buf);
	if(buf[len-3] != '*'){
		printf("Not one right pattern packet: * wrong position\n");
		return;
	}

	crc = bcc(buf+1, len-4);
	crc_str[0] = hex_str[(crc&0xF0)>>4];
	crc_str[1] = hex_str[crc&0x0F];
	if(strncasecmp(crc_str, buf+len-2, 2)){
		printf("Not one right pattern packet:crc is 0x%x\n", crc);
		return;
	}

	buf[len-3] = 0;
	if(!strncmp("GNRMC", buf+1, 5)){
		parse_gnrmc(&mgps, buf+7); // omit the first comma ($GPRMC,)
	} else if(!strncmp("GNGGA", buf+1, 5)){
		parse_gngga(&mgps, buf+7); // omit the 1st comma ($GPGGA,)

		if(flag_send){
			fix_gps_info(&mgps);
			print_gps(&mgps);
			flag_send = 0;
		}
	}
}

int parse_buf(char *buf, int len)
{
	int i = 0, istart = 0;

	while((buf[i] != '$')&& (i < len)){
		i++;
	}
	if(i >= len){
		return len;
	}

	istart = i;
	for (; i < len; i++){
		if(buf[i] == '\r'){
			buf[i] = 0;
			parse_packet(buf + istart);
			istart = ++i;
			if((i < len) && buf[i] == '\n'){
				istart++;
				i++;
			}
		}
	}
	
	return istart;
}

//////////////////////////////////////////////////////////////////
void itmer_func(int signo)
{
	printf("timestamp: %ld\n", time(NULL));
	flag_send = 1;
//	fix_gps_info(&mgps);
//	print_gps(&mgps);
}

// start based on the multiple of near_seconds
void set_interval_timer(struct itimerval *it, int seconds, int near_seconds)
{
	time_t tt, diff;

	it->it_interval.tv_sec = seconds;
	it->it_interval.tv_usec = 0;
	it->it_value.tv_usec = 0;

	tt = time(NULL);
	diff = (tt + near_seconds) % near_seconds;

	it->it_value.tv_sec = near_seconds - diff;
	setitimer(ITIMER_REAL, it, NULL);
	printf("setitimer: now: %ld, left: %ld\n", tt, it->it_value.tv_sec);
}

int main(int argc, char *argv[])
{
#if 1
	int uart_fd = 0;
	char rx_buf[BUF_SIZE];
	int speed = 0;
	char pserial[3]  = {0}, flag_data = 0;
	int rx_len = 0, parse_len = 0;
	int num_errors = 0;
	int read_pos = 0, write_pos = 0, left = 0;
	
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

	uart_fd = init_uart(argv[1], speed, pserial[0]-'0', pserial[2]-'0', pserial[1]);
	if (uart_fd < 0){
		printf("FUNC:%s, init uart %s failed !\n", __func__, argv[1]);
		exit(-1);
	}

	signal(SIGALRM, itmer_func);
	memset(&mgps, 0, sizeof(mgps));

	struct itimerval it;
	set_interval_timer(&it, 10, 180);

	while(1){
		left = sizeof(rx_buf)-1 - write_pos;
		if(left < MIN_SIZE){ // lt maximum_length of the line 
			write_pos = write_pos - read_pos;
			memcpy(rx_buf, rx_buf + read_pos, write_pos);
			read_pos = 0;
			left = sizeof(rx_buf)-1 - write_pos;
		}

		rx_len = dread(uart_fd, rx_buf + write_pos, left, 0);
		if(rx_len < 0){
			if(errno == ETIME){
				continue;
			}
			num_errors++;
			if(num_errors > 5){
				printf("Restart uart\n");
				uart_fd = init_uart(argv[1], speed, pserial[0]-'0', pserial[2]-'0', pserial[1]);
				if (uart_fd < 0){
					printf("FUNC:%s, init uart %s failed !\n", __func__, argv[1]);
					exit(-1);
				}

				num_errors = 0;
			}
			continue;
		}
		rx_buf[write_pos+rx_len] = 0;
//		printf("[%s]\n-----recv: %d, read: %d, write: %d-----\n", 
//				rx_buf+write_pos, rx_len, read_pos, write_pos);
		write_pos += rx_len;

		// deal with packet
		parse_len = parse_buf(rx_buf + read_pos, write_pos - read_pos);
		read_pos += parse_len;
	}

	close(uart_fd);

#else
	char str[]="092927.000,A,2235.9058,N,11400.0518,E,0.000,74.11,151216,,D";
	parse_gnrmc(&mgps, str);
	char str1[]="092927.000,2235.9058,N,11400.0518,E,2,9,1.03,53.1,M,-2.4,M,0.0,0";
	parse_gngga(&mgps, str1);
	fix_gps_info(&mgps);
	print_gps(&mgps);
#endif

	return 0;
}

