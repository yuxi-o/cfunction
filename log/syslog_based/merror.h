#ifndef __MERROR_H__
#define __MERROR_H__

enum merror_num
{
	E_DUMMY = -1,
	E_NOFILE,
	E_WRITE,
	E_READ,
	E_TIMEOUT,
	E_UNKNOWN,
};


extern char *merror_msg[E_UNKNOWN]; 

#endif
