#ifndef __MUART_H__
#define __MUART_H__

#include <unistd.h>
#include <termios.h>

#define PATH_LEN		16
#define WRITE_MAX_BYTES	128

typedef struct serial_uart{
	int fd;
	int baudrate;
	char databits;
	char stopbits;
	char parity;
	char dev[PATH_LEN];
	struct termios old_conf; 
	int write_max_bytes_once;
} muart_t;

int muart_open(muart_t *m, const char *dev);
int muart_setup(muart_t *m, int speed, char databits, char stopbits, char parity);
ssize_t muart_read(muart_t *m, void *buf, size_t nbytes);
ssize_t muart_read_timeout(muart_t *m, void *buf, size_t nbytes, unsigned int timeout);
ssize_t muart_read_nbytes(muart_t *m, void *buf, size_t nbytes, unsigned int timeout);
ssize_t muart_write(muart_t *m, void *buf, size_t nbytes);
int muart_close(muart_t *m);

#endif

